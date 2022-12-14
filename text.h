#ifndef TEXT_H
#define TEXT_H

#include <opencv2/core.hpp>
#include <string>

#include "layer.h"

class Text{
public:
	struct Styling{
		int red;
		int green;
		int blue;
		int alpha;
		float fontSize;
		int maxCharWidth;
	};
private:
	bool characters[94][8][8];
	Styling styling;
public:
	Text() {}
	Text(std::string fontFile){
		cv::Mat fontImage = cv::imread("./Fonts/" + fontFile, cv::IMREAD_GRAYSCALE);
		if(!fontImage.empty()){
			for(int i = 0; i < 94; i++){
				int current_char_offset = i * 8;
				cv::Mat current_char = fontImage(cv::Rect(current_char_offset % 80, (current_char_offset / 80) * 8, 8, 8));
				for(int y = 0; y < 8; y++){
					for(int x = 0; x < 8; x++){
						characters[i][y][x] = current_char.at<uint8_t>(y, x) == 0;
					}
				}
			}
			printf("Read font file: %s\n", fontFile.c_str());
		}
		else{
			printf("ERROR: Font file not found\n");
		}

		styling.red = 255;
		styling.green = 255;
		styling.blue = 255;
		styling.alpha = 255;
		styling.fontSize = 1;
		styling.maxCharWidth = 128;
	}

	cv::Mat getLine(std::string text){
		cv::Mat wordImage(8, 8 * text.length(), CV_8UC4);
		for(int i = 0; i < text.length(); i++){
			for(int y = 0; y < 8; y++){
				for(int x = 0; x < 8; x++){
					wordImage.at<cv::Vec4b>(y, x + i * 8) =
						characters[text[i]-32][y][x] ?
						cv::Vec4b(styling.blue, styling.green, styling.red, styling.alpha) :
						cv::Vec4b(0, 0, 0, 0);
				}
			}
		}
		return wordImage;
	}

	cv::Mat getText(std::string text){
		int xDim = 0, yDim = 1, curr_x = 0, delimPos = 0;
		std::vector<cv::Mat> wordList;
		std::string delim = " ";
		do{
			delimPos = text.find(delim);
			std::string word = delimPos > -1 ? text.substr(0, delimPos) : text;
			while(word.length() > styling.maxCharWidth){
				wordList.push_back(getLine(word.substr(0, styling.maxCharWidth)));
				yDim += 1;
				xDim = styling.maxCharWidth;
				curr_x = 0;
				word = word.substr(styling.maxCharWidth + 1, word.length());
				delimPos = text.find(delim); 
			}
			wordList.push_back(getLine(word));

			if(curr_x + word.length() >= styling.maxCharWidth){
				yDim += 1;
				curr_x = 0;
			}
			curr_x += word.length() + (delimPos > -1 ? 1 : 0);
			if(curr_x > xDim){
				xDim = curr_x;
			}
			if(delimPos > -1){
				text.erase(0, delimPos + 1);
			}
		}
		while(delimPos > -1);

		yDim *= 8;
		xDim *= 8;
		cv::Mat textImage(cv::Size(xDim, yDim), CV_8UC4, cv::Scalar(0));

		int xOff = 0, yOff = 0;
		//printf("Dim: %d x %d\n", xDim / 8, yDim / 8);
		for(cv::Mat word : wordList){
			if(xOff != 0 && (xOff + word.cols) / 8 >= styling.maxCharWidth){
				yOff += 8;
				xOff = 0;
			}
			//printf("%d -> %d, %d -> %d\n", xOff, word.cols, yOff, word.rows);
			word.copyTo(textImage(cv::Rect(xOff, yOff, word.cols, word.rows)));
			xOff += word.cols + 8;
		}

		return textImage; 
	}

	void setStyling(int r, int g, int b, int a, float s, int w){
		styling.red = r;
		styling.green = g;
		styling.blue = b;
		styling.alpha = a;
		styling.fontSize = s;
		styling.maxCharWidth = w;
	}
	void setStyling(Styling s){
		styling = s;
	}
	Styling getStyling(){
		return styling;
	}
};

#endif
