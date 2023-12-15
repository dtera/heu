//
// Created by HqZhao on 2023/11/16.
//

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

class StopWatch {
 public:
  StopWatch(bool mark_defaut = false) {
    if (mark_defaut) {
      Mark();
    }
  }

  // start a new mark
  inline void Mark(const std::string &mark = default_mark()) {
    mark_[mark] = std::chrono::high_resolution_clock::now();
  }

  // return a mark's cost , default in nanoseconds
  inline long long CountNano(const std::string &mark = default_mark()) {
    return CountLL<std::nano>(mark);
  }

  // return a mark's cost , default in microseconds
  inline long long CountMicro(const std::string &mark = default_mark()) {
    return CountLL<std::micro>(mark);
  }

  // return a mark's cost , default in milliseconds
  inline long long CountMilli(const std::string &mark = default_mark()) {
    return CountLL<std::milli>(mark);
  }

  // return a mark's cost , default in seconds
  inline long long CountSeconds(const std::string &mark = default_mark()) {
    return CountDuration<long long, std::chrono::seconds>(mark);
  }

  // return a mark's cost , default in minutes
  inline long CountMinutes(const std::string &mark = default_mark()) {
    return CountDuration<long, std::chrono::minutes>(mark);
  }

  // return a mark's cost , default in hours
  inline long CountHours(const std::string &mark = default_mark()) {
    return CountDuration<long, std::chrono::hours>(mark);
  }

  // return a mark's cost in nanoseconds
  inline std::string ShowTickNano(const std::string &mark = default_mark()) {
    return std::to_string(CountNano(mark)) + "ns";
  }

  // return a mark's cost in microseconds
  inline std::string ShowTickMicro(const std::string &mark = default_mark()) {
    return std::to_string(CountNano(mark) / 1000.0) + "ws";
  }

  // return a mark's cost in milliseconds
  inline std::string ShowTickMills(const std::string &mark = default_mark()) {
    return std::to_string(CountMicro(mark) / 1000.0) + "ms";
  }

  // return a mark's cost in seconds
  inline std::string ShowTickSeconds(const std::string &mark = default_mark()) {
    return std::to_string(CountMilli(mark) / 1000.0) + "s";
  }

  // return a mark's cost in minutes
  inline std::string ShowTickMinutes(const std::string &mark = default_mark()) {
    return std::to_string(CountSeconds(mark) / 60.0) + "m";
  }

  // return a mark's cost in hours
  inline std::string ShowTickHours(const std::string &mark = default_mark()) {
    return std::to_string(CountMinutes(mark) / 60.0) + "h";
  }

  // remove mark and return its cost
  inline double Stop(const std::string &mark = default_mark()) {
    double cost = CountMilli(mark);
    mark_.erase(mark);
    return cost;
  }

  template <typename _Fp>
  void Print(_Fp fp, const std::string &mark = default_mark()) {
    auto show = std::bind(fp, this, std::placeholders::_1);
    std::cout << mark << " costs " << show(mark) << std::endl;
  }

  void PrintWithNano(const std::string &mark = default_mark()) {
    Print(&StopWatch::ShowTickNano, mark);
  }

  void PrintWithMicro(const std::string &mark = default_mark()) {
    Print(&StopWatch::ShowTickMicro, mark);
  }

  void PrintWithMills(const std::string &mark = default_mark()) {
    Print(&StopWatch::ShowTickMills, mark);
  }

  void PrintWithSeconds(const std::string &mark = default_mark()) {
    Print(&StopWatch::ShowTickSeconds, mark);
  }

  void PrintWithMinutes(const std::string &mark = default_mark()) {
    Print(&StopWatch::ShowTickMinutes, mark);
  }

  void PrintWithHours(const std::string &mark = default_mark()) {
    Print(&StopWatch::ShowTickHours, mark);
  }

 protected:
  static inline std::string &default_mark() {
    static std::string inst("__default__");
    return inst;
  }

  // return a mark's cost
  template <typename _Rep, typename _Duration>
  inline _Rep CountDuration(const std::string &mark = default_mark()) {
    return std::chrono::duration_cast<_Duration>(
               std::chrono::high_resolution_clock::now() - mark_[mark])
        .count();
  }

  // return a mark's cost
  template <typename _Period>
  inline long long CountLL(const std::string &mark = default_mark()) {
    return CountDuration<long long, std::chrono::duration<long long, _Period>>(
        mark);
  }

  std::unordered_map<std::string,
                     std::chrono::high_resolution_clock::time_point>
      mark_;
};
