#include "main_window.hpp"

#include "event_dispatcher.hpp"
#include "log_stream.hpp"
#include "main_scene.hpp"

#include <windowsx.h>

#include <stdexcept>


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


MainWindow::MainWindow() = default;

MainWindow::~MainWindow() noexcept {
    Shutdown();
}

void MainWindow::Show() {
    if (FAILED(CreateMainWindow())) {
        log << "Failed to create application window";
        throw std::runtime_error("Failed to create window");
    }

    ShowWindow(m_window_handle, SW_SHOWNORMAL);

    RunEventLoop();
}

void MainWindow::Close() noexcept {
    m_shutting_down.store(true, std::memory_order_release);
}

void MainWindow::RegisterWindowClass() noexcept {
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindow::WindowProc;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    wcex.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassEx(&wcex);
}

HRESULT MainWindow::CreateMainWindow() noexcept {
    RegisterWindowClass();

    m_window_handle = CreateWindow(WINDOW_CLASS_NAME,
                                   L"Application Main Window",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   800,
                                   600,
                                   nullptr,
                                   nullptr,
                                   HINST_THISCOMPONENT,
                                   this);

    if (m_window_handle) {
        return S_OK;
    }

    return E_FAIL;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept {
    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        MainWindow* window = (MainWindow*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            window_handle,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(window)
        );

        window->m_event_dispatcher = std::make_shared<EventDispatcher>();
        window->m_scene = std::make_unique<MainScene>(window_handle, window->m_event_dispatcher);
        return 1;
    }
    
    LRESULT result = 0;

    auto window = reinterpret_cast<MainWindow*>(::GetWindowLongPtr(window_handle, GWLP_USERDATA));
    bool was_processed = true;

    switch (message) {
    case WM_PAINT:
    {
        window->m_event_dispatcher->PostEvent(PaintEvent());
        break;
    }
    case WM_CLOSE:
    {
        window->m_shutting_down.store(true, std::memory_order_release);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        int x = GET_X_LPARAM(l_param);
        int y = GET_Y_LPARAM(l_param);
        constexpr bool MOUSE_PRESSED = true;
        window->m_event_dispatcher->PostEvent(MouseEvent(MouseEvent::DetailedCode::MOUSEL_DOWN, x, y, MOUSE_PRESSED));
        break;
    }
    case WM_LBUTTONUP:
    {
        int x = GET_X_LPARAM(l_param);
        int y = GET_Y_LPARAM(l_param);
        window->m_event_dispatcher->PostEvent(MouseEvent(MouseEvent::DetailedCode::MOUSEL_UP, x, y));
        break;
    }
    case WM_SIZE:
    {
        UINT width = LOWORD(l_param);
        UINT height = HIWORD(l_param);
        window->m_event_dispatcher->PostEvent(ResizeEvent(width, height));
        break;
    }
    case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(l_param);
        int y = GET_Y_LPARAM(l_param);
        bool mouse_pressed = MK_LBUTTON & w_param;
        window->m_event_dispatcher->PostEvent(MouseEvent(MouseEvent::DetailedCode::MOUSE_MOVE, x, y, mouse_pressed));
        break;
    }
    default:
    {
        was_processed = false;
        break;
    }
    }
    result = DefWindowProc(window_handle, message, w_param, l_param);

    return result;
}

void MainWindow::Shutdown() noexcept {
    m_scene.reset();
}

void MainWindow::RunEventLoop() {
    MSG message;

    log << "Run event loop";
    while (GetMessage(&message, m_window_handle, 0, 0) && !m_shutting_down.load(std::memory_order_acquire)) {
        DispatchMessage(&message);
    }
}
