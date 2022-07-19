#include <iostream>
#include <emscripten/emscripten.h>

using namespace std;

int EMSCRIPTEN_KEEPALIVE main() {
    std::cout << "Test!" << std::endl;

    return 0;
}
