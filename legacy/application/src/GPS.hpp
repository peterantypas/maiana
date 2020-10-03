/*
 * GPS.hpp
 *
 *  Created on: Nov 9, 2015
 *      Author: peter
 */

#ifndef GPS_HPP_
#define GPS_HPP_

#include <cstdint>
#include <time.h>
#include "EventQueue.hpp"

class GPSDelegate
{
public:
	virtual ~GPSDelegate()
	{
	}
	//virtual void locationResolved(time_t UTC, double lat, double lng)=0;
	virtual void timeSlotStarted(uint32_t slotNumber)=0;
};

class GPS : public EventConsumer
{
public:
	static GPS &instance();
	virtual ~GPS();

	void init();
	void onRX(char c);
	void onPPS();
	void startTimer();
	void stopTimer();
	void resetTimer();
	void onTimerIRQ();
	time_t UTC();
	struct tm &time();
	uint32_t aisSlot();
	double lat();
	double lng();
	void setDelegate(GPSDelegate *delegate);

	void onIRQ(uint32_t mask, void *data);
	void processEvent(const Event &event);
private:
	GPS();
	void reset();
	void processLine(const char *buff);
	void parseSentence(const char *buff);
private:
	char mBuff[100];
	uint8_t mBuffPos;
	time_t mUTC;
	double mLat;
	double mLng;
	bool mStarted;
	uint32_t mSlotNumber;
	GPSDelegate *mDelegate;
	double mCOG;
	double mSpeed;
	uint32_t mPeriod;
	struct tm mTime;
};

#endif /* GPS_HPP_ */
