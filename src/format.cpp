#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    int minutes = seconds % 3600;
    std::ostringstream oss;
    oss.fill('0');
    oss << std::setw(2) << seconds / 3600;
    oss << std::setw(1) << ":";
    oss << std::setw(2) << minutes / 60;
    oss << std::setw(1) << ":";
    oss << std::setw(2) << minutes % 60;
    return oss.str();
}
