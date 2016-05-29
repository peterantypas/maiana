#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <vector>
#include <inttypes.h>

using namespace std;

class Utils
{
public:
	static void delay(uint32_t ticks);
	static void makeLowercase(std::string &);
	static void makeUppercase(std::string &);
	static void trim(std::string &s);
	static void tokenize(const string &str, char delim, vector<string> &result);

	static std::string toString(int);
	static std::string toString(unsigned);
	static std::string toString(double);
	static std::string toString(long long);
	static int toInt(const std::string &);
	static long long toInt64(const std::string &);
	static double toDouble(const std::string &);

	static double latitudeFromNMEA(const string &decimal, const string &hemisphere);
	static double longitudeFromNMEA(const string &decimal, const string &hemisphere);

	static double haversineDistance(double lat1, double lon1, double lat2, double lon2);

	static uint16_t crc16(uint8_t* data, uint16_t len);
	static uint16_t reverseBits(uint16_t data);

    static uint32_t coordinateToUINT32(double value);
    static double coordinateFromUINT32(uint32_t aisCoordinate, uint8_t numBits);

    static bool inISR();
};

#endif
/* 
 Local Variables: ***
 mode: c++ ***
 End: ***
 */
