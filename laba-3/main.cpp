#include "rbtree.h"
#include <iostream>
#include <string>

static const char* statusToString(ReturnData::Status status) {
    switch (status) {
        case ReturnData::ok:
            return "OK";
        case ReturnData::exist:
            return "Exist";
        case ReturnData::noSuchWord:
            return "NoSuchWord";
    }
    return "NoSuchWord";
}

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
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    RBTree tree;

    Command command;
    ReturnData data{.status = ReturnData::noSuchWord, .value = 0};
    std::string output;
    output.reserve(1 * 1024 * 1024);
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
            output += statusToString(data.status);
            if (command.func == Command::find && data.status == ReturnData::ok) {
                output += ": ";
                output += std::to_string(data.value);
            }
            output.push_back('\n');
            if (output.size() >= (1 * 1024 * 1024)) {
                std::cout << output;
                output.clear();
            }
        }
        catch (const std::exception& e) {
            output += "ERROR: ";
            output += e.what();
            output.push_back('\n');
            if (output.size() >= (1 * 1024 * 1024)) {
                std::cout << output;
                output.clear();
            }
        }
    }
    std::cout << output;
}