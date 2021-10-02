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
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line;
  string key;
  float value;
  float memtotal, memfree, buffers, cached;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") { memtotal = value; }
        else if (key == "MemFree:") { memfree = value; }
        else if (key == "Buffers:") { buffers = value; }
        else if (key == "Cached:") { cached = value; }   
      } 
    }     
  }
  return ((memtotal - memfree - buffers - cached) / memtotal); 
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long value{0};

  // access the uptime file for the system
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) { 
      std::istringstream linestream(line);
      linestream >> value;
      return value;
    }
  }
  return value;
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
    for (string time; filestream >> time; times.push_back(time));
  }
  return times; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line;
  string key;
  int value{0};
  // access the stat file for the process
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return value;
        }
      }
    }  
  }
 return value;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line;
  string key;
  int value{0};

  // access the stat file for the process
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return value;
        }
      }
    }  
  }
 return value;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  string value{""};

  // access the comdline file for the process
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) { 
      std::replace(line.begin(), line.end(), ' ', '#');
      std::istringstream linestream(line);
      linestream >> value;
      return value;
    }
  }
  return value;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line;
  string key;
  string value{" "};
  double ram;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize") {
          //convert from kB to MB
          ram = std::stod(value) * 0.001; 
          value = std::to_string(ram);
          //format to be displayed with two decimals
          value = value.substr(0, value.size()-4);
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string line;
  string key;
  string value{0};

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid") {return value; }
      }
    }
  }
  return value;
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
    for (string time; filestream >> time; times.push_back(time));
  }
  return times; 
}