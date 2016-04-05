#include <algorithm>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <future>
#include <map>
#include <limits>
#include <string>
#include <vector>
#include <set>
#include <cctype>
#include <thread>
#include <experimental/filesystem>

#define NOMINMAX
#include <Windows.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class PerformanceTimer
{
public:
	PerformanceTimer() { QueryPerformanceCounter(&start); }
	double time()
	{
		LARGE_INTEGER now, freq;
		QueryPerformanceCounter(&now);
		QueryPerformanceFrequency(&freq);
		return double(now.QuadPart - start.QuadPart) / double(freq.QuadPart);
	}
private:
	LARGE_INTEGER start;
};

template< typename Index, typename Function >
void parallel_ranges(Index begin, Index end, Function const &f)
{
	if (end <= begin) return;

	auto n_partitions = std::thread::hardware_concurrency();

	auto partition_size = ( end - begin ) / n_partitions;
	auto remainder = ( end - begin ) - n_partitions * partition_size;

	std::vector<std::future<void>> futures;
	futures.reserve(n_partitions);

	Index end_p = begin;
	while (n_partitions--)
	{
		Index begin_p = end_p;
		end_p = begin_p + partition_size;

		if (remainder) --remainder, ++end_p;

		futures.push_back(std::async([begin_p, end_p, &f]()
			{f(begin_p, end_p); }));
	}

	for (auto &f : futures) f.wait();
}

template< typename Index, typename Function >
void parallel_for(Index begin, Index end, Function const &f)
{
	parallel_ranges(begin, end, [&f](Index begin_p, Index end_p)
		{ for (Index i = begin_p; i != end_p; ++i) f(i); });
}

template<typename DataType, int ChannelCount>
class Image
{
public:
	using Pixel = std::array<DataType, ChannelCount>;

	Image(int width, int height)
		: m_width(width), m_height(height), m_data(width * height)
	{}

	Image(int width, int height, Pixel *data)
		: m_width(width), m_height(height), m_data(data, data + width * height)
	{}

	Image(int width, int height, DataType *data)
		: m_width(width), m_height(height), m_data(width * height)
	{
		for (int i = 0; i < width * height; ++i)
		{
			std::copy(data, data + ChannelCount, m_data[i].data());
			data += ChannelCount;
		}
	}


	Pixel &at(int x, int y) { return m_data[x + y * m_width]; }
private:
	std::vector<Pixel> m_data;
	int m_width;
	int m_height;
};

int parse_number(char const **in)
{
	int num = 0;
	while (std::isdigit(**in))
	{
		num = num * 10 + **in - '0';
		++*in;
	}
	return num;
}

std::vector<int> parse_numbers(char const *in)
{
	std::set<int> set;
	bool range = false;
	int range_begin = 0;
	while (*in)
	{
		if (std::isdigit(*in))
		{
			int num = parse_number(&in);

			if (!range) range_begin = num;

			for (int i = range_begin; i <= num; ++i)
				set.insert(i);
			range = false;
		}
		if (*in == '-' && range_begin)
			range = true;

		if (*in)
			++in;
	}
	std::vector<int> vector(set.begin(), set.end());
	return vector;
}


std::vector<unsigned char> read_file(char const *filename)
{
	using namespace std;

	ifstream is(filename, ios::binary);

	// get length of file:
	is.seekg(0, ios::end);
	std::streamoff length = is.tellg();
	std::vector<unsigned char> buffer;

	if (length > 0)
	{
		buffer.resize(length);
		is.seekg(0, ios::beg);
		is.read(( char * )buffer.data(), length);
	}

	return buffer;
}

bool fit(stbtt_fontinfo *font, std::vector<int> const &chars,
	int size, int padding, int width, int height,
	std::vector<stbrp_rect> &boxes)
{
	float scale = stbtt_ScaleForPixelHeight(font, (float)size);

	for (int i = 0; i < chars.size(); ++i)
	{
		int x0, y0, x1, y1;
		stbtt_GetCodepointBitmapBox(font, chars[i], scale, scale,
			&x0, &y0, &x1, &y1);
		boxes[i].id = chars[i];
		boxes[i].x = 0;
		boxes[i].y = 0;
		boxes[i].w = x1 - x0 + padding * 2;
		boxes[i].h = y1 - y0 + padding * 2;
		boxes[i].was_packed = 0;
	}
	stbrp_context context;
	std::vector<stbrp_node> nodes(width);
	stbrp_init_target(&context, width, height, nodes.data(), width);
	stbrp_pack_rects(&context, boxes.data(), (int)boxes.size());

	for (auto &box : boxes)
		if (!box.was_packed)
			return false;
	
	return true;
}

