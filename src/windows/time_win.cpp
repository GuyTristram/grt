#include "input/time.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define  MMNODRV         //Installable driver support
#define  MMNOSOUND       //Sound support
#define  MMNOWAVE        //Waveform support
#define  MMNOMIDI        //MIDI support
#define  MMNOAUX         //Auxiliary audio support
#define  MMNOMIXER       //Mixer support
//#define  MMNOTIMER       //Timer support
#define  MMNOJOY         //Joystick support
#define  MMNOMCI         //MCI support
#define  MMNOMMIO        //Multimedia file I/O support
#define  MMNOMMSYSTEM    //General MMSYSTEM functions
#include <Mmsystem.h>


RealTime::RealTime() : m_time(timeGetTime())
{
}

RealTime::~RealTime()
{
}

double RealTime::elapsed()
{
	return double(timeGetTime() - m_time) * 0.001;
}

void RealTime::reset()
{
	m_time = timeGetTime();
}
