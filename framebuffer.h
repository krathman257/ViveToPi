#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <opencv2/core.hpp>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <fcntl.h>

#define TARGET_BPP 32

class Framebuffer{
private:
	int fbfd;
	char *fbp;
	long int screenSize;
	struct fb_var_screeninfo var_info;
	struct fb_fix_screeninfo fix_info;

public:
	Framebuffer() {}
	Framebuffer(const char *fbName){
		fbfd = 0;
		fbp = 0;
		screenSize = 0;

		printf("Opening framebuffer %s...\n", fbName);

		//Open framebuffer device
		fbfd = open(fbName, O_RDWR);
		if(!fbfd){
			printf("Error: Unable to open framebuffer device\n");
		}
		printf("Framebuffer device opened\n");
	
		//Get variable screen info, set bits per pixel to TARGET_BPP
		if(ioctl(fbfd, FBIOGET_VSCREENINFO, &var_info)){
			printf("Error: Unable to read variable screen info for device\n");
		}
		if(var_info.bits_per_pixel != TARGET_BPP){
			var_info.bits_per_pixel = TARGET_BPP;
			if(ioctl(fbfd, FBIOPUT_VSCREENINFO, &var_info)){
				printf("Error: Unable to change bits per pixel\n");
			}
			printf("Changed bits per pixel to %d\n", TARGET_BPP);
		}
	
		//Get fixed screen info
		if(ioctl(fbfd, FBIOGET_FSCREENINFO, &fix_info)){
			printf("Error: Unable to read fixed screen info for device\n");
		}
	
		//Map framebuffer to userspace memory
		screenSize = fix_info.smem_len;
		fbp = (char*)mmap(0, screenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	
		if((int)fbp == -1){
			printf("Error: Unable to memory map device\n");
		}
	
		printf("Framebuffer successfully initialized\n");
	}

	//Place row of color data
	void putRow(uchar* row, int x, int y, size_t size){
		int pix_offset = x * (TARGET_BPP / 8) + y * fix_info.line_length;

		memcpy((char*)(fbp+pix_offset), row, size);
	}

	//Change pixel (x, y) to color c
	void putPixel(int x, int y, cv::Vec4b c){
		int pix_offset = x * (TARGET_BPP / 8) + y * fix_info.line_length;
		
		*((char*)(fbp + pix_offset)) = c[0];
		*((char*)(fbp + pix_offset + 1)) = c[1];
		*((char*)(fbp + pix_offset + 2)) = c[2];
	}	

	//Release memory
	void closeFramebuffer(){
		munmap(fbp, screenSize);
		close(fbfd);
	}

	//Return variable info
	fb_var_screeninfo getVarInfo(){
		return var_info;
	}
};

#endif
