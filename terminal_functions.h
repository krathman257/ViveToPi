#ifndef TERMINAL_FUNCTIONS_H
#define TERMINAL_FUNCTIONS_H

#include <string>
#include <fstream>

#include "layer.h"

class TerminalFunctions{
private:
	struct Instruction{
		std::vector<std::string> command;
		std::vector<int> flags;
	};
	std::vector<Instruction> instructionList;
	Canvas *canvas;
	bool *run;

public:
	TerminalFunctions(Canvas *c, bool *r){
		canvas = c;
		run = r;

		loadInstructions("default");
	}

	void processInstructions(){
		std::vector<Layer> layers;

		for(Instruction inst : instructionList){
			if(containsFlag(inst, 20)){
				layers.push_back(processInstructions_getLayer(inst));
			}
			else if(containsFlag(inst, 21)){
				for(int i = 0; i < layers.size(); i++){
					if(layers[i].getName() == inst.command[1]){
						processInstructions_processLayer(inst, &layers, i);
						break;
					}
				}
			}
			else if(containsFlag(inst, 22)){
				for(int i = 0; i < layers.size(); i++){
					if(layers[i].getName() == inst.command[1]){
						canvas->draw(layers[i]);
						break;
					}
				}
			}
		}
	}

	Layer processInstructions_getLayer(Instruction inst){
		Layer result;
		
		for(int f : inst.flags){
			switch(f){
				//Camera
				case 201:
					result = canvas->getCameraFrame();
					break;
				//Image
				case 202:
					result = (canvas->getImageFrame(inst.command[3])).copy();
					break;
				default:
					break;
			}
		}
			
		result.setName(inst.command[1]);
		return result;
	}

	void processInstructions_processLayer(Instruction inst, std::vector<Layer> *layers, int index){
		std::string input = "";
		for(int f : inst.flags){
			switch(f){
				//Resize
				case 30:
					//Resize dimensions
					if(containsFlag(inst, 300)){
						(*layers)[index].resizeLayer(parseInteger(inst.command[4]), parseInteger(inst.command[5]));
					}
					//Resize scale
					else{
						(*layers)[index].resizeLayer(parseFloat(inst.command[4]));
					}
					break;
				//Rotate
				case 31:
					(*layers)[index].rotateLayer(parseInteger(inst.command[3]));
					break;
				//Alpha
				case 32:
					//Alpha flat
					if(containsFlag(inst, 320)){
						(*layers)[index].setAlpha(parseFloat(inst.command[4]));
					}
					//Alpha circular
					else{
						(*layers)[index].setAlphaPattern_Circular(parseInteger(inst.command[4]), parseInteger(inst.command[5]));
					}
					break;
				//Text
				case 33:
					for(int i = 3; i < inst.command.size(); i++){
						input += inst.command[i];
						if(i != inst.command.size() - 1){
							input += " ";
						}
					}
					(*layers)[index].overlayText(input, canvas->getText());
					break;
				//Overlay
				case 34:
					for(int i = 0; i < layers->size(); i++){
						if((*layers)[i].getName() == inst.command[3]){
							(*layers)[index].overlay((*layers)[i]);
							break;
						}
					}
					break;
				default:
					break;
			}
		}
	}

	void printInstructions(){
		int counter = 0;
		printf("****************\n"
		       "* INSTRUCTIONS *\n"
		       "****************\n");
		for(Instruction inst : instructionList){
			printf("%d) ", counter++);
			for(std::string com : inst.command){
				printf("%s ", com.c_str());
			}
			printf("\n");
		}
		printf("****************\n");
	}

	void processFlags(std::vector<std::string> command, std::vector<int> flags){
		Instruction inst{ command, flags };
		for(int f : flags){
			switch(f){
				case 0: //Exit
					exit(run);
					break;
				case 1: //Help
					displayHelpMessage();
					break;
				case 2: //Clear
					clearInstructions();
					break;
				case 3: //Load
					loadInstructions(inst);
					break;
				case 4: //Save
					saveInstructions(inst);
					break;
				case 5: //Change display
					setDisplayOutput(inst);
					break;
				case 6: //Print information
					printInfo(inst);
					break;
				case 10:
					pushInstruction(inst);
					refactorInstructions();
					break;
				case 12:
					editInstruction(inst);
					refactorInstructions();
					break;
				case 13:
					deleteInstruction(inst);
					refactorInstructions();
					break;
				default:
					//printf("Flag Found: %d\n", f);
					break;
			}
		}
	}

