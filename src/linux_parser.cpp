#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include "linux_parser.h"
#include "format.h"

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
  float memTotal = 1;
  float memFree = 0;
  std::string line;
  std::string title;
  std::string token;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    //First line contains total memory, second line contains free memory.
    for(int i=0; i<2; i++) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> title >> token;
      if (title == "MemTotal:"){
        memTotal = stof(token);
      }else if (title == "MemFree:"){
        memFree = stof(token);
      }
    }
  }
  return ((memTotal - memFree) / memTotal);
}

//Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string token;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream stream(line);
    if (stream >> token) {
      return stoi(token);
    }
  }
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
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      tokens.push_back(token);
    } 
  }
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
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (getline(stream, line)) {
    std::istringstream linestream(line);
    while (linestream >> token) {
      if (token == "cpu") {
        while (linestream >> token){tokens.push_back(token);}
      }
    }
  }
  return tokens;
}
//Get the total number of processes
int LinuxParser::TotalProcesses(){
  std::string line, key, value;
  int totalProcesses = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)){
    std::istringstream linestream(line);
    while (linestream >> key >> value){
      if (key == "processes"){
        totalProcesses = stoi(value);
        return totalProcesses;
      }
    }
  }
  return totalProcesses;
}
//Get the number of running processes
int LinuxParser::RunningProcesses(){
  std::string line, key, value;
  int runningProcesses = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  while (std::getline(filestream, line)){
    std::istringstream linestream(line);
    while (linestream >> key >> value){
      if (key == "procs_running"){
        runningProcesses = stoi(value);
        return runningProcesses;
      }
    }
  }
  return runningProcesses;
}
//Get the command associated with a particular process
string LinuxParser::Command(int pid){
  std::string line;
  std::ifstream stream(LinuxParser::kProcDirectory + 
                      to_string(pid) +
                      LinuxParser::kCmdlineFilename);
  std::getline(stream, line);
  return line;

}

//Get the RAM used by a particular process
string LinuxParser::Ram(int pid){
  std::string token, ram="0", line, title;
  std::ifstream stream(LinuxParser::kProcDirectory + 
                      to_string(pid) +
                      LinuxParser::kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while(linestream >> title >> ram){
        if(title == "VmSize:"){
          ram = std::to_string(stoi(ram) / 1024);
          return ram;
        }
      }
    }
  }
  return "0";                     
}
//Get the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::string token, uid, title, line;
  std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) +
                       LinuxParser::kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while(linestream >> title >> uid){
        if (title == "Uid:"){
          return uid; 
        }
      }
    }
  }
  return "0";
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
  return "0";
}


// Read and return the  uptime for a particular process
long int LinuxParser::UpTime(int pid) {
  long int upTime = 0;
  string token;
  std::ifstream stream(LinuxParser::kProcDirectory + 
                      to_string(pid) +
                      LinuxParser::kStatFilename);
  if (stream.is_open()) {
    for (int i = 0; stream >> token; ++i)
      //The start time (in clock ticks) is the 22nd value
      if (i == 22) {
        long int upTime{stol(token)};
        upTime /= sysconf(_SC_CLK_TCK);
        //upTime = stol(Format::ElapsedTime(upTime));
        return upTime;
      }
  }
  return upTime;
}
