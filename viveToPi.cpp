#include <stdlib.h>
#include <thread>

#include "canvas.h"
#include "terminal.h"

int main(int, char**){
	//Clear terminal
	system("clear");

	std::mutex mut;

	//Initialize canvas
	Canvas canvas("/dev/fb1", "/dev/fb0", 0, "./Images/", "font2.png", true, false, &mut);
	std::vector<std::vector<std::string>> instructionList;
	Terminal terminal;

	//Clear Vive framebuffer
	canvas.fill(0, 0, 0);

	bool run = true;

	std::thread th(&Terminal::terminalThread, terminal, &canvas, &instructionList, &run, &mut);

	//Draw camera
	while(run){
		canvas.processInstructions(instructionList);
	}

	th.join();

	//Cleanup
	std::cout << "Closing ViveToPi..." << std::endl;
	canvas.closeAll();
	return 0;
}
