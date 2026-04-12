#include "rbtree.h"
#include <iostream>
#include <string>

struct Command {
    enum Function {
        insert,
        remove,
        find,
        save,
        load
    };

    Function func;
    std::string key;
    unsigned long long value;

    friend std::istream& operator>>(std::istream& is, Command& command) {
        std::string token;
        is >> token;

        if (token == "+") {
            command.func = insert;
            is >> command.key;
            is >> command.value;
        }
        else if (token == "-") {
            command.func = remove;
            is >> command.key;
        }
        else if (token == "!") {
            is >> token;
            if (token == "Save") {
                command.func = save;
            }
            else if (token == "Load") {
                command.func = load;
            }
            is >> command.key;
        }
        else {
            command.func = find;
            command.key = token;
        }
        
        return is;
    }
};


int main(void) {
    RBTree tree;

    Command command;
    ReturnData data;
    while (true) {
        try {
            if (!(std::cin >> command)) break;
            switch (command.func) {
                case Command::insert:
                    data.status = tree.insert(command.key, command.value);
                    break;
                case Command::remove:
                    data.status = tree.remove(command.key);
                    break;
                case Command::find:
                    data = tree.find(command.key);
                    break;
                case Command::save:
                    data.status = tree.save(command.key);
                    break;
                case Command::load:
                    data.status = tree.load(command.key);
                    break;
            }
            std::cout << data.status;
            if (command.func == Command::find && data.status == ReturnData::ok) {
                std::cout << ": " << data.value;
            }
            std::cout << '\n';
        }
        catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << '\n';
        }
    }
}