	//Clear instruction list
	void clearInstructions(){
		instructionList = { };
	}

	//Save current instruction list to file
	void saveInstructions(Instruction inst){
		std::ofstream saveFile;
		saveFile.open("./InstructionLists/" + inst.command[1] + ".inli");
		for(int i = 0; i < instructionList.size(); i++){
			for(int c = 0; c < instructionList[i].command.size(); c++){
				saveFile << instructionList[i].command[c];
				if(c != instructionList[i].command.size() - 1){
					saveFile << " ";
				}
			}
			saveFile << '\n';
			for(int f = 0; f < instructionList[i].flags.size(); f++){
				saveFile << instructionList[i].flags[f];
				if(f != instructionList[i].flags.size() - 1){
					saveFile << " ";
				}
			}
			saveFile << '\n';
		}
		saveFile.close();
		printf("Instruction List saved to file: ./InstructionLists/%s.inli\n", inst.command[1].c_str());
	}

	//Load instruction list from file
	void loadInstructions(Instruction inst){
		loadInstructions(inst.command[1]);
	}

	void loadInstructions(std::string filename){
		std::ifstream loadFile;
		loadFile.open("./InstructionLists/" + filename + ".inli");
		if(!loadFile.is_open()){
			printf("Unable to load ./InstructionLists/%s.inli\n", filename.c_str());
		}
		else{
			clearInstructions();
			std::vector<std::string> readFlags;
			std::string input;
			while(getline(loadFile, input)){
				Instruction inst;
				inst.command = splitString(input, " ");
				getline(loadFile, input);
				readFlags = splitString(input, " ");
				for(std::string f : readFlags){
					if(f != ""){
						inst.flags.push_back(std::stoi(f));
					}
				}
				instructionList.push_back(inst);
			}
			printf("Instruction List loaded from file: ./InstructionList/%s.inli\n", filename.c_str());
		}
		loadFile.close();
	}

	//Change which displays video is output to
	void setDisplayOutput(Instruction inst){

		//Parse boolean
		bool output = containsFlag(inst, 53);

		//Parse display output
		if(containsFlag(inst, 51)){
			canvas->setMonitorOutput(output);
		}
		else{
			canvas->setViveOutput(output);
		}
	}

	void pushInstruction(Instruction inst){
		int pushIndex = -1;
		bool indexGiven = containsFlag(inst, 11);
		if(indexGiven){
			pushIndex = parseInteger(inst.command[1]);
		}
		else{
			pushIndex = instructionList.size();
		}
		if(0 <= pushIndex && pushIndex <= instructionList.size()){
			std::vector<std::string> newCommand(inst.command.begin() + (indexGiven ? 2 : 1), inst.command.end());
			inst.command = newCommand;
			instructionList.insert(instructionList.begin() + pushIndex, inst);
		}
		else{
			displayInvalidMessage("Index out of range");
		}
	}

	void deleteInstruction(Instruction inst){
		int delIndex = -1;
		if(canParseInteger(inst.command[1])){
			delIndex = parseInteger(inst.command[1]);
			if(0 <= delIndex && delIndex < instructionList.size()){
				instructionList.erase(instructionList.begin() + delIndex);
			}
			else{
				displayInvalidMessage("Index out of range");
			}
		}
		else{
			displayInvalidMessage("Invalid index");
		}
	}

	void editInstruction(Instruction inst){
		int editIndex = -1;
		if(canParseInteger(inst.command[1])){
			editIndex = parseInteger(inst.command[1]);
			if(0 <= editIndex && editIndex < instructionList.size()){
				std::vector<std::string> newCommand(inst.command.begin() + 2, inst.command.end());
				inst.command = newCommand;
				instructionList.at(editIndex) = inst;
			}
			else{
				displayInvalidMessage("Index out of range");
			}
		}
		else{
			displayInvalidMessage("Invalid index");
		}
	}

