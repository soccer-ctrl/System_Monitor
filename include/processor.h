#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <string>
#include <vector>
using std::vector;

class Processor {
 public:
  double Utilization();  


 private:
 std::vector<std::string> cpuUtilization = {};

 vector<double> getCpuTime(const vector<std::string>&);
  long cached_active_ticks_;
  long cached_idle_ticks_;

};

#endif
