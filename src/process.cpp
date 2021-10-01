#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return  cpu_; }

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization
string Process::Ram() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return uptime_; }

//  Overload the "less than" comparison operator for Process objects
//  sort by CPU usage
bool Process::operator<(Process const& a) const { return a.CpuUtilization() < CpuUtilization(); }

// Calculate the CPU based on parsed values
float Process::CalcCpuUtilization(int pid) {
    vector<string> times = LinuxParser::CpuUtilization(pid);
    
    float total = stof(times[13]) + stof(times[14]) + stof(times[15]) + stof(times[16]);
    float seconds = LinuxParser::UpTime() - ( stof(times[21]) / sysconf(_SC_CLK_TCK));

    return ((total / sysconf(_SC_CLK_TCK)) / seconds);
 }

// Constructor for process class
 Process::Process (int p) : pid_(p) {
    user_ = LinuxParser::User(p);
    command_ = LinuxParser::Command(p);
    ram_ = LinuxParser::Ram(p);
    uptime_ = LinuxParser::UpTime(p);
    cpu_ = Process::CalcCpuUtilization(p);
  }
