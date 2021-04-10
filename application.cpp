#include "application.hpp"

#include "log_stream.hpp"
#include "main_window.hpp"

#include <windowsx.h>

#include <stdexcept>


Application::Application() {
    m_main_window = std::make_unique<MainWindow>();
}

Application::~Application() noexcept {
    m_main_window->Close();
}

void Application::Run() {
    m_main_window->Show();
}
