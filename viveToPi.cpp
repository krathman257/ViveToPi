#include <stdlib.h>
#include <thread>

#include "canvas.h"
#include "terminal.h"
#include "terminal_functions.h"

int main(int, char**){
	//Clear terminal
	system("clear");

	std::mutex mut;
	bool run = true;

	//Initialize canvas
	std::vector<std::vector<std::string>> instructionList;
	Canvas canvas("/dev/fb1", "/dev/fb0", 0, "./Images/", "font2.png", true, false, &mut);
	TerminalFunctions terminalFunctions(&canvas, &instructionList, &mut, &run);
	Terminal terminal("./Terminal/instructions.term", &terminalFunctions);

	//Clear Vive framebuffer
	canvas.fill(0, 0, 0);

	std::thread th(&Terminal::terminalThread, terminal, &run);

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
