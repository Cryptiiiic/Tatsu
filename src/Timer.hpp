//
// Created by cryptic on 1/30/23.
//

#ifndef TATSU_TIMER_HPP
#define TATSU_TIMER_HPP
#pragma once

#include <chrono>

static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> start;
static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> stop;
static std::chrono::microseconds duration;

#define TIMER_START() do { start = std::chrono::high_resolution_clock::now(); } while(false)
#define TIMER_PRINT() do { fmt::print(fg((fmt::color)0x00c200), "{0}: {1}: time: {2}μs\n", __PRETTY_FUNCTION__, __LINE__, duration.count()); } while(false)
#define TIMER_STOP() do { stop = std::chrono::high_resolution_clock::now(); duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); TIMER_PRINT(); } while(false)

#endif //TATSU_TIMER_HPP
