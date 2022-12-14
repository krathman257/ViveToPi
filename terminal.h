#ifndef TERMINAL_H
#define TERMINAL_H

#include <iostream>
#include <fstream>

#include "terminal_functions.h"
#include "helper.h"

class Terminal{
private:
	struct CommandTreeNode{
		std::string command;
		int flag = -1;
		std::vector<CommandTreeNode> nextCommands;
	};

	TerminalFunctions *functions;
	CommandTreeNode command_root;

public:
	Terminal(std::string commandFile, TerminalFunctions *tf) {
		functions = tf;
		command_root = buildCommandTree(commandFile);
	}

	void terminalThread(bool *run){
		std::vector<std::string> command;
		std::string input = "";

		displayWelcomeMessage();

		while(*run){
			printf("Please enter a command: ");
			std::getline(std::cin, input);
			command = splitString(input, " ");
			parseCommand(command, command_root);
		}
	}

	CommandTreeNode buildCommandTree(std::string filename){
		CommandTreeNode result;
		std::fstream file;
		file.open(filename, std::ios::in);
		if(!file.is_open()){
			printf("Error: Unable to open file\n");
			return result;
		}
		std::vector<std::vector<std::string>> readLines;
		std::string line;
		while(getline(file, line)){
			if(line != "" && line[0] != '#'){
				readLines.push_back(splitString(line, " "));
			}
		}
		file.close();

		for(int i = 0; i < readLines.size(); i++){
			if(!isCommandTreeLabel(readLines[i][0])){
				result.nextCommands.push_back(buildCommandTreeNode(readLines, i, 0));
			}
		}

		printf("Command Tree successfully built\n");
		//printCommandTree(command_root);
		return result;
	}

	CommandTreeNode buildCommandTreeNode(std::vector<std::vector<std::string>> lines, int line_ind, int word_ind){
		CommandTreeNode result;
		int offset = 1;
		result.command = lines[line_ind][word_ind];
		if(word_ind == lines[line_ind].size() - offset){
			return result;
		}
		else{
			if(isCommandTreeFlag(lines[line_ind][word_ind + offset])){
				result.flag = parseInteger(lines[line_ind][word_ind + offset++].substr(1));
				if(word_ind == lines[line_ind].size() - offset){
					return result;
				}
			}
			if(isCommandTreeLabel(lines[line_ind][word_ind + offset])){
				std::vector<CommandTreeNode> nodes;
				nodes = buildCommandTreeNodes(lines, lines[line_ind][word_ind + offset]);
				for(CommandTreeNode ctn : nodes){
					result.nextCommands.push_back(ctn);
				}
			}
			else{
				result.nextCommands.push_back(buildCommandTreeNode(lines, line_ind, word_ind + offset));
			}
		}
		return result;
	}

	std::vector<CommandTreeNode> buildCommandTreeNodes(std::vector<std::vector<std::string>> lines, std::string label){
		std::vector<CommandTreeNode> result;
		for(int i = 0; i < lines.size(); i++){
			if(lines[i][0] == label){
				if(!isCommandTreeLabel(lines[i][1])){
					CommandTreeNode node;
					node = buildCommandTreeNode(lines, i, 1);
					result.push_back(node);
				}
				else{
					std::vector<CommandTreeNode> nodes;
					nodes = buildCommandTreeNodes(lines, lines[i][1]);
					for(CommandTreeNode ctn : nodes){
						result.push_back(ctn);
					}
				}
			}
		}
		return result;
	}

	void parseCommand(std::vector<std::string> command, CommandTreeNode ctn){
		std::vector<int> flags;
		bool invalid = false;
		for(std::string word : command){
			if(ctn.nextCommands.size() == 0){
				break;
			}
			if(!getNextNode(word, &ctn)){
				displayInvalidMessage("Command not recognized: " + word);
				invalid = true;
				break;
			}
			if(ctn.flag >= 0){
				flags.push_back(ctn.flag);
			}
		}
		if(ctn.nextCommands.size() != 0 && !invalid){
			displayInvalidMessage("Too few arguments");
			printf("Expected: ");
			for(CommandTreeNode c : ctn.nextCommands){
				printf("%s ", c.command.c_str());
			}
			printf("\n");
			invalid = true;
		}

		/*printf("Found flags: ");
		for(int f : flags){ printf("%d ", f); }
		printf("\n");*/

		if(!invalid){
			functions->processFlags(command, flags);
		}
	}

	bool getNextNode(std::string word, CommandTreeNode *ctn){
		if(ctn->nextCommands.size() == 0){ return false; }
		for(CommandTreeNode c : ctn->nextCommands){
			if(c.command == "INT" || c.command == "FLT" || c.command == "STR"){
				if(c.command == "INT" && canParseInteger(word)){
					*ctn = c;
					return true;
				}
				if(c.command == "FLT" && canParseFloat(word)){
					*ctn = c;
					return true;
				}	
				if(c.command == "STR"){
					*ctn = c;
					return true;
				}
			}
			else if(c.command == word){
				*ctn = c;
				return true;
			}
		}
		return false;
	}

	bool isCommandTreeLabel(std::string word){
		return word[0] == '[' && word[word.length()-1] == ']';
	}

	bool isCommandTreeFlag(std::string word){
		return word[0] == '/' && canParseInteger(word.substr(1));
	}

	void printCommandTree(CommandTreeNode ctn, int length=0, bool first=true){
		std::string label = ctn.command;
		if(label.length() > 0){
			for(int i = 0; i < (first ? 1 : length); i++){
				printf("-");
			}
		}
		if(ctn.flag >= 0){
			label += "(" + std::to_string(ctn.flag) + ")";
		}
		if(ctn.nextCommands.size() == 0){
			label += "\n";
		}
		printf("%s", label.c_str());

		for(int i = 0; i < ctn.nextCommands.size(); i++){
			printCommandTree(ctn.nextCommands[i], length+label.length()+1, i == 0);
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

	void displayWelcomeMessage(){
		printf("******************\n"
		       "* Welcome to the *\n"
		       "*   VIVE TO PI   *\n"
		       "*  System  Menu  *\n"
		       "******************\n"
		      );
	}
};

#endif
