//
// Created by cryptic on 1/30/23.
//

#ifndef TATSU_TIMER_HPP
#define TATSU_TIMER_HPP
#pragma once

#include <chrono>

static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> start;
static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> start1;
static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> stop;
static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> stop1;
static std::chrono::microseconds duration;
static std::chrono::milliseconds duration1;

#define TIMER_START() do { start = std::chrono::high_resolution_clock::now(); } while(false)
#define TIMER_START1() do { start1 = std::chrono::high_resolution_clock::now(); } while(false)
#define TIMER_PRINT() do { fmt::print(fg((fmt::color)0x00c200), "{0}: {1}: time: {2}μs\n", __PRETTY_FUNCTION__, __LINE__, duration.count()); } while(false)
#define TIMER_PRINT1() do { fmt::print(fg((fmt::color)0x00c200), "{0}: {1}: time: {2}ms\n", __PRETTY_FUNCTION__, __LINE__, duration1.count()); } while(false)
#define TIMER_STOP() do { stop = std::chrono::high_resolution_clock::now(); duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); TIMER_PRINT(); } while(false)
#define TIMER_STOP1() do { stop1 = std::chrono::high_resolution_clock::now(); duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop1 - start1); TIMER_PRINT1(); } while(false)
#define TIMER() do { TIMER_START(); TIMER_START1(); } while(false)
#define TIMER1() do { TIMER_STOP(); TIMER_STOP1(); } while(false)

#endif //TATSU_TIMER_HPP