	void refactorInstructions(){
		for(int i = 0; i < instructionList.size(); i++){
			//Layer flag
			if(containsFlag(instructionList[i], 20)){ 
				//Image flag
				if(containsFlag(instructionList[i], 202)){
					if(!(doesImageExist(instructionList[i]))){
						instructionList.erase(instructionList.begin() + i--);
						printf("Irrelevant instruction found, erasing\n");
					}
				}
			}

			//Process flag
			else if(containsFlag(instructionList[i], 21)){
				if((!isInstructionValid(instructionList[i], i)) ||
				   (!isInstructionValid(instructionList[i], i, 3) && containsFlag(instructionList[i], 34))){
					instructionList.erase(instructionList.begin() + i--);
					printf("Irrelevant instruction found, erasing\n");
				}
			}

			//Draw flag
			else if(containsFlag(instructionList[i], 22)){
				if(!isInstructionValid(instructionList[i], i)){
					instructionList.erase(instructionList.begin() + i--);
					printf("Irrelevant instruction found, erasing\n");
				}
			}

			else{
				printf("Error: Unrecognized instruction: %s\n", instructionList[i].command[0].c_str());
			}
		}
	}

	bool isInstructionValid(Instruction inst, int listInd, int commandInd=1){
		std::string layerName = inst.command[commandInd];
		for(int i = 0; i < listInd; i++){
			if(containsFlag(instructionList[i], 20) && instructionList[i].command[1] == layerName){
				return true;
			}
		}
		return false;
	}

	bool containsFlag(Instruction inst, int query){
		for(int f : inst.flags){
			if(f == query){
				return true;
			}
		}
		return false;
	}

	bool doesImageExist(Instruction inst){
		return canvas->getImageManager().doesImageExist(inst.command[inst.command.size()-1]);
	}

	void printInfo(Instruction inst){
		if(containsFlag(inst, 61)){
			printInstructions();
		}
	}

	void printInstruction(Instruction inst){
		for(std::string w : inst.command){
			printf("%s ", w.c_str());
		}
		printf("\n");
	}

	void exit(bool *run){
		*run = false;
		printf("Exiting program. Goodbye...\n");
	}

	void displayHelpMessage(){
		printf("*************************************************************\n"
		       "* HELP                                                      *\n"
		       "*************************************************************\n"
		       "* help -> Display this message                              *\n"
		       "* print instructions -> Print the current instruction list  *\n"
		       "* clear -> Delete all instructions in the instruction list  *\n"
		       "* save [NAME] -> Save the current instruction list in       *\n"
		       "*                file [NAME].inli                           *\n"
		       "* load [NAME] -> Load an instruction list from file         *\n"
		       "*                [NAME].inli                                *\n"
		       "* push [INSTRUCTION] -> Add a new instruction at the end of *\n"
		       "*                       the list                            *\n"
		       "* push [#] [INSTRUCTION] -> Push a new instruction directly *\n"
		       "*                          at Instruction [#]               *\n"
		       "* edit [#] [INSTRUCTION] -> Edit Instruction [#]            *\n"
		       "* delete [#] -> Delete Instruction [#]                      *\n"
		       "*                                                           *\n"
		       "* display [vive | monitor] [true | false] -> set the video  *\n"
		       "*                                            output         *\n"
		       "* exit -> Exit the program                                  *\n"
		       "*************************************************************\n"
		       "* INSTRUCTIONS                                              *\n"
		       "*************************************************************\n"
		       "* layer [NAME] camera -> Grab the most recent camera frame  *\n"
		       "* layer [NAME] image [FILE] -> Load a .PNG file             *\n"
		       "*                                                           *\n"
		       "* process [NAME] resize [ dimensions [INT] [INT] | scale    *\n"
		       "*				[FLT] ] -> Resize a layer   *\n"
		       "* process [NAME] rotate [FLT] -> Rotate a layer to an angle *\n"
		       "* process [NAME] alpha [ flat [INT] | circular [INT]        *\n"
		       "*               [INT] ] -> Set a flat alpha, or a circular  *\n"
		       "*               patterned alpha (inner, outer)              *\n"
		       "* process [NAME] text [STR] -> Print text on a layer        *\n"
		       "* process [NAME] overlay [NAME] -> Overlay a layer onto     *\n"
		       "*                                    another layer          *\n"
		       "*                                                           *\n"
		       "* draw [NAME] -> Draw a layer to the selected outputs       *\n"
		       "*************************************************************\n");
	}
};

#endif
