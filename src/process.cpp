#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {}

//Return this process's ID
int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_; }

//Set cached ticks for a particular process
void Process::CpuUtilization(int pid){

  long activeTicks = LinuxParser::ActiveJiffies(pid);
  long systemTicks = LinuxParser::Jiffies();

  long durationActive = activeTicks - cached_active_ticks_;
  long durationTotal = systemTicks - cached_system_ticks_ ;

  cpu_ = static_cast<float>(durationActive) / durationTotal;

  cached_active_ticks_ = activeTicks;
  cached_system_ticks_ = systemTicks;
}

//Return the command that generated this process
string Process::Command() const {return LinuxParser::Command(Pid());}

//Return this process's memory utilization
string Process::Ram() const {return LinuxParser::Ram(Pid()); }

//Return the user (name) that generated this process
string Process::User() const {return LinuxParser::User(Pid());}

//Return the age of this process (in seconds)
long int Process::UpTime() const {return LinuxParser::UpTime(Pid());}

//Overload the "less than" and "greater than" comparison operators for Process objects
bool Process::operator<(const Process& a) const{
  return CpuUtilization() < a.CpuUtilization();
}
bool Process::operator>(const Process& a) const{
  return CpuUtilization() > a.CpuUtilization();
}