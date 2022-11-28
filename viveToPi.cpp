#include "canvas.h"

int main(int, char**){
	
	//Initialize canvas
	Canvas canvas("/dev/fb1", "/dev/fb0", 0, "/home/pi/Desktop/ViveToPi/Images/");

	//Clear Vive framebuffer
	canvas.fill(0, 0, 0);

	//Draw background image
	//canvas.drawImage("image3.png");

	//Draw camera
	while(true){
		canvas.drawCamera();

		//Currently no way to break loop
	}

	//Cleanup
	std::cout << "Closing ViveToPi..." << std::endl;
	canvas.closeAll();
	return 0;
}
