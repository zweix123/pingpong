#include <chrono>

inline long long getNanoEpochTime() {
  return std::chrono::system_clock::now().time_since_epoch() /
         std::chrono::nanoseconds(1);
}