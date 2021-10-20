/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/


#ifndef GPS_HPP_
#define GPS_HPP_

#include <inttypes.h>
#include <time.h>
#include "EventQueue.hpp"

class GPSDelegate
{
public:
	virtual ~GPSDelegate()
	{
	}

	virtual void timeSlotStarted(uint32_t slotNumber)=0;
};

class GPS : public EventConsumer
{
public:
	static GPS &instance();
	virtual ~GPS();

	void init();
	void enable();
	void disable();
	bool isEnabled();
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
	void processLine(const char *buff);
	void parseSentence(const char *buff);
private:
	char mBuff[100];
	uint8_t mBuffPos;
	time_t mUTC;
	float mLat;
	float mLng;
	bool mStarted;
	uint32_t mSlotNumber;
	GPSDelegate *mDelegate;
	float mCOG;
	float mSpeed;
	uint32_t mPeriod;
	struct tm mTime;
};

#endif /* GPS_HPP_ */
