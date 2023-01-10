#ifndef TERMINAL_FUNCTIONS_H
#define TERMINAL_FUNCTIONS_H

#include <string>

class TerminalFunctions{
private:
	Canvas *canvas;
	std::vector<std::vector<std::string>> *instructionList;
	std::mutex *mut;
	bool *run;
public:
	TerminalFunctions(Canvas *c, std::vector<std::vector<std::string>> *il, std::mutex *m, bool *r){
		canvas = c;
		instructionList = il;
		mut = m;
		run = r;
	}

	void processFlags(std::vector<std::string> command, std::vector<int> flags){
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
				case 5: //Change display
					setDisplayOutput(command);
					break;
				case 6: //Print information
					printInfo(command);
					break;
				case 10:
					pushInstruction(command);
					refactorInstructions();
					break;
				case 11:
					editInstruction(command);
					refactorInstructions();
					break;
				case 12:
					deleteInstruction(command);
					refactorInstructions();
					break;
				default:
					printf("Flag Found: %d\n", f);
					break;
			}
		}
	}

	//Clear instruction list
	void clearInstructions(){
		*instructionList = { };
	}

	//Change which displays video is output to
	void setDisplayOutput(std::vector<std::string> command){

		//Parse boolean
		bool output = (command[2] == "true");

		//Parse display output
		if(command[1] == "monitor"){
			canvas->setMonitorOutput(output);
		}
		else{
			canvas->setViveOutput(output);
		}
	}

	void pushInstruction(std::vector<std::string> command){
		int pushIndex = -1;
		bool indexGiven = false;
		try{
			pushIndex = std::stoi(command[1]);
			indexGiven = true;
		}
		catch(std::out_of_range const &ex){}
		catch(std::invalid_argument const &ex){
			mut->lock();
			pushIndex = instructionList->size();
			mut->unlock();
		}
		mut->lock();
		if(0 <= pushIndex && pushIndex <= instructionList->size()){
			std::vector<std::string> instruction(command.begin() + (indexGiven ? 2 : 1), command.end());
			instructionList->insert(instructionList->begin() + pushIndex, instruction);
		}
		else{
			displayInvalidMessage("Index out of range");
		}
		mut->unlock();
	}

	void deleteInstruction(std::vector<std::string> command){
		int delIndex = -1;
		if(canParseInteger(command[1])){
			delIndex = parseInteger(command[1]);
			mut->lock();
			if(0 <= delIndex && delIndex < instructionList->size()){
				instructionList->erase(instructionList->begin() + delIndex);
			}
			else{
				displayInvalidMessage("Index out of range");
			}
			mut->unlock();
		}
		else{
			displayInvalidMessage("Invalid index");
		}
	}

	void editInstruction(std::vector<std::string> command){
		int editIndex = -1;
		if(canParseInteger(command[1])){
			editIndex = parseInteger(command[1]);
			mut->lock();
			if(0 <= editIndex && editIndex < instructionList->size()){
				std::vector<std::string> instruction(command.begin() + 2, command.end());
				instructionList->at(editIndex) = instruction;
			}
			else{
				displayInvalidMessage("Index out of range");
			}
			mut->unlock();
		}
		else{
			displayInvalidMessage("Invalid index");
		}
	}

	void refactorInstructions(){
		mut->lock();
		for(int i = 0; i < instructionList->size(); i++){
			if((*instructionList)[i][0] == "layer"){ }
			else if((*instructionList)[i][0] == "process"){
				if(!isInstructionValid((*instructionList)[i][1], i)){
					instructionList->erase(instructionList->begin() + i--);
					printf("Irrelevant instruction found, erasing\n");
				}
				else if((*instructionList)[i][2] == "overlay" && !isInstructionValid((*instructionList)[i][3], i)){
					instructionList->erase(instructionList->begin() + i--);
					printf("Irrelevant instruction found, erasing\n");
				}
			}
			else if((*instructionList)[i][0] == "draw"){
				if(!isInstructionValid((*instructionList)[i][1], i)){
					instructionList->erase(instructionList->begin() + i--);
					printf("Irrelevant instruction found, erasing\n");
				}
			}
			else{
				printf("Error: Unrecognized instructions: %s\n", (*instructionList)[i][0].c_str());
			}
		}
		mut->unlock();
	}

	bool isInstructionValid(std::string layerName, int ind){
		for(int i = 0; i < ind; i++){
			if((*instructionList)[i][0] == "layer" && (*instructionList)[i][1] == layerName){
				return true;
			}
		}
		return false;
	}

	void printInfo(std::vector<std::string> command){
		if(command[1] == "instructions"){
			canvas->printInstructions(*instructionList);
		}
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
		       "* push [INSTRUCTION] -> Add a new instruction at the end of *\n"
		       "*                       the list                            *\n"
		       "* push [#] [INSTRUCTION] -> Add a new instruction directly  *\n"
		       "*                          after Instruction [#]            *\n"
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
		       "* process [NAME] rotate [FLT] -> Spin a layer at a speed    *\n"
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
