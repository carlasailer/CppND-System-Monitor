#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <experimental/filesystem>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// generic functions for parsing 
template <typename T>
T findValueByKey (string const &keyFilter, string const &file) {
  string line, key;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + file);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          return value;
        }
      }
    }
  }
  return value;
};

template <typename T>
T getValueOfFile( string const &file) {
  string line;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + file);
    if (stream.is_open()) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> value;
    }
  return value;
};


// helper function to determine whether string is only digits 
bool LinuxParser::is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

// Read and return the system OS info
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;

  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the Kernel info
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Read and return Pids 
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  float memTotal = findValueByKey<float>(filterMemTotalString, kMeminfoFilename);
  float memFree = findValueByKey<float>(filterMemFreeString, kMeminfoFilename);
  float buffers = findValueByKey<float>(filterBufferString, kMeminfoFilename);
  float cached = findValueByKey<float>(filterCachedString, kMeminfoFilename);
  
  return ((memTotal - memFree - buffers - cached) / memTotal); 
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  return long(getValueOfFile<long>(kUptimeFilename));
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  string line;
  vector<string> times{};

  // read from proc stat file
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    filestream.ignore(5, ' ');
    //write each item after cpu into a vector
    for (string time; filestream >> time; times.emplace_back(time));
  }
  return times; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  return findValueByKey<int>(filterProcesses, kStatFilename);
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  return findValueByKey<int>(filterRunningProcesses, kStatFilename);
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  return string(getValueOfFile<string>(std::to_string(pid) + kCmdlineFilename));
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string value;

  // use VmRSS instead of VmSize to get exact physical memory instead of all virtual memory
  double ram = findValueByKey<double>(filterProcMem, std::to_string(pid) + kStatusFilename);
  
  //convert from kB to MB
  ram *= 0.001; 

  //format to be displayed with two decimals
  value = std::to_string(ram);
  value = value.substr(0, value.size()-4);
  
  return value;
 
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  return findValueByKey<string>(filterUID, std::to_string(pid) + kStatusFilename);
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  string name{" "};
  string password;
  string uid;

  // get the user id associated with the process
  string find_uid = LinuxParser::Uid(pid);
  
  // use the uid to find the user name
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while(linestream >> name >> password >> uid) {
        if (uid == find_uid) { return name; } 
      }
    }
  }
  return name; 
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  string line;
  string value{};
  int position = 22;
  float uptime{0.0f};

  // access the uptime file for the process
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
    
  if (filestream.is_open()) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      for (auto i = 0; i < position; i++) {
        linestream >> value;
      }    
  }
  //convert lock ticks to seconds
  if (LinuxParser::is_number(value)) { 
    uptime = std::stof(value) / sysconf(_SC_CLK_TCK);
    //substract process uptime from system uptime
    return (LinuxParser::UpTime() - uptime); 
  }

  else { return uptime; }
}

// Read and return the CPU times for the process
vector<string> LinuxParser::CpuUtilization(int pid) {
  string line;
  vector<string> times{};

  // read from proc stat file
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    for (string time; filestream >> time; times.emplace_back(time));
  }
  return times; 
}