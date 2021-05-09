#pragma once
#include <list>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <iostream>

struct Timer
{
	// Cherno style performance measurement functionality, from: https://www.youtube.com/watch?v=YG4jexlSAjc
	// Usage: instantiate a Timer struct inside the scope you want to measure.
	// upon exit from scope, it will output the time diff to the console.

	std::chrono::duration<float> duration;
	std::chrono::_V2::system_clock::time_point start, end;

	Timer()
	{
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		float ms = duration.count() * 1000.0f;
		std::cout << "Timer took " << ms << "ms\n";
	}
};