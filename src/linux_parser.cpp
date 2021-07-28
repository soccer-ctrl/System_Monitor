#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include "linux_parser.h"
#include "format.h"
#include "time.h"   

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
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

//Get system memory utilisation (as percent of total memory)
float LinuxParser::MemoryUtilization() {
  std::string memTotal = "MemTotal:";
  std::string memFree = "MemFree:";
  try{
    float total = LinuxParser::findValueByKey<float>(memTotal, (kProcDirectory + kMeminfoFilename));
    float free = LinuxParser::findValueByKey<float>(memFree, (kProcDirectory + kMeminfoFilename));
    return ((total - free) / total);
  }catch(...){return 0;}
}

//Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string token;
  std::ifstream fileStream(kProcDirectory + kUptimeFilename);
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream stream(line);
    if (stream >> token) {
      return stoi(token);
    }
  }
  fileStream.close();
  return 0;
}
//Read and return the number of jiffies
long LinuxParser::Jiffies() {return (UpTime() * sysconf(_SC_CLK_TCK));}

//Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  ///parsing the /proc/[pid]/stat file
  string line, token;
  vector<std::string> tokens;
  long jiffies = 0;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatFilename);
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      tokens.push_back(token);
    } 
  }
  fileStream.close();
  /*
  We want values in vector positions 13 (user), 14 (kernel), 
  15 (user children), and 16 (kernel children) 
  */
 jiffies = stol(tokens[13])+
           stol(tokens[14])+
           stol(tokens[15])+
           stol(tokens[16]);
  return jiffies;
}

//Read and return the active jiffies for the system
long LinuxParser::ActiveJiffies(){
  vector<string> times = CpuUtilization();
  return (stol(times[CPUStates::kUser_]) + 
          stol(times[CPUStates::kNice_]) +
          stol(times[CPUStates::kSystem_]) + 
          stol(times[CPUStates::kIRQ_]) +
          stol(times[CPUStates::kSoftIRQ_]) + 
          stol(times[CPUStates::kSteal_]) +
          stol(times[CPUStates::kGuest_]) + 
          stol(times[CPUStates::kGuestNice_])
          );
}
//Read and return the idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> times = CpuUtilization();
  return (stol(times[CPUStates::kIdle_]) + 
          stol(times[CPUStates::kIOwait_])
          );
}
//Get the current CPU utilisation (from /proc/stat)
vector<string> LinuxParser::CpuUtilization(){
  vector<std::string> tokens;
  std::string line, token;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  while (getline(fileStream, line)) {
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == "cpu") {
        while (linestream >> token){tokens.push_back(token);}
      }
    }
  }
  fileStream.close();
  return tokens;
}
//Get the total number of processes
int LinuxParser::TotalProcesses(){
  std::string key = "processes";
  try{
    int totalProcesses = LinuxParser::findValueByKey<int>(key, (kProcDirectory + kStatFilename));
    return totalProcesses;
  }catch(...){return 0;}
}

//Get the number of running processes
int LinuxParser::RunningProcesses(){
  std::string key = "procs_running";
  try{
    int totalProcesses = LinuxParser::findValueByKey<int>(key, (kProcDirectory + kStatFilename));
    return totalProcesses;
  }catch(...){return 0;}
}

//Get the command associated with a particular process
string LinuxParser::Command(int pid){
  std::string line;
  std::ifstream fileStream(LinuxParser::kProcDirectory + 
                      to_string(pid) +
                      LinuxParser::kCmdlineFilename);
  std::getline(fileStream, line);
  fileStream.close();
  return line;

}

//Get the RAM used by a particular process
/*
NOTE:
I have used "VmRSS" here instead of "VmSize", as VmSize is the sum of all the 
virtual memory, whereas VmRSS gives the exact physical memory being used as a 
part of the physical RAM.
*/
string LinuxParser::Ram(int pid){
  std::string key = "VmRSS:";
  try{
    int ram = LinuxParser::findValueByKey<int>(key, (kProcDirectory + to_string(pid) +  kStatusFilename));
    return to_string(ram / 1024);
  }catch(...){return "0";}
}

//Get the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::string key = "Uid:";
  try{
    int uid = LinuxParser::findValueByKey<int>(key, (kProcDirectory + to_string(pid) +  kStatusFilename));
    return to_string(uid);
  }catch(...){return "0";}
}

//Read and return the user associated with a process
string LinuxParser::User(int pid){
  string pidString = Uid(pid);
  string line, name, x, pidValue;
  std::ifstream fileStream(kPasswordPath);
  if(fileStream.is_open()){
    while (std::getline(fileStream, line)) {
      //Replace each instance of ":" with a space
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      //Then parse for tokens
      while(linestream >> name >> x >> pidValue){
        if (pidString == pidValue){
          fileStream.close();
          return name;
        }
      }
    } 
  }
  fileStream.close();
  return "0";
}

// Read and return the  uptime for a particular process
long int LinuxParser::UpTime(int pid) {
  string token, line;
  vector <string> tokens;
  std::ifstream fileStream(LinuxParser::kProcDirectory + 
                      to_string(pid) +
                      LinuxParser::kStatFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream linestream(line);
      while(linestream >> token){
        tokens.push_back(token);
      }
    }
  }
  fileStream.close();
  try{
    //The process start time (in clock ticks) was the 22nd token
    //(so vector positon 21)
    long int procStartTime = stol(tokens[21]) / sysconf(_SC_CLK_TCK);
    //Process uptime calculated as system uptime - process start time
    long int sysUpTime = UpTime();
    return sysUpTime - procStartTime;
  }catch(...){ return 0;}
}
