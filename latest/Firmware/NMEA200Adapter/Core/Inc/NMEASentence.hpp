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


#ifndef __NMEA_H__
#define __NMEA_H__

#include <string>
#include <vector>

using namespace std;

class NMEASentence
{
public:
  NMEASentence(const string &raw);
  const string &code() const;
  const vector<string> &fields() const;
  const string &raw() const;
private:
  void parse();
  string mRaw;
  string mCode;
  vector<string> mFields;
};

#endif

/* 
 Local Variables: ***
 mode: c++ ***
 End: ***
 */