std::vector<unsigned char> draw(stbtt_fontinfo *font, std::vector<int> const &chars,
	int size, int padding, int width, int height,
	std::vector<stbrp_rect> const &boxes)
{
	std::vector<unsigned char> bitmap(width * height);

	float scale = stbtt_ScaleForPixelHeight(font, (float)size);
	parallel_for(0, (int)boxes.size(), [&](int i) {
		stbtt_MakeCodepointBitmap(font,
			bitmap.data() + boxes[i].x + padding + ( boxes[i].y + padding ) * width,
			boxes[i].w - padding * 2, boxes[i].h - padding * 2, width, scale, scale, boxes[i].id);
	});

	return bitmap;
}

struct GlyphOffsets
{
	int x, y, advance;
};

std::vector<GlyphOffsets> glyph_offsets(stbtt_fontinfo *font, std::vector<int> const &chars,
	int size, int padding)
{
	std::vector<GlyphOffsets> offsets;
	offsets.reserve(chars.size());

	float scale = stbtt_ScaleForPixelHeight(font, (float)size);

	for (auto c : chars)
	{
		int x0, y0, x1, y1, advance, lsb;
		stbtt_GetCodepointHMetrics(font, c, &advance, &lsb);
		stbtt_GetCodepointBitmapBox(font, c, scale, scale,
			&x0, &y0, &x1, &y1);
		offsets.push_back({ x0 - padding, y0 - padding, (int)(advance * scale) });
	}

	return offsets;
}

struct KerningPair
{
	int first;
	int second;
	int advance;
};

std::vector<KerningPair> get_kerning_pairs(const stbtt_fontinfo *font, std::vector<int> const &chars, int size)
{
	std::map<int, int> glyph_to_cp;
	for (auto c : chars)
		glyph_to_cp[stbtt_FindGlyphIndex(font, c)] = c;

	std::vector<KerningPair> pairs;

	// we only look at the first table. it must be 'horizontal' and format 0.
	if (!font->kern)
		return pairs;

	float scale = stbtt_ScaleForPixelHeight(font, (float)size);

	stbtt_uint8 *data = font->data + font->kern;

	if (ttUSHORT(data + 2) < 1) // number of tables, need at least 1
		return pairs;
	if (ttUSHORT(data + 8) != 1) // horizontal flag must be set in format
		return pairs;

	for (int i = 0; i < ttUSHORT(data + 10); ++i)
	{
		stbtt_uint32 pair = ttULONG(data + 18 + ( i * 6 ));
		int first = pair >> 16;
		int second = pair & 0xFFFF;
		auto first_cp = glyph_to_cp.find(first);
		auto second_cp = glyph_to_cp.find(second);
		if (first_cp != glyph_to_cp.end() && second_cp != glyph_to_cp.end())
		{
			int advance = (int)(scale * ttSHORT(data + 22 + ( i * 6 )));
			if( advance )
				pairs.push_back({ first_cp->second, second_cp->second, advance });
		}
	}
	return pairs;
}


int option(int argc, char* argv[], int index, int default_value)
{
	if (argc > index)
	{
		const char *c = argv[index];
		return parse_number(&c);
	}
	return default_value;
}

const float inf_neg = -100000000.f;
const float inf_pos = 100000000.f;
//const float inf_neg = std::numeric_limits<float>::lowest();
//const float inf_pos = std::numeric_limits<float>::max();


// Workspace for distance_transform
struct distance_transform_ws
{
	distance_transform_ws(int size) : v(size), z(size + 1) {}
	std::vector<int> v;
	std::vector<float> z;
};

// From http://cs.brown.edu/~pff/dt/index.html
// Distance Transforms of Sampled Functions
// P. Felzenszwalb, D. Huttenlocher
// Theory of Computing, Vol. 8, No. 19, September 2012

template< typename In, typename Out >
void distance_transform(int n, In f, Out out, int divider = 1);

