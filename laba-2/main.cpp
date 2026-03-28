#include <iostream>
#include <string>

struct Command {
    enum Function {
        add,
        del,
        find,
        save,
        load
    };

    Function func;
    std::string data;
    unsigned long long number;

    friend std::istream& operator>>(std::istream& is, Command& command) {
        std::string token;
        is >> token;

        if (token == "+") {
            command.func = add;
            is >> command.data;
            is >> command.number;
        }
        else if (token == "-") {
            command.func = del;
            is >> command.data;
        }
        else if (token == "!") {
            is >> token;
            if (token == "Save") {
                command.func = save;
            }
            else if (token == "Load") {
                command.func = load;
            }
            is >> command.data;
        }
        else {
            command.func = find;
            command.data = token;
        }
        
        return is;
    }
};


int main(void) {
    
}