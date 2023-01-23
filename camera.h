#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <iostream>

#include "layer.h"

class Camera{
private:
	cv::VideoCapture cap;
	cv::Mat rawFrame;

	Layer frame;

	int deviceID;
	int apiID;
public:
	Camera() {}
	Camera(int did){
		deviceID = did;
		apiID = cv::CAP_ANY;

		cap.open(deviceID, apiID);

		if(!cap.isOpened()){
			printf("Error: Unable to open camera %d\n", deviceID);
		}

		cap.read(rawFrame);

		frame = Layer(rawFrame);
	}

	//Read and return next frame
	Layer readFrame(){
		cap.read(rawFrame);
		frame.setImage(rawFrame);
		return frame;
	}

	//Print camera and frame properties
	void printInfo(){
		readFrame();

		printf("Capture Device API: %s\n", cap.getBackendName().c_str());
		printf("Capture Device Format ID: %d\n", cap.get(cv::CAP_PROP_FORMAT));
		printf("Frame Width X Height (Raw, Processed): %d X %d, %d X %d\n", 
			rawFrame.cols, rawFrame.rows, frame.getImage().cols, frame.getImage().rows);
		printf("Frame Channels (Raw, Processed): %d, %d\n", rawFrame.channels(), frame.getImage().channels());
		printf("Frame Depth ID (Raw, Processed): %d, %d\n", rawFrame.depth(), frame.getImage().depth());
	}

	//Release camera
	void closeCamera(){
		cap.release();
	}
};

#endif
