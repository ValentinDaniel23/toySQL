#ifndef TOYSQL_INPUT_H
#define TOYSQL_INPUT_H

#include <iostream>
#include <string>
#include <iomanip>

class InputBuffer {
    std :: string buffer;

public:
    explicit InputBuffer(size_t initial_size) {
        buffer.resize(initial_size);
    }

    explicit InputBuffer() = default;

    std :: string& get() {
        return buffer;
    }
};

inline void print_prompt() {
    std::cout << "toy >";
}

void read_input(InputBuffer &input_buffer);

#endif //TOYSQL_INPUT_H