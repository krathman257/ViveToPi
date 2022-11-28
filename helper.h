#ifndef HELPER_H
#define HELPER_H

#include <chrono>
#include <string>

//Functions to measure execution time
/* auto start = getTime();
 * [PROCESS]
 * auto stop = getTime();
 * printTime("Message", start, stop);
 */

std::chrono::time_point<std::chrono::high_resolution_clock> getTime(){
	return std::chrono::high_resolution_clock::now();
}

void printTime(std::string message, std::chrono::time_point<std::chrono::high_resolution_clock> startTime, std::chrono::time_point<std::chrono::high_resolution_clock> stopTime){
	std::chrono::duration<double> duration = stopTime - startTime;
	std::cout << message << ": " << duration.count() << " s" << std::endl;
}

#endif
