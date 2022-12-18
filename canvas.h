#include <opencv2/core.hpp>
#include <bitset>

#include "imageManager.h"
#include "framebuffer.h"
#include "camera.h"
#include "layer.h"

#include "helper.h"

#define PI 3.14159
#define MONITOR_SCALE 0.333

class Canvas{
private:
	Framebuffer fb_vive;
	Framebuffer fb_monitor;
	ImageManager images;
	Camera camera;

	int vive_xres, vive_xres_eye, vive_yres;
	int mon_xres, mon_yres;

	Framebuffer *getFramebuffer(int dev){
		return dev == 1 ? &fb_vive : &fb_monitor;
	}

	Layer test;
	int rotateSpeed = 5;

public:
	//Constructor
	Canvas(std::string dev_dir_vive, std::string dev_dir_mon, int cam_dev, std::string imagePath){
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

		test = images.getImage("image4.png");
		test.setAlphaPattern_Circular(10, 100, false, 100, 200);
	}

	//Draw camera to framebuffer
	void drawCamera(bool monitor = false, bool vive = true){
		//draw(camera.readFrame(), monitor, vive);

		//auto t1 = getTime();
		Layer frame = camera.readFrame();
		//auto t2 = getTime();
		//test.rotateLayer(rotateSpeed);
		//auto t3 = getTime();
		//frame.overlay(test);
		//auto t4 = getTime();
		frame.resizeLayer(2.7);
		//auto t5 = getTime();
		draw(frame, monitor, vive);
		/*auto t6 = getTime();
		printTime("Read Frame", t1, t2);
		printTime("Rotate Frame", t2, t3);
		printTime("Overlay Frame", t3, t4);
		printTime("Resize Frame", t4, t5);
		printTime("Draw Frame", t5, t6);
		printTime("Total Time", t1, t6);
		printf("\n");*/
	}

	//Draw image from ImageManager
	void drawImage(std::string name, bool monitor = false, bool vive = true){
		draw(images.getImage(name), monitor, vive);
	}

	//Center Mat and draw to framebuffer
	void draw(Layer l, bool monitor = false, bool vive = true){
		int vive_x_offset = (1080 - l.getWidth()) / 2;
		int vive_x_offset_right = vive_x_offset + 1080;
		int vive_y_offset = (1200 - l.getHeight()) / 2;
		int mon_x_offset = mon_xres - (1080 * MONITOR_SCALE);
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

		if(vive){
			for(int i = 0; i < m.rows; i++){
				fb_vive.putRow(m.ptr(i), vive_x_offset, vive_y_offset+i, rowSize);
				fb_vive.putRow(m.ptr(i), vive_x_offset_right, vive_y_offset+i, rowSize);
			}
		}
		if(monitor){
			resize(m, m, cv::Size(1080 * MONITOR_SCALE, 1200 * MONITOR_SCALE), cv::INTER_NEAREST);
			rowSize = sizeof(cv::Vec4b) * m.cols;
			for(int i = 0; i < m.rows; i++){
				fb_monitor.putRow(m.ptr(i), mon_x_offset, i, rowSize);
			}
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

	//Always run on exit
	void closeAll(){
		camera.closeCamera();
		fb_vive.closeFramebuffer();
		fb_monitor.closeFramebuffer();
	}
};
