#include <iostream>
#include "getUserInput.h"

using namespace std;

int getUserInput(const char* message) {
    int result;
    std::cout << message;
    std::cin >> result;

    return result;
}