template< typename In, typename Out >
void distance_transform(int n, In f, Out out, distance_transform_ws &ws, int divider = 1)
{
	auto sq = [](auto f) {return f * f;};
	int k = 0;
	ws.v[0] = 0;
	ws.z[0] = inf_neg;
	ws.z[1] = inf_pos;

	for (int q = 1; q < n; ++q)
	{
		float s;
		while (( s = ( ( f(q) + q*q ) - ( f(ws.v[k]) + sq(ws.v[k]) ) ) / ( 2 * q - 2 * ws.v[k] ) )
		       <= ws.z[k])
			--k;
		++k;
		ws.v[k] = q;
		ws.z[k] = s;
		ws.z[k + 1] = inf_pos;
	}

	k = 0;
	int next_out = divider / 2;
	for (int q = 0; q < n; ++q)
	{
		while (ws.z[k + 1] < q)
			++k;
		if (q == next_out)
		{
			out.set(sq(q - ws.v[k]) + f(ws.v[k]));
			++out;
			next_out += divider;
		}
	}
}

template< typename In, typename Out >
void distance_transform(int n, In f, Out out, int divider)
{
	distance_transform_ws ws(n);
	distance_transform(n, f, out, ws, divider);
}

template< typename Type >
class Distance
{
public:
	Distance(Type *data, int stride, float map( Type ) ) : m_data(data), m_stride(stride), m_map(map) {}
	float operator()(int i)
	{
		return m_map(*(m_data + i * m_stride));
	}
private:
	Type *m_data;
	int m_stride;
	float (*m_map)(Type);
};

class OutToFloat
{
public:
	OutToFloat(float *data, int stride) : m_data(data), m_stride(stride) {}
	void set(float f) { *m_data = f; }
	OutToFloat &operator++() { m_data += m_stride; return *this; }
private:
	float *m_data;
	int m_stride;
};

class OutToByte
{
public:
	OutToByte(unsigned char *data, int stride, int range) : m_data(data), m_stride(stride), m_range(range) {}
	void set(float f) { *m_data = (unsigned char)std::min(255.f * std::sqrt(f)/ m_range, 255.f); }
	OutToByte &operator++() { m_data += m_stride; return *this; }
private:
	unsigned char  *m_data;
	int m_stride;
	int m_range;
};


std::vector<float> make_unsigned_dt(unsigned char *bitmap, int width, int height, int stride, float map(unsigned char), int divider = 1)
{
	std::vector<float> temp(width * height / divider);
	std::vector<float> result(width * height / (divider * divider));

	// Note: The temp buffer is transposed (rows and columns switched) with
	// respect to the input and output. This results in better performance in
	// the second stage due to dequential memory reads.
	// Typical test showed 3.2s (untransposed) vs 2.6s (transposed).
	// (0.9s with parallel_for)

	//parallel_for(0, height, [&](int i) {
	//	distance_transform(width,
	//		Distance<unsigned char>(bitmap + i * stride, 1, map),
	//		OutToFloat(temp.data() + i, height), divider);
	//});

	//parallel_for(0, width / divider, [&](int i) {
	//	distance_transform(height,
	//		Distance<float>(temp.data() + i * height, 1, [](float d) {return d; }),
	//		OutToFloat(result.data() + i, width / divider), divider);
	//});

	parallel_ranges(0, height, [&](int begin, int end) {
		distance_transform_ws ws(width);
		for (int i = begin; i != end; ++i)
			distance_transform(width,
				Distance<unsigned char>(bitmap + i * stride, 1, map),
				OutToFloat(temp.data() + i, height), ws, divider);
	});

	parallel_ranges(0, width / divider, [&](int begin, int end) {
		distance_transform_ws ws(height);
		for (int i = begin; i != end; ++i)
			distance_transform(height,
				Distance<float>(temp.data() + i * height, 1, [](float d) {return d; }),
				OutToFloat(result.data() + i, width / divider), ws, divider);
	});
	return result;
}

