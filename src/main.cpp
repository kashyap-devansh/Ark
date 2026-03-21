#include <iostream>
#include <fstream>

#include "parser.h"
#include "databaseManager.h"
#include "error.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: ./file.exe code.ark\n";
        return 1;
    }

    DatabaseManager manager;

    std::ifstream file(argv[1]);
    if(!file) {
        std::cerr << "failed to open " << argv[1] << "\n";
        return 1;
    }

    std::string command;
    char ch;
    int currentLine = 1;
    int commandStartLine = 1;

    while(file.get(ch)) {
        if(ch == '\n') currentLine++;

        if(ch == '-' && file.peek() == '-') {
            while(file.get(ch) && ch != '\n');
            
            currentLine++;
            continue;
        }

        command += ch;

        if(ch == ';') {
            
            try {
                Parser parser(command, commandStartLine);
                parser.parse(manager); 
            }
            catch(const ArkException& e) {
                std::cerr << e.what() << "\n";

                return 1;
            }

            command.clear();

            commandStartLine = currentLine;
        }
    } 

    return 0;
}