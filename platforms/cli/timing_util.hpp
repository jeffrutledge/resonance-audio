#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#ifdef REPORT_TIME

#define TIME_POINT(NAME)             \
  TIME_POINT_NAMES.push_back(#NAME); \
  TIME_POINTS.push_back(std::chrono::high_resolution_clock::now());

#define REPORT_TIME_DELTAS write_time_deltas2(TIME_POINT_NAMES, TIME_POINTS);

std::vector<std::string> TIME_POINT_NAMES;
std::vector<std::chrono::high_resolution_clock::time_point> TIME_POINTS;

bool write_time_deltas2(
    const std::vector<std::string>& time_point_names,
    const std::vector<std::chrono::high_resolution_clock::time_point>&
        time_points) {
  const size_t kNumericalWidth = 15;
  const size_t kNameWidth = 30;
  if (time_point_names.size() != time_points.size()) {
    return false;
  } else {
    // Header
    std::cout << std::setw(kNameWidth + 2) << "Interval Name |";
    std::cout << std::setw(kNumericalWidth + 2) << "last section |";
    std::cout << std::setw(kNumericalWidth + 2) << "from start |";
    std::cout << std::setw(kNumericalWidth + 2) << "percentage" << std::endl;

    // Data
    auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(
                          time_points.back() - time_points.front())
                          .count();
    for (size_t i = 1; i < time_point_names.size(); ++i) {
      auto last_section = std::chrono::duration_cast<std::chrono::microseconds>(
                              time_points[i] - time_points[i - 1])
                              .count();
      auto until_now = std::chrono::duration_cast<std::chrono::microseconds>(
                           time_points[i] - time_points[0])
                           .count();
      double percentage =
          static_cast<double>(last_section) / static_cast<double>(total_time);
      std::cout << std::setw(kNameWidth) << time_point_names[i] << " |";
      std::cout << std::setw(kNumericalWidth) << last_section << " |";
      std::cout << std::setw(kNumericalWidth) << until_now << " |";
      std::cout << std::setw(kNumericalWidth) << std::fixed << percentage * 100
                << std::endl;
    }
  }
  return true;
}
#else

#define TIME_POINT(NAME)

#endif
