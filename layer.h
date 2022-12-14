#ifndef LAYER_H
#define LAYER_H

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <algorithm>

#include "helper.h"

class Layer{
private:
	cv::Mat image;
public:
	Layer() {}
	Layer(cv::Mat i){
		setImage(i);
	}

	//Overlay with Layer
	void overlay(Layer top){
		int offsets[6];
		offsets[0] = (image.rows - top.getHeight()) / 2; //Top Y Offset
		offsets[1] = (image.cols - top.getWidth()) / 2; //Top X Offset

		offsets[2] = std::max(0, offsets[0]); //Y Start
		offsets[3] = std::max(0, offsets[1]); //X Start
		offsets[4] = std::min(image.rows, offsets[0] + top.getWidth()); //Y End
		offsets[5] = std::min(image.cols, offsets[1] + top.getHeight()); //X End

		//auto start = getTime();

		/*for(int y = offsets[2]; y < offsets[4]; y++){
			for(int x = offsets[3]; x < offsets[5]; x++){
				addColorAlpha(
					&(image.at<cv::Point3_<uint8_t>>(y,x)),
					top.getImage().at<cv::Point3_<uint8_t>>(y-offsets[0],x-offsets[1]),
					top.getAlpha(),
					x, y
				);
			}
		}*/
		alphaOperator alpha_op(top.getImage(), offsets);
		image.forEach<cv::Vec4b>(alpha_op);

		//auto stop = getTime();
		//printTime("Time to overlay images:", start, stop);
	}

	//Alpha Functor
	struct alphaOperator{
	private:
		cv::Mat top_image;
		int top_y_offset, top_x_offset,
		    y_start, x_start,
		    y_end, x_end;
	public:
		alphaOperator(cv::Mat ti, int *offsets) : 
			top_image(ti),
			top_y_offset(offsets[0]), top_x_offset(offsets[1]),
			y_start(offsets[2]), x_start(offsets[3]),
			y_end(offsets[4]), x_end(offsets[5]) {}
		
		void operator()(cv::Vec4b &p, const int *pos) const {
			if(pos[0] >= y_start && pos[0] < y_end && pos[1] >= x_start && pos[1] < x_end){
				cv::Vec4b top_color = top_image.at<cv::Vec4b>(pos[0]-top_y_offset, pos[1]-top_x_offset);
				float alphaMult = (float)top_color[3] / 255.0;
				p[0] = p[0] * (1.0 - alphaMult) + top_color[0] * alphaMult;
				p[1] = p[1] * (1.0 - alphaMult) + top_color[1] * alphaMult;
				p[2] = p[2] * (1.0 - alphaMult) + top_color[2] * alphaMult;
			}
		}
	};

	//Resize Layer by dimensions
	void resizeLayer(int x_dim, int y_dim){
		//cv::Size s(x_dim, y_dim);
		resize(image, image, cv::Size(x_dim, y_dim), cv::INTER_LINEAR);
	}

	//Resize Layer by scale
	void resizeLayer(float scale){
		resize(image, image, cv::Size(), scale, scale, cv::INTER_LINEAR);
	}

	void cropLayer(int width, int height){
		int x = (image.cols - width) / 2;
		int y = (image.rows - height) / 2;
		cropLayer(x, y, width, height);
	}

	void cropLayer(int x, int y, int width, int height){
		cv::Mat temp = image(cv::Rect(x, y, width, height));
		temp.copyTo(image);
	}

	//Rotate Layer
	//NOTE: Crops to original dimensions. Fix?
	void rotateLayer(int angle){
		//Get center of image
		cv::Point2f centerPoint((image.cols - 1) / 2.0, (image.rows - 1) / 2.0);

		//Calculate rotation matrix
		cv::Mat rotation_mat = cv::getRotationMatrix2D(centerPoint, angle, 1.0);

		//Apply rotation matrix
		cv::warpAffine(image, image, rotation_mat, image.size());
	}

	//Add pixels with alpha
	/*void addColorAlpha(cv::Point3_<uint8_t> *bottom, cv::Point3_<uint8_t> top, cv::Mat top_alpha, int x, int y){
		float a = top_alpha.at<float>(y, x);
		bottom->x = bottom->x * (1.0 - a) + top.x * a;
		bottom->y = bottom->y * (1.0 - a) + top.y * a;
		bottom->z = bottom->z * (1.0 - a) + top.z * a;
	}*/

	void setImage(cv::Mat i){
		cv::cvtColor(i, image, cv::COLOR_BGR2BGRA);
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

	void setAlphaPattern_Circular(int inner, int outer){
		int yCenter = image.rows / 2;
		int xCenter = image.cols / 2;
		for(int y = 0; y < image.rows; y++){
			for(int x = 0; x < image.cols; x++){
				float distFromCenter = sqrt(pow(x-xCenter, 2) + pow(y-yCenter, 2));
				float alpha_raw = (distFromCenter - inner) / (outer - inner);
				image.at<cv::Vec4b>(y, x)[3] = std::max(0, std::min((int)(alpha_raw * 255.0), 255));
			}
		}
	}

	cv::Mat getImage(){ return image; }
	int getHeight(){ return image.rows; }
	int getWidth(){ return image.cols; }
};

#endif
