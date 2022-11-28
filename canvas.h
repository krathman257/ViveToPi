#include <opencv2/core.hpp>
#include <bitset>

#include "imageManager.h"
#include "framebuffer.h"
#include "camera.h"
#include "layer.h"

#include "helper.h"

#define PI 3.14159

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
	}

	//Draw camera to framebuffer
	void drawCamera(bool monitor = false, bool vive = true){
		draw(camera.readFrame(), monitor, vive);

		//auto t1 = getTime();
		//Layer frame = camera.readFrame();
		//auto t2 = getTime();
		//frame.resizeLayer(1.75);
		//auto t3 = getTime();
		//draw(frame, monitor, vive);
		/*auto t4 = getTime();
		printTime("Read Frame", t1, t2);
		printTime("Resize Frame", t2, t3);
		printTime("Draw Frame", t3, t4);
		printf("\n");*/
	}

	//Draw image from ImageManager
	void drawImage(std::string name, bool monitor = false, bool vive = true){
		draw(images.getImage(name), monitor, vive);
	}

	//Center Mat and draw to framebuffer
	void draw(Layer l, bool monitor = false, bool vive = true){
		cv::Mat m = l.getImage();

		int vive_y_offset = (1200 - m.rows) / 2;
		int vive_x_offset = (1080 - m.cols) / 2;
		int mon_x_offset = mon_xres - m.cols;

		drawPixel drawPixel_op(monitor, vive, fb_monitor, fb_vive, 0, mon_x_offset, vive_y_offset, vive_x_offset);
		m.forEach<cv::Point3_<uint8_t>>(drawPixel_op);
	}

	//Draw Pixel Functor
	struct drawPixel{
	private:
		bool use_monitor, use_vive;
		Framebuffer fb_monitor, fb_vive;
		int yOff_monitor, xOff_monitor,
		    yOff_vive, xOff_vive;
	public:
		drawPixel(bool m, bool v, Framebuffer fb_m, Framebuffer fb_v, int yom, int xom, int yov, int xov) :
			use_monitor(m), use_vive(v), fb_monitor(fb_m), fb_vive(fb_v),
			yOff_monitor(yom), xOff_monitor(xom), yOff_vive(yov), xOff_vive(xov) {}

		void operator()(cv::Point3_<uint8_t> &p, const int *pos) const {
			if(use_monitor){
				fb_monitor.putPixel(pos[1] + xOff_monitor, pos[0] + yOff_monitor, p);
			}
			if(use_vive){
				fb_vive.putPixel(pos[1] + xOff_vive, pos[0] + yOff_vive, p);
				fb_vive.putPixel(pos[1] + xOff_vive + 1080, pos[0] + yOff_vive, p);
			}
		}
	};

	//Fill framebuffer with color
	void fill(char r, char g, char b, int dev = 1){		
		Framebuffer *fb = getFramebuffer(dev);
		for(int y = 0; y < fb->getVarInfo().yres; y++){
			for(int x = 0; x < fb->getVarInfo().xres; x++){
				fb->putPixel(x, y, cv::Point3_<uint8_t>(b,g,r));
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
				fb->putPixel(x, y, cv::Point3_<uint8_t>(b,g,r));
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
