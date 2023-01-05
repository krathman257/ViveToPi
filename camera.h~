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
		//cap.read(rawFrame);
		readFrame();

		std::cout << "Capture Device API: " << cap.getBackendName() << std::endl;
		std::cout << "Capture Device Format ID: " << cap.get(cv::CAP_PROP_FORMAT) << std::endl;
		std::cout << "Capture Device Framerate: " << cap.get(cv::CAP_PROP_FPS) << std::endl;
		std::cout << "Raw Frame Width X Height: " << rawFrame.cols << " X " << rawFrame.rows << std::endl;
		std::cout << "Raw Frame Channels: " << rawFrame.channels() << std::endl;
		std::cout << "Raw Frame Depth ID: " << rawFrame.depth() << std::endl;
		std::cout << "Processed Frame Width X Height: " << frame.getImage().cols << " X " << frame.getImage().rows << std::endl;
		std::cout << "Processed Frame Channels: " << frame.getImage().channels() << std::endl;
		std::cout << "Processed Frame Depth ID: " << frame.getImage().depth() << std::endl;
	}

	//Release camera
	void closeCamera(){
		cap.release();
	}
};

#endif
