#include <stdlib.h>
#include <thread>

#include "canvas.h"
#include "terminal.h"
#include "terminal_functions.h"

int main(int, char**){
	//Clear terminal
	system("clear");

	std::mutex d;
	std::mutex n;
	std::mutex l;
	priorityMutex mut;
	mut.data = &d;
	mut.next = &n;
	mut.low = &l;
	//std::mutex mut;
	bool run = true;

	//Initialize objects
	Canvas canvas("/dev/fb1", "/dev/fb0", 0, "./Images/", "font2.png", true, true);
	TerminalFunctions terminalFunctions(&canvas, &run);
	Terminal terminal("./Terminal/instructions.term", &terminalFunctions, mut);

	//Clear Vive framebuffer
	canvas.fill(0, 0, 0);

	std::thread th(&Terminal::terminalThread, terminal, &run);

	//Draw camera
	while(run){
		lowPriorityLock(mut);
		//mut.lock();
		terminalFunctions.processInstructions();
		lowPriorityUnlock(mut);
		//mut.unlock();
	}

	th.join();

	//Cleanup
	std::cout << "Closing ViveToPi..." << std::endl;
	canvas.closeAll();
	return 0;
}
