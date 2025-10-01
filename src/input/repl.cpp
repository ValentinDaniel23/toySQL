#include <string>
#include "Repl.h"

void read_input(InputBuffer &input_buffer) {
    int ch = std::cin.peek();

    if (ch == '\n') {
        std::getline(std::cin, input_buffer.get());
        return;
    }

    std::getline(std::cin, input_buffer.get(), ';');

    std :: string ignore;
    std::getline(std::cin, ignore);
}