std::vector<unsigned char> make_dt(unsigned char *bitmap, int width, int height, int stride, int max_distance, int divider)
{
	auto positive = make_unsigned_dt(bitmap, width, height, stride,
		[](unsigned char d) {return d < 127 ? inf_pos : 0.f; }, divider);

	auto negative = make_unsigned_dt(bitmap, width, height, stride,
		[](unsigned char d) {return d >= 127 ? inf_pos : 0.f; }, divider);

	width /= divider;
	height /= divider;

	std::vector<unsigned char> result(width * height);
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			float p_pos = positive[y * width + x];
			float p_neg = negative[y * width + x];
			float signed_dist = std::min(std::sqrt(p_pos > p_neg ? p_pos : p_neg), ( float )max_distance);
			if (p_neg > p_pos)
				signed_dist = -signed_dist;
			result[y * width + x] = (unsigned char)(127.f * signed_dist / max_distance + 127);
		}
	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "usage: blah\n";
		return 1;
	}

	std::string font_file(argv[1]);
	int width = option( argc, argv, 2, 1024);
	int height = option(argc, argv, 3, width);
	int padding = option(argc, argv, 4, 0);
	int scale = option(argc, argv, 5, 1);

	std::string cmd;
	for (int i = 6; i < argc; ++i)
	{
		cmd += " ";
		cmd += argv[i];
	}

	auto chars = parse_numbers(cmd.c_str());
	if (chars.empty())
		chars = parse_numbers("32-126");

	auto font_data = read_file(font_file.c_str());
	if (font_data.empty())
	{
		std::cout << "Error reading font file\n";
		return 1;
	}
	stbtt_fontinfo font;
	stbtt_InitFont(&font, font_data.data(), 0);

	std::vector<stbrp_rect> boxes(chars.size());

	std::cout << "Finding best size..." << std::endl;
	PerformanceTimer fit_timer;
	int lower = 0, upper = 20;
	while (fit(&font, chars, upper, padding, width, height, boxes))
	{
		lower = upper;
		upper *= 2;
	}

	while (lower < upper - 1)
	{
		int mid = ( lower + upper ) / 2;
		if (fit(&font, chars, mid, padding, width, height, boxes))
			lower = mid;
		else
			upper = mid;
	}


	if (fit(&font, chars, lower, padding, width, height, boxes))
	{
		std::cout << "Time taken: " << fit_timer.time() << std::endl;
		PerformanceTimer fit_timer;
		std::cout << "Rendering glyphs..." << std::endl;
		using path = std::experimental::filesystem::path;

		auto scaled_boxes = boxes;
		for (auto &box : scaled_boxes)
			box.x *= scale,  box.y *= scale, box.w *= scale, box.h *= scale;

		PerformanceTimer draw_timer;
		auto scaled_bitmap = draw(&font, chars, lower * scale, padding * scale, width * scale, height * scale, scaled_boxes);
		std::cout << "Time taken: " << draw_timer.time() << std::endl;

		std::cout << "Calculating signed distance..." << std::endl;
		PerformanceTimer distance_timer;
		auto bitmap = make_dt(scaled_bitmap.data(), width * scale, height * scale, width * scale, padding * scale, scale);
		std::cout << "Time taken: " << distance_timer.time() << std::endl;

		path png_name = path(font_file).filename();
		png_name.replace_extension(".png");
		stbi_write_png(png_name.string().c_str(), width, height, 1, bitmap.data(), width);

		path out_name = path(font_file).filename();
		out_name.replace_extension(".bdf");

		auto offsets = glyph_offsets(&font, chars, lower, padding);
		auto kerning_pairs = get_kerning_pairs(&font, chars, lower);

		std::ofstream bdf(out_name.string());
		bdf << png_name.string() << '\n';
		bdf << width << ' ' << height << ' ' << lower << ' ' << padding << '\n';
		bdf << chars.front() << ' ' << chars.back() << '\n';
		for (int i = 0; i < chars.size(); ++i)
		{
			bdf << chars[i] << ' ' << boxes[i].x << ' ' << boxes[i].y << ' '
				<< boxes[i].w << ' ' << boxes[i].h << ' '
				<< offsets[i].x << ' ' << offsets[i].y << ' ' << offsets[i].advance << '\n';
		}
		bdf << kerning_pairs.size() << '\n';
		for (auto &k : kerning_pairs)
		{
			bdf << k.first << ' ' << k.second << ' ' << k.advance << '\n';
		}
	}
	else
	{
		std::cout << "Failed to fit. Try larger image size or a smaller padding value.\n";
		return 1;
	}

	return 0;
}

