#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include "linux_parser.h"

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  //constructor to extract all relevant info upon initialization
  Process(int p);

  //getter functions
  int Pid();                               
  std::string User();                      
  std::string Command();                   
  float CpuUtilization() const;            
  std::string Ram();                       
  long int UpTime();                       
  bool operator<(Process const& a) const; 

  //calculate cpu function
  float CalcCpuUtilization(int pid);

  // Declare any necessary private members
 private:
  int pid_{0};
  std::string user_{" "};
  std::string command_{" "};
  float cpu_{0.0f};
  std::string ram_{" "};
  long int uptime_{0};

};

#endif