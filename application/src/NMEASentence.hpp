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
    bool parse();
    const string &mRaw;
    string mCode;
    vector<string> mFields;
};

#endif

/* 
 Local Variables: ***
 mode: c++ ***
 End: ***
 */
