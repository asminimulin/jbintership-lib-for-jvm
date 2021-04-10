#pragma once

#include "forward_definitions.hpp"

#include <atomic>
#include <memory>


class MainWindow;

class Application {
public:
    Application();
    ~Application() noexcept;

    void Run();
private:
    std::unique_ptr<MainWindow> m_main_window;
};

