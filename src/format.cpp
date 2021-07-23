#include <string>

#include "format.h"

using std::string;
using std::to_string;

// TODO: Helper function to format the time correctly
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS

//Add a "0" to the start of the number string if it is a single digit
std::string Format::FormatTime(int i){
    std::string time = to_string(i);
    if(i < 10){time = "0"+to_string(i);}
    return time;
}
//Convert and separate the time into hrs, mins and secs
string Format::ElapsedTime(long int time) {
  std::string hours = Format::FormatTime(time / (60 * 60));
  std::string minutes = Format::FormatTime((time / 60) % 60);
  std::string seconds = Format::FormatTime(time % 60);

  return (hours + ":" + minutes + ":" + seconds);
}
