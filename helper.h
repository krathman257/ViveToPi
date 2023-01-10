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

//Functions to parse integers and floats from strings
bool canParseInteger(std::string word){
	try{
		std::stoi(word);
	}
	catch(std::out_of_range const &ex){ return false; }
	catch(std::invalid_argument const &ex){ return false; }
	return true;
}

int parseInteger(std::string word){
	int result = -1;
	try{
		result = std::stoi(word);
	}
	catch(std::out_of_range const &ex){ printf("Error: Cannot parse integer, out of range\n"); }
	catch(std::invalid_argument const &ex){ printf("Error: Cannot parse integer, invalid argument\n"); }
	return result;
}

bool canParseFloat(std::string word){
	try{
		std::stof(word);
	}
	catch(std::out_of_range const &ex){ return false; }
	catch(std::invalid_argument const &ex){ return false; }
	return true;
}

float parseFloat(std::string word){
	float result = -1;
	try{
		result = std::stof(word);
	}
	catch(std::out_of_range const &ex){ printf("Error: Cannot parse integer, out of range\n"); }
	catch(std::invalid_argument const &ex){ printf("Error: Cannot parse integer, invalid argument\n"); }
	return result;
}

//Print Invalid Command error message
void displayInvalidMessage(std::string message){
	printf("Invalid command: %s\n", message.c_str());
}

#endif
