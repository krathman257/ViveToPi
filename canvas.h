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

	bool monitor, vive;
	Layer blankScreen;
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
		textStyling.maxCharWidth = 10;
		textStyling.fontSize = 3;
		text.setStyling(textStyling);

		//Initialize BlankScreen, set default display outputs
		blankScreen = Layer(cv::Size(1080, 1200), cv::Vec4b(0, 0, 0, 255));
		setOutput(m, v);
	}

	//Draw to framebuffer
	void draw(Layer l){
		draw(l, monitor, vive);
	}

	void draw(Layer l_raw, bool drawMonitor, bool drawVive){
		//Overlay layer onto BlankScreen
		Layer l = blankScreen.copy();
		l.overlay(l_raw);

		cv::Mat m = l.getImage();
		int rowSize;

		//Draw on Vive framebuffer
		if(drawVive){
			rowSize = sizeof(cv::Vec4b) * m.cols;
			for(int i = 0; i < m.rows; i++){
				fb_vive.putRow(m.ptr(i), 0, i, rowSize);
				fb_vive.putRow(m.ptr(i), 1080, i, rowSize);
			}
		}
		//Rescale and draw on Monitor framebuffer
		if(drawMonitor){
			resize(m, m, cv::Size(), MONITOR_SCALE, MONITOR_SCALE, cv::INTER_NEAREST);
			int mon_x_offset = mon_xres - m.cols;

			rowSize = sizeof(cv::Vec4b) * m.cols;
			for(int i = 0; i < m.rows; i++){
				fb_monitor.putRow(m.ptr(i), mon_x_offset, i, rowSize);
			}
		}
	}

	//Fill framebuffers with black
	void clear(bool monitor=true, bool vive=true){
		draw(blankScreen, monitor, vive);
	}

	//Set display outputs
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

	//Get methods
	Layer getImageFrame(std::string name){
		return images.getImage(name);
	}
	Layer getCameraFrame(){
		return camera.readFrame();
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
