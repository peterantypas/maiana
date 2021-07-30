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

#include "NMEASentence.hpp"
#include "Utils.hpp"

NMEASentence::NMEASentence(const string &raw)
: mRaw(raw)
{
  parse();
}

const string &NMEASentence::code() const
{
  return mCode;
}

const string &NMEASentence::raw() const
{
  return mRaw;
}

const vector<string> &NMEASentence::fields() const
{
  return mFields;
}

bool NMEASentence::parse()
{
  if ( mRaw.empty() )
    return false;

  if ( mRaw[0] != '$' && mRaw[0] != '!' )
    return false;

  Utils::tokenize(mRaw, ',', mFields);
  mFields[0] = mFields[0].substr(1); // Remove the $ or ! from the first field
  string &lastField = *mFields.rbegin();

  size_t p = lastField.find("*");
  lastField = lastField.substr(0, p);
  mCode = mFields[0];

  return true;
}
