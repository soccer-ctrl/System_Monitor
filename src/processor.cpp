#include "processor.h"
#include "linux_parser.h"
#include <vector>
#include <string>
#include <unistd.h>
using std::vector;


//Return the aggregate CPU utilization
/*
USING THE FOLLOWING ALGORITHM:
(FROM: https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux)

PrevIdle = previdle + previowait
Idle = idle + iowait

PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal
NonIdle = user + nice + system + irq + softirq + steal

PrevTotal = PrevIdle + PrevNonIdle
Total = Idle + NonIdle

# differentiate: actual value minus the previous one
totald = Total - PrevTotal
idled = Idle - PrevIdle

CPU_Percentage = (totald - idled)/totald

*/
double Processor::Utilization() {
    double cpuPercentage = 0.0;
    double nonIdle = LinuxParser::ActiveJiffies();
    double idle = LinuxParser::IdleJiffies();

    //Calcualate and return the CPU percentage
    double prevTotal = cached_idle_ticks_ + cached_active_ticks_;
    double total = idle + nonIdle;  
    double totalD = total - prevTotal;
    double idleD = idle - cached_idle_ticks_;

    cpuPercentage = (totalD - idleD) / totalD;

    //Store current usage as previous usage
    cached_active_ticks_ = nonIdle;
    cached_idle_ticks_ = idle;

    return cpuPercentage;
}
