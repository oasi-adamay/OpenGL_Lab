#ifndef _TIMER_H_
#define _TIMER_H_

//-----------------------------------------------------------------------------
class Timer{
private:
	cv::TickMeter meter;
	string msg;
public:
	Timer(string _msg){ msg = _msg;  meter.start(); }
	~Timer(void){ meter.stop(); std::cout << msg << meter.getTimeMilli() << "[ms]" << std::endl; }
};

#endif