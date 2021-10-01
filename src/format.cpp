#include <string>

#include "format.h"

using std::string;

// Helper function 
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    int h = seconds / 3600;
    int min = (seconds % 3600) / 60;
    int sec = seconds % 60;
    string formatted;
    
    if (h < 10) {formatted += "0";}
    formatted += std::to_string(h) + ":";
    if (min < 10) { formatted += "0";}
    formatted += std::to_string(min) + ":";
    if (sec < 10) { formatted += "0";}
    formatted += std::to_string(sec);
     
    return formatted;
}