#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <dirent.h>
#include <iostream>
#include <string>
#include <map>

#include "layer.h"

class ImageManager{
private:
	std::map<std::string, Layer> images = {};

	DIR *dpdf; //Directory
	struct dirent *epdf; //Found file
public:
	ImageManager() {}
	ImageManager(std::string path){
		cv::Mat temp;

		dpdf = opendir(path.c_str());
		if(dpdf != NULL){
			while(epdf = readdir(dpdf)){
				temp = cv::imread(path + (epdf->d_name));
				if(temp.data != NULL){
					images.insert(std::pair<std::string, Layer>(std::string(epdf->d_name), Layer(temp)));
					std::cout << 
						epdf->d_name << 
						" inserted, Raw Channels=" << temp.channels() << 
						", Raw DepthID=" << temp.depth() << 
					std::endl;
				}
			}
			std::cout << images.size() << " images added to ImageManager" << std::endl;
		}
		closedir(dpdf);
	}

	//Get image from map
	Layer getImage(std::string name){
		std::map<std::string, Layer>::iterator i = images.find(name);
		if(i == images.end()){
			std::cout << "Error: Image not found" << std::endl;
			return Layer();
		}
		return i->second;
	}

	bool doesImageExist(std::string name){
		std::map<std::string, Layer>::iterator i = images.find(name);
		return i != images.end();
	}
};

#endif
