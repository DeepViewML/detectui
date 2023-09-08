#pragma once

#include <chrono>
#include <string>

using Clock = std::chrono::high_resolution_clock;

class StopWatch
{
  public:
    void Start();
    void Stop();

    std::chrono::microseconds GetDuration();

    void LogDuration(const std::string &operationName);

  private:
    std::chrono::time_point<Clock> start;
    std::chrono::time_point<Clock> stop;
};