#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  void CpuUtilization(int pid);
  std::string Ram() const;
  long int UpTime() const;
  bool operator<(const Process& a) const;
  bool operator>(const Process& a) const;
  Process(int pid);
  long Jiffies() const;


 private:
  int pid_;
  float cpu_{};
  long cached_active_ticks_{};
  long cached_idle_ticks_{};
  long cached_system_ticks_{};
};

#endif