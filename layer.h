#ifndef LAYER_H
#define LAYER_H

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <algorithm>

#include "helper.h"

class Layer{
private:
	cv::Mat image;
	cv::Mat alpha;
public:
	Layer() {}
	Layer(cv::Mat i){
		image = i;
		alpha = cv::Mat(image.rows, image.cols, CV_32F, 1.0f);
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
		alphaOperator alpha_op(top.getImage(), top.getAlpha(), offsets);
		image.forEach<cv::Point3_<uint8_t>>(alpha_op);

		//auto stop = getTime();
		//printTime("Time to overlay images:", start, stop);
	}

	//Alpha Functor
	struct alphaOperator{
	private:
		cv::Mat top_image, top_alpha;
		int top_y_offset, top_x_offset,
		    y_start, x_start,
		    y_end, x_end;
	public:
		alphaOperator(cv::Mat ti, cv::Mat ta, int *offsets) : 
			top_image(ti), top_alpha(ta),
			top_y_offset(offsets[0]), top_x_offset(offsets[1]),
			y_start(offsets[2]), x_start(offsets[3]),
			y_end(offsets[4]), x_end(offsets[5]) {}
		
		void operator()(cv::Point3_<uint8_t> &p, const int *pos) const {
			if(pos[0] >= y_start && pos[0] < y_end && pos[1] >= x_start && pos[1] < x_end){
				float a = top_alpha.at<float>(pos[0]-top_y_offset, pos[1]-top_x_offset);
				cv::Point3_<uint8_t> top_color = top_image.at<cv::Point3_<uint8_t>>(pos[0]-top_y_offset, pos[1]-top_x_offset);
				p.x = p.x * (1.0 - a) + top_color.x * a;
				p.y = p.y * (1.0 - a) + top_color.y * a;
				p.z = p.z * (1.0 - a) + top_color.z * a;
			}
		}
	};

	//Resize Layer by dimensions
	void resizeLayer(int x_dim, int y_dim){
		cv::Size s(x_dim, y_dim);
		resize(image, image, s, cv::INTER_LINEAR);
		resize(alpha, alpha, s, cv::INTER_LINEAR);
	}

	//Resize Layer by scale
	void resizeLayer(float scale){
		//auto t1 = getTime();
		resize(image, image, cv::Size(), scale, scale, cv::INTER_LINEAR);
		//auto t2 = getTime();
		resize(alpha, alpha, cv::Size(), scale, scale, cv::INTER_LINEAR);
		/*auto t3 = getTime();
		printTime("Resize Image", t1, t2);
		printTime("Resize Alpha", t2, t3);
		printf("\n");*/
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
		cv::warpAffine(alpha, alpha, rotation_mat, alpha.size());
	}

	//Add pixels with alpha
	void addColorAlpha(cv::Point3_<uint8_t> *bottom, cv::Point3_<uint8_t> top, cv::Mat top_alpha, int x, int y){
		float a = top_alpha.at<float>(y, x);
		bottom->x = bottom->x * (1.0 - a) + top.x * a;
		bottom->y = bottom->y * (1.0 - a) + top.y * a;
		bottom->z = bottom->z * (1.0 - a) + top.z * a;
	}

	//Set image (old and new images must have the same dimensions)
	void setImage(cv::Mat i){
		if(i.cols != image.cols || i.rows != image.rows){
			printf("Error: Unable to set Layer Image, dimensions must be the same\n");
		}
		else{
			image = i;
		}
	}

	//Set flat alpha value across image
	void setAlpha(float val){
		for(int y = 0; y < alpha.rows; y++){
			for(int x = 0; x < alpha.cols; x++){
				alpha.at<float>(y, x) = val;
			}
		}
	}

	void setAlphaPattern_Circular(int inner, int outer){
		int yCenter = alpha.rows / 2;
		int xCenter = alpha.cols / 2;
		for(int y = 0; y < alpha.rows; y++){
			for(int x = 0; x < alpha.cols; x++){
				float distFromCenter = sqrt(pow(x-xCenter, 2) + pow(y-yCenter, 2));
				float alpha_raw = (distFromCenter - inner) / (outer - inner);
				alpha.at<float>(y, x) = std::max(0.0f, std::min(alpha_raw, 1.0f));
			}
		}
	}

	cv::Mat getImage(){ return image; }
	cv::Mat getAlpha(){ return alpha; }
	int getHeight(){ return image.rows; }
	int getWidth(){ return image.cols; }
};

#endif
