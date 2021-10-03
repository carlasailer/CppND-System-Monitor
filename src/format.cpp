#include <string>
#include <iostream>
#include <chrono>

#include "format.h"

using std::string;

// Helper function 
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long s) {
    std::chrono::seconds seconds{s};

    std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(seconds);
    seconds -= std::chrono::duration_cast<std::chrono::seconds>(hours);

    std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
    seconds -= std::chrono::duration_cast<std::chrono::seconds>(minutes);
    
    string formatted{""};
    if (hours.count() < 10) {formatted += "0";}
    formatted += std::to_string(hours.count()) + ":";
    if (minutes.count() < 10) { formatted += "0";}
    formatted += std::to_string(minutes.count()) + ":";
    if (seconds.count() < 10) { formatted += "0";}
    formatted += std::to_string(seconds.count());
     
    return formatted;
}