#include <stdlib.h>
#include <thread>

#include "canvas.h"
#include "terminal.h"
#include "terminal_functions.h"

int main(int, char**){
	//Clear terminal
	system("clear");

	std::mutex d, n, l;
	priorityMutex mut {&d, &n, &l};

	bool run = true;

	//Initialize objects
	Canvas canvas("/dev/fb1", "/dev/fb0", 0, "./Images/", "font2.png", true, true);
	TerminalFunctions terminalFunctions(&canvas, &run);
	Terminal terminal("./Terminal/instructions.term", &terminalFunctions, mut);

	//Clear Vive framebuffer
	canvas.clear();

	//Start Terminal thread
	std::thread th(&Terminal::terminalThread, terminal, &run);

	//Draw camera
	while(run){
		lowPriorityLock(mut);
		terminalFunctions.processInstructions();
		lowPriorityUnlock(mut);
	}

	th.join();

	//Cleanup
	printf("Closing ViveToPi...\n");
	canvas.closeAll();
	return 0;
}
