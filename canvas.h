#ifndef CANVAS_H
#define CANVAS_H

#include <opencv2/core.hpp>
#include <bitset>

#include "imageManager.h"
#include "framebuffer.h"
#include "camera.h"
#include "layer.h"
#include "text.h"

#include "helper.h"

#define PI 3.14159
#define MONITOR_SCALE 0.5

class Canvas{
private:
	Framebuffer fb_vive;
	Framebuffer fb_monitor;
	ImageManager images;
	Camera camera;
	Text text;

	int vive_xres, vive_xres_eye, vive_yres;
	int mon_xres, mon_yres;

	Framebuffer *getFramebuffer(int dev){
		return dev == 1 ? &fb_vive : &fb_monitor;
	}

	bool monitor, vive;
	Layer blankScreen;
	Layer test;
	int rotateSpeed = 5;
	Text::Styling textStyling;

public:
	//Constructor
	Canvas(std::string dev_dir_vive, std::string dev_dir_mon, int cam_dev, std::string imagePath, std::string textPath, bool m, bool v){
		//Initialize Framebuffers
		fb_vive = Framebuffer(dev_dir_vive.c_str());
		vive_xres = fb_vive.getVarInfo().xres;
		vive_xres_eye = vive_xres / 2;
		vive_yres = fb_vive.getVarInfo().yres;

		fb_monitor = Framebuffer(dev_dir_mon.c_str());
		mon_xres = fb_monitor.getVarInfo().xres;
		mon_yres = fb_monitor.getVarInfo().yres;

		//Initialize ImageManager
		images = ImageManager(imagePath);

		//Initialize Camera
		camera = Camera(cam_dev);
		camera.printInfo();

		//Initialize Text
		text = Text(textPath);
		textStyling = text.getStyling();
		textStyling.maxCharWidth = 15;
		textStyling.fontSize = 3;
		text.setStyling(textStyling);

		blankScreen = Layer(cv::Size(1080, 1200), cv::Vec4b(0, 0, 0, 255));
		setOutput(m, v);
	}

	/*
	//Draw camera to framebuffer
	void drawCamera(){
		Layer frame = camera.readFrame();
		frame.resizeLayer(2.0);
		draw(frame);
	}

	//Draw text on camera to framebuffer
	void drawText(std::string words){
		Layer frame = camera.readFrame();
		frame.overlayText(words, text);
		frame.resizeLayer(2.0);
		draw(frame);
	}

	//Draw image from ImageManager
	void drawImage(std::string name){
		draw(images.getImage(name));
	}*/

	Layer getCameraFrame(){
		return camera.readFrame();
	}

	Layer getImageFrame(std::string name){
		return images.getImage(name);
	}

	void draw(Layer l){
		draw(l, monitor, vive);
	}

	//Center Mat and draw to framebuffer
	void draw(Layer l, bool drawMonitor, bool drawVive){
		int vive_x_offset = (1080 - l.getWidth()) / 2;
		int vive_x_offset_right = vive_x_offset + 1080;
		int vive_y_offset = (1200 - l.getHeight()) / 2;
		int rect_x_start, rect_y_start, rect_width, rect_height;

		if(vive_x_offset < 0){
			rect_x_start = -1 * vive_x_offset;
			rect_width = 1080;
			vive_x_offset = 0;
			vive_x_offset_right = 1080;
		}
		else{
			rect_x_start = 0;
			rect_width = l.getWidth();
		}

		if(vive_y_offset < 0){
			rect_y_start = -1 * vive_y_offset;
			rect_height = 1200;
			vive_y_offset = 0;
		}
		else{
			rect_y_start = 0;
			rect_height = l.getHeight();
		}

		cv::Mat m = (l.getImage())(cv::Rect(rect_x_start, rect_y_start, rect_width, rect_height));	

		int rowSize = sizeof(cv::Vec4b) * m.cols;

		if(drawVive){
			for(int i = 0; i < m.rows; i++){
				fb_vive.putRow(m.ptr(i), vive_x_offset, vive_y_offset+i, rowSize);
				fb_vive.putRow(m.ptr(i), vive_x_offset_right, vive_y_offset+i, rowSize);
			}
		}
		if(drawMonitor){
			resize(m, m, cv::Size(), MONITOR_SCALE, MONITOR_SCALE, cv::INTER_NEAREST);
			int mon_x_offset = mon_xres - m.cols;
			rowSize = sizeof(cv::Vec4b) * m.cols;
			for(int i = 0; i < m.rows; i++){
				fb_monitor.putRow(m.ptr(i), mon_x_offset, i, rowSize);
			}
		}
	}

	void setMonitorOutput(bool m){ setOutput(m, vive); }
	void setViveOutput(bool v){ setOutput(monitor, v); }

	void setOutput(bool m, bool v){
		monitor = m;
		vive = v;
		if(!m){
			draw(blankScreen, true, false);
		}
		if(!v){
			draw(blankScreen, false, true);
		}
	}

	//Fill framebuffer with color
	void fill(char r, char g, char b, int dev = 1){		
		Framebuffer *fb = getFramebuffer(dev);
		for(int y = 0; y < fb->getVarInfo().yres; y++){
			for(int x = 0; x < fb->getVarInfo().xres; x++){
				fb->putPixel(x, y, cv::Vec4b(b,g,r,1));
			}
		}
	}

	//Fill rectangle with color
	void fill(char r, char g, char b, int x_start, int y_start, int width, int height, int dev = 1){
		Framebuffer *fb = getFramebuffer(dev);
		int y_end = std::min((int)(fb->getVarInfo().yres), y_start+height);
		int x_end = std::min((int)(fb->getVarInfo().xres), x_start+width);
		for(int y = y_start; y < y_end; y++){
			for(int x = x_start; x < x_end; x++){
				fb->putPixel(x, y, cv::Vec4b(b,g,r,1));
			}
		}
	}

	ImageManager getImageManager(){
		return images;
	}

	Text getText(){
		return text;
	}

	//Always run on exit
	void closeAll(){
		camera.closeCamera();
		fb_vive.closeFramebuffer();
		fb_monitor.closeFramebuffer();
	}
};

#endif
