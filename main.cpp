#include "application.hpp"

#include <iostream>
#include <memory>
#include <thread>


int main() {

    auto app = std::make_unique<Application>();
    app->Run();

    return 0;
}