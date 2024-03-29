#ifndef LAYER_H
#define LAYER_H

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <algorithm>
#include <cmath>

#define SIMD_OPENCV_ENABLE
#include "Simd/SimdLib.hpp"

#include "text.h"
#include "helper.h"

class Layer{
private:
	cv::Mat image;
	std::string name = "";
public:
	Layer() {}
	Layer(cv::Mat i){
		setImage(i);
	}
	Layer(cv::Size size, cv::Vec4b color){
		cv::Mat i(size, CV_8UC4, color);
		setImage(i);
	}

	//Return copy of Layer
	Layer copy(){
		Layer result;
		result.setImage(image);
		result.setName(name);
		return result;
	}

	//Overlay with Layer
	void overlay(Layer top){
		overlay(top, (image.cols - top.getWidth()) / 2, (image.rows - top.getHeight()) / 2);
	}
	void overlay(Layer top, int top_x_offset, int top_y_offset){
		//Calculate variables
		int y_start = std::max(0, top_y_offset);
		int x_start = std::max(0, top_x_offset);
		int y_end = std::min(image.rows, top_y_offset + top.getHeight());
		int x_end = std::min(image.cols, top_x_offset + top.getWidth());

		int width = x_end - x_start;
		int height = y_end - y_start;

		if(width >= 0 && height >= 0){
			cv::Mat alpha;
	
			//Create Regions of Interest
			Simd::View<Simd::Allocator> vw_bot = image(cv::Rect(x_start, y_start, width, height));
			Simd::View<Simd::Allocator> vw_top = top.getImage()(cv::Rect(x_start-top_x_offset, y_start-top_y_offset, width, height));
	
			//Extract Alpha channel
			cv::extractChannel((cv::Mat)vw_top, alpha, 3);
			
			//Blend Alpha
			Simd::AlphaBlending(vw_top, (Simd::View<Simd::Allocator>)alpha, vw_bot);
		}
	}

	//Add text, centered on the screen
	void overlayText(std::string message, Text text){
		cv::Mat messageImage = text.getText(message);
		float fontScalar = text.getStyling().fontSize;
		resize(messageImage, messageImage, cv::Size(), fontScalar, fontScalar, cv::INTER_NEAREST);
		overlay(Layer(messageImage));
	}

	//Add text, at coordinates
	void overlayText(std::string message, Text text, int x, int y){
		cv::Mat messageImage = text.getText(message);
		float fontScalar = text.getStyling().fontSize;
		resize(messageImage, messageImage, cv::Size(), fontScalar, fontScalar, cv::INTER_NEAREST);
		overlay(Layer(messageImage), x, y);
	}

	//Resize Layer by dimensions
	void resizeLayer(int x_dim, int y_dim){
		resize(image, image, cv::Size(x_dim, y_dim), cv::INTER_NEAREST);
	}

	//Resize Layer by scale
	void resizeLayer(float scale){
		resize(image, image, cv::Size(), scale, scale, cv::INTER_NEAREST);
	}

	//Crop Layer in center
	void cropLayer(int width, int height){
		int x = (image.cols - width) / 2;
		int y = (image.rows - height) / 2;
		cropLayer(x, y, width, height);
	}

	//Crop Layer at given coordinates
	void cropLayer(int x, int y, int width, int height){
		cv::Mat temp = image(cv::Rect(x, y, width, height));
		temp.copyTo(image);
	}

	//Rotate Layer
	void rotateLayer(int angle){
		//Get center of image
		cv::Point2f centerPoint((image.cols - 1) / 2.0, (image.rows - 1) / 2.0);

		//Calculate rotation matrix
		cv::Mat rotation_mat = cv::getRotationMatrix2D(centerPoint, angle, 1.0);

		//Apply rotation matrix
		cv::warpAffine(image, image, rotation_mat, image.size());
	}



	//Set flat alpha value across image
	void setAlpha(float val){
		int alphaVal = 255.0 * val;
		for(int y = 0; y < image.rows; y++){
			for(int x = 0; x < image.cols; x++){
				image.at<cv::Vec4b>(y, x)[3] = alphaVal;
			}
		}
	}

	//Set alpha in circular pattern
	void setAlphaPattern_Circular(int inner, int outer, bool middle=true, int min_alpha=0, int max_alpha=255){
		int yCenter = image.rows / 2;
		int xCenter = image.cols / 2;
		for(int y = 0; y < image.rows; y++){
			for(int x = 0; x < image.cols; x++){
				float distFromCenter = sqrt(pow(x-xCenter, 2) + pow(y-yCenter, 2));
				float alpha_raw = (distFromCenter - inner) / (outer - inner);
				int alpha = std::max(min_alpha, std::min((int)(alpha_raw * 255.0), max_alpha));
				image.at<cv::Vec4b>(y, x)[3] = middle ? 255 - alpha : alpha;
			}
		}
	}

	//Get / Set functions
	void setImage(cv::Mat i){
		cv::cvtColor(i, image, cv::COLOR_BGR2BGRA);
	}
	cv::Mat getImage(){
		return image;
	}

	void setName(std::string n){
		name = n;
	}
	std::string getName(){
		return name;
	}

	int getHeight(){
		return image.rows;
	}
	int getWidth(){
		return image.cols;
	}
};

#endif
