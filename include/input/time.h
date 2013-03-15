#ifndef TIME_H
#define TIME_H

class RealTime
{
public:
	RealTime();
	~RealTime();

	double elapsed();
	void reset();
private:
	long m_time;
};
#endif
