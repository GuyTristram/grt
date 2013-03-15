#include "math/perlin.h"
// Permutation table

namespace
{
	const unsigned char perm[256] =
	{
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,
		69,142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,
		252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,
		168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
		60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,
		1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,
		86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,
		118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,
		170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,
		22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
		107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,
		150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,
		61,156,180
	};

	static  int    permutation[512];
	static  float  grad4d[512];

	void init_tables()
	{
		for (int i=0; i<256; ++i)
			permutation[256+i] = permutation[i] = (int)perm[i];

		float kkf[256];
		for (int i=0; i<256; ++i)
			kkf[i] = -1.0f + 2.0f * i / 255.0f;

		for (int i=0; i<512; ++i)
			grad4d[i] = kkf[permutation[i]];
	}

	inline  float  fade(float t) { return t * t * (3 - 2 * t); }
	inline  float  plerp(float t, float a, float b) { return  a + t * (b - a); }
	inline  int    fastfloor (float x) { return x > 0 ? (int)x : (int)x - 1; }
}

float perlin( float3 point )
{
	static bool init = false;
	if( !init )
	{
		init_tables();
		init = true;
	}
    int X = fastfloor(point.x);
    int Y = fastfloor(point.y);
    int Z = fastfloor(point.z);
    // Find relative x,y,z of point in cube
    point.x -= X;
    point.y -= Y;
    point.z -= Z;
    // Find unit cube that contains point
    X = X & 255;
    Y = Y & 255;
    Z = Z & 255;
    // Compute fade curves for each of x,y,z.
    float u = fade(point.x);
    float v = fade(point.y);
    float w = fade(point.z);
    // Hash coordinates of the 8 cube corners
    int A  = permutation[X]   + Y;
    int B  = permutation[X+1] + Y;
    int AA = permutation[A]   + Z;
    int AB = permutation[A+1] + Z;
    int BA = permutation[B]   + Z;
    int BB = permutation[B+1] + Z;
    // Add blended results from 8 corners of cube
    return plerp(w, plerp(v, plerp(u, grad4d[AA],
                                      grad4d[BA]),
                             plerp(u, grad4d[AB],
                                      grad4d[BB])),
                    plerp(v, plerp(u, grad4d[AA + 1],
                                      grad4d[BA + 1]),
                             plerp(u, grad4d[AB + 1],
                                      grad4d[BB + 1])));
}
