#ifndef TERMINAL_H
#define TERMINAL_H

#include "canvas.h"

class Terminal{
private:
	Canvas *canvas;
	std::map<std::string, int> commandList;
	std::vector<std::vector<std::string>> *instructionList;
	std::mutex *mut;
public:
	enum Command{
		PUSH,
		DELETE,
		DISPLAY,
		HELP,
		EXIT,
		EMPTY,
		EDIT,
		TRUE,
		FALSE,
		MONITOR,
		VIVE,
		INVALID,
		PRINT_INSTRUCTIONS
	};

	Terminal() {
		
		//Map input strings to commands
		commandList["push"] = PUSH;
		commandList["delete"] = DELETE;
		commandList["display"] = DISPLAY;
		commandList["help"] = HELP;
		commandList["exit"] = EXIT;
		commandList[""] = EMPTY;
		commandList["edit"] = EDIT;
		commandList["true"] = TRUE;
		commandList["false"] = FALSE;
		commandList["monitor"] = MONITOR;
		commandList["vive"] = VIVE;
		commandList["print_instructions"] = PRINT_INSTRUCTIONS;
	}

	void terminalThread(Canvas *c, std::vector<std::vector<std::string>> *il, bool *run, std::mutex *m){
		std::vector<std::string> command;
		std::string input = "";
		Command curr_command;
		instructionList = il;
		canvas = c;
		mut = m;

		displayWelcomeMessage();

		while(*run){
			printf("Please enter a command: ");
			std::getline(std::cin, input);
			command = splitString(input, " ");
			curr_command = getCommand(command[0]);

			switch(curr_command){
				case PUSH:
					pushInstruction(command);
					break;
				case DELETE:
					deleteInstruction(command);
					break;
				case DISPLAY:
					setDisplayOutput(command);
					break;
				case HELP:
					displayHelpMessage();
					break;
				case EXIT:
					exit(run);
					break;
				case EMPTY:
					break;
				case EDIT:
					editInstruction(command);
					break;
				case PRINT_INSTRUCTIONS:
					printInstructions();
					break;
				case INVALID:
					displayInvalidMessage("Command not found");
					break;
				default:
					break;
			}
		}
	}

	//Split input by delimiter
	std::vector<std::string> splitString(std::string input, std::string delim){
		std::vector<std::string> result;
		int delimPos = -1;

		//While delimiter found, push found segment
		while((delimPos = input.find(delim)) != std::string::npos){
			result.push_back(input.substr(0, delimPos));
			input.erase(0, delimPos + delim.length());
		}

		//Push last segment
		result.push_back(input);
		return result;
	}

	void pushInstruction(std::vector<std::string> command){
		if(command.size() > 1){
			std::size_t pushIndex = -1;
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
				if(isValidInstruction(instruction)){
					instructionList->insert(instructionList->begin() + pushIndex, instruction);
				}
				else{
					displayInvalidMessage("Invalid instruction");
				}
			}
			else{
				displayInvalidMessage("Index out of range");
			}
			mut->unlock();
		}
		else{
			displayInvalidMessage("Incorrect number of arguments");
		}
	}

	void deleteInstruction(std::vector<std::string> command){
		if(command.size() == 2){
			std::size_t delIndex = -1;
			try{
				delIndex = std::stoi(command[1]);
			}
			catch(std::out_of_range const &ex){}
			catch(std::invalid_argument const &ex){
				displayInvalidMessage("Invalid index");
			}
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
			displayInvalidMessage("Incorrect number of arguments");
		}

	}

	void editInstruction(std::vector<std::string> command){
		if(command.size() > 2){
			std::size_t editIndex = -1;
			try{
				editIndex = std::stoi(command[1]);
			}
			catch(std::out_of_range const &ex){}
			catch(std::invalid_argument const &ex){
				displayInvalidMessage("Invalid index");
			}
			mut->lock();
			if(0 <= editIndex && editIndex < instructionList->size()){
				std::vector<std::string> instruction(command.begin() + 2, command.end());
				if(isValidInstruction(instruction)){
					instructionList->at(editIndex) = instruction;
				}
				else{
					displayInvalidMessage("Invalid instruction");
				}
			}
			else{
				displayInvalidMessage("Index out of range");
			}
			mut->unlock();
		}
		else{
			displayInvalidMessage("Incorrect number of arguments");
		}
	}

	bool isValidInstruction(std::vector<std::string> instruction){
		return true;
	}

	void printInstructions(){
		canvas->printInstructions(*instructionList);
	}

	//Change which displays video is output to
	void setDisplayOutput(std::vector<std::string> command){

		//Verify command size
		if(command.size() == 3){

			//Parse boolean
			bool output, valid = true;
			Command curr_command;

			curr_command = getCommand(command[2]);
			switch(curr_command){
				case TRUE:
					output = true;
					break;
				case FALSE:
					output = false;
					break;
				default:
					valid = false;
					displayInvalidMessage("Command not recognized: " + command[2]);
					break;
			}

			//Parse display output
			if(valid){
				curr_command = getCommand(command[1]);
				switch(curr_command){
					case MONITOR:
						canvas->setMonitorOutput(output);
						break;
					case VIVE:
						canvas->setViveOutput(output);
						break;
					default:
						displayInvalidMessage("Command not recognized: " + command[1]);
						break;
				}
			}
		}
		else{
			displayInvalidMessage("Incorrect number of arguments");
		}
	}

	void exit(bool *run){
		*run = false;
		printf("Exiting program. Goodbye...\n");
	}

	void displayInvalidMessage(std::string message){
		printf("Invalid command: %s\n", message.c_str());
	}

	void displayWelcomeMessage(){
		printf("******************\n"
		       "* Welcome to the *\n"
		       "*   VIVE TO PI   *\n"
		       "*  System  Menu  *\n"
		       "******************\n"
		      );
	}

	void displayHelpMessage(){
		printf("*************************************************************\n"
		       "* HELP                                                      *\n"
		       "*************************************************************\n"
		       "* help -> Display this message                              *\n"
		       "* print_instructions -> Print the current instruction list  *\n"
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
		       "* Under Development                                         *\n"
		       "*************************************************************\n"
		      );
	}

	bool verifyCommand(std::string input, Command c){
		return commandList.find(input)->second == c;
	}

	Command getCommand(std::string input){
		std::map<std::string, int>::iterator position = commandList.find(input);
		if(position == commandList.end()){
			return INVALID;
		}
		return (Command)(position->second);
	}
};

#endif
