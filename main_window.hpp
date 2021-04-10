#pragma once

#include "forward_definitions.hpp"
#include "event_dispatcher.hpp"

#include <windows.h>

#include <atomic>
#include <memory>


class MainScene;


class MainWindow {
public:
    MainWindow();
    ~MainWindow() noexcept;

    void Show();

    void Close() noexcept;

private:
    static constexpr const wchar_t* WINDOW_CLASS_NAME = L"Application Main Window";
    void RegisterWindowClass() noexcept;
    HRESULT CreateMainWindow() noexcept;

    
    static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept;
    
    void Shutdown() noexcept;

    void RunEventLoop();

    HWND m_window_handle{ nullptr };
    std::atomic_bool m_shutting_down{ false };

    std::unique_ptr<MainScene> m_scene;
    std::shared_ptr<EventDispatcher> m_event_dispatcher;
};

