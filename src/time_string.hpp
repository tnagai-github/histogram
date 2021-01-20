#include <ctime>
#include <sstream>
#include <string>

namespace misc 
  {
  void put_string_of_time(std::string &starting_date){
    std::time_t t = std::time(nullptr);
    std::stringstream ss ;
    ss<< std::put_time(std::localtime(&t), "%c %Z");
    starting_date+=ss.str();
  }
}

