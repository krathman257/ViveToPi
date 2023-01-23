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
	priorityMutex mut;

public:
	Terminal(std::string commandFile, TerminalFunctions *tf, priorityMutex m) {
		functions = tf;
		command_root = buildCommandTree(commandFile);
		mut = m;
	}

	//Terminal Thread function
	void terminalThread(bool *run){
		std::vector<std::string> command;
		std::string input = "";

		functions->displayWelcomeMessage();

		while(*run){
			printf("Please enter a command: ");
			std::getline(std::cin, input);
			command = splitString(input, " ");
			parseCommand(command, command_root);
		}
	}

	//Build Command Tree, used for command verification and flag-based functionality
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
		return result;
	}

	//Recursively add Command Tree Node
	CommandTreeNode buildCommandTreeNode(std::vector<std::vector<std::string>> lines, int line_ind, int word_ind){
		CommandTreeNode result;
		int offset = 1;
		result.command = lines[line_ind][word_ind];
		
		//If at the end of the command
		if(word_ind == lines[line_ind].size() - offset){
			return result;
		}
		else{
			//If a flag is found
			if(isCommandTreeFlag(lines[line_ind][word_ind + offset])){
				result.flag = parseInteger(lines[line_ind][word_ind + offset++].substr(1));
				if(word_ind == lines[line_ind].size() - offset){
					return result;
				}
			}
			//If a label is found
			if(isCommandTreeLabel(lines[line_ind][word_ind + offset])){
				std::vector<CommandTreeNode> nodes;
				nodes = buildCommandTreeNodes(lines, lines[line_ind][word_ind + offset]);
				for(CommandTreeNode ctn : nodes){
					result.nextCommands.push_back(ctn);
				}
			}
			//Otherwise
			else{
				result.nextCommands.push_back(buildCommandTreeNode(lines, line_ind, word_ind + offset));
			}
		}
		return result;
	}

	//Recursively add several Command Tree Nodes, used for parsing labels
	std::vector<CommandTreeNode> buildCommandTreeNodes(std::vector<std::vector<std::string>> lines, std::string label){
		std::vector<CommandTreeNode> result;

		//For each line starting with the found label
		for(int i = 0; i < lines.size(); i++){
			if(lines[i][0] == label){
				
				//If the label leads to a regular command
				if(!isCommandTreeLabel(lines[i][1])){
					CommandTreeNode node;
					node = buildCommandTreeNode(lines, i, 1);
					result.push_back(node);
				}

				//If the label leads to another label
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

	//Run through the Command Tree with a command
	void parseCommand(std::vector<std::string> command, CommandTreeNode ctn){
		std::vector<int> flags;
		bool invalid = false;
		for(std::string word : command){

			//If no node is found, command is invalid
			if(!getNextNode(word, &ctn)){
				displayInvalidMessage("Command not recognized: " + word);
				invalid = true;
				break;
			}
			if(ctn.flag >= 0 && !containsFlag(flags, ctn.flag)){
				flags.push_back(ctn.flag);
			}
		}

		//Special case for STR_R, a recursive node that allows an arbitrary number of strings
		if(ctn.nextCommands.size() != 0 && !invalid){
			bool STR_R_found = false;
			for(CommandTreeNode c : ctn.nextCommands){
				STR_R_found = (STR_R_found || c.command == "STR_R");
			}
			if(!STR_R_found){
				displayInvalidMessage("Too few arguments");
				printf("Expected: ");
				for(CommandTreeNode c : ctn.nextCommands){
					printf("%s ", c.command.c_str());
				}
				printf("\n");
				invalid = true;
			}
		}

		//If command is valid, process
		if(!invalid){
			highPriorityLock(mut);
			functions->processFlags(command, flags);
			highPriorityUnlock(mut);
		}
	}

	//Get the next Command Tree Node based on the command
	bool getNextNode(std::string word, CommandTreeNode *ctn){

		//If there are no remaining nodes, command is invalid
		if(ctn->nextCommands.size() == 0){ return false; }
		for(CommandTreeNode c : ctn->nextCommands){

			//Handle INT, FLT, STR and STR_R nodes
			if(c.command == "INT" || c.command == "FLT" || c.command == "STR" || c.command == "STR_R"){
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
				if(c.command == "STR_R"){
					return true;
				}
			}
			else if(c.command == word){
				*ctn = c;
				return true;
			}
		}

		//If command doesn't point to a new node, command is invalid
		return false;
	}

	//Returns whether a specific flag has been inserted
	bool containsFlag(std::vector<int> flags, int query){
		for(int f : flags){
			if(f == query){
				return true;
			}
		}
		return false;
	}

	//Returns whether a string is in the format of a label ( [LABEL] )
	bool isCommandTreeLabel(std::string word){
		return word[0] == '[' && word[word.length()-1] == ']';
	}

	//Returns whether a string is in the format of a flag ( /# )
	bool isCommandTreeFlag(std::string word){
		return word[0] == '/' && canParseInteger(word.substr(1));
	}

	//Recursively prints the Command Tree
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
};

#endif
