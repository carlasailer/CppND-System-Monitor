#include <vector>
#include <string>
#include <chrono>
#include <thread>

#include "processor.h"
#include "linux_parser.h"

using namespace std;

// Return the aggregate CPU utilization
float Processor::Utilization() { 
    float previdle{0.0f}, idle{0.0f}, prevnonidle{0.0f}, nonidle{0.0f};
    vector<string> time1 = LinuxParser::CpuUtilization();
    this_thread::sleep_for(chrono::milliseconds(30));
    vector<string> time2 = LinuxParser::CpuUtilization();

    //Idle = idle + iowait
    if (LinuxParser::is_number(time1[3]) && LinuxParser::is_number(time1[4]) && LinuxParser::is_number(time2[3]) && LinuxParser::is_number(time1[4])) {
      previdle = stof(time1[3]) + stof(time1[4]);
      idle = stof(time2[3]) + stof(time2[4]);
    }

    //nonidle = user + nice + system + irq + softirq + steal
    if (LinuxParser::is_number(time1[0]) && LinuxParser::is_number(time1[1]) && LinuxParser::is_number(time1[2]) && LinuxParser::is_number(time1[5]) && LinuxParser::is_number(time1[7])) {
       prevnonidle = stof(time1[0]) + stof(time1[1]) + stof(time1[2]) + stof(time1[5]) + stof(time1[6]) + stof(time1[7]);
    }
    
    if (LinuxParser::is_number(time2[0]) && LinuxParser::is_number(time2[1]) && LinuxParser::is_number(time2[2]) && LinuxParser::is_number(time2[5]) && LinuxParser::is_number(time2[7])) {
      nonidle = stof(time2[0]) + stof(time2[1]) + stof(time2[2]) + stof(time2[5]) + stof(time2[6]) + stof(time2[7]);
    }
    //total = idle + nonidle
    float prevtotal = previdle + prevnonidle;
    float total = idle + nonidle;

    //differentiate 
    float totald = total - prevtotal;
    float idled = idle - previdle;

    return ((totald - idled) / totald);


}
    
  