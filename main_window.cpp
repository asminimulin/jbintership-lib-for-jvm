#include "main_window.hpp"

#include "log_stream.hpp"

#include <windowsx.h>

#include <stdexcept>


const D2D1::ColorF MainWindow::BG_COLOR{ 1.0f * 37 / 255, 1.0f * 133 / 255, 1.0f * 75 / 255, 1.0f };

MainWindow::MainWindow() {
    if (FAILED(CreateDeviceIndependentResources())) {
        log << "Failed to Create device independent Direct2D resources";
        throw std::runtime_error("Failed to create D2D1Factory");
    }
}

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

HRESULT MainWindow::CreateDeviceIndependentResources() noexcept {
    HRESULT success = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_direct2d_factory);
    return success;
}

HRESULT MainWindow::CreateDeviceResources() noexcept {
    if (m_renderer_target) {
        return S_OK;
    }
    log << "Create device resources";

    HRESULT result = S_OK;

    RECT rect;
    GetClientRect(m_window_handle, &rect);

    auto size = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);

    result = m_direct2d_factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                        D2D1::HwndRenderTargetProperties(m_window_handle, size),
                                                        &m_renderer_target);

    if (SUCCEEDED(result)) {
        m_renderer_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &m_triangle_brush);
    }

    if (FAILED(result)) {
        log << "Failed to create triangle brush";
    }

    return result;
}

void MainWindow::ReleaseDeviceResources() noexcept {
    SafeReleaseDirect2DObject(m_renderer_target);
    SafeReleaseDirect2DObject(m_triangle_brush);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept {
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        MainWindow* window = (MainWindow*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            window_handle,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(window)
        );

        result = 1;
    } else {

        auto window = reinterpret_cast<MainWindow*>(::GetWindowLongPtr(window_handle, GWLP_USERDATA));
        bool was_processed = true;

        switch (message) {
        case WM_PAINT:
        {
            window->OnRender();
            break;
        }
        case WM_CLOSE:
        {
            log << "Received close event";
            window->m_shutting_down.store(true, std::memory_order_release);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(l_param);
            int y = GET_Y_LPARAM(l_param);
            window->CreateTriangle(x, y);
            window->OnRender();
            break;
        }
        case WM_LBUTTONUP:
        {
            window->DeleteTriangle();
            window->OnRender();
            break;
        }
        case WM_SIZE:
        {
            UINT width = LOWORD(l_param);
            UINT height = HIWORD(l_param);
            window->OnResize(width, height);
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (MK_LBUTTON & w_param) {
                window->DeleteTriangle();
                int new_x = GET_X_LPARAM(l_param);
                int new_y = GET_Y_LPARAM(l_param);
                window->CreateTriangle(new_x, new_y);
                window->OnRender();
            }
            break;
        }
        default:
        {
            was_processed = false;
            break;
        }
        }
        result = DefWindowProc(window_handle, message, w_param, l_param);
    }

    return result;
}

void MainWindow::OnRender() noexcept {
    if (FAILED(CreateDeviceResources())) {
        return;
    }

    PAINTSTRUCT paint_struct;
    BeginPaint(m_window_handle, &paint_struct);

    m_renderer_target->BeginDraw();

    m_renderer_target->Clear(BG_COLOR);
    if (m_triangle) {
        m_renderer_target->DrawGeometry(m_triangle, m_triangle_brush);
    }

    auto result = m_renderer_target->EndDraw();
    if (FAILED(result) || result == D2DERR_RECREATE_TARGET) {
        ReleaseDeviceResources();
        log << "OnRender failed. Last error: " << GetLastError();
    }

    EndPaint(m_window_handle, &paint_struct);
}

void MainWindow::OnResize(UINT width, UINT height) noexcept {
    if (m_renderer_target) {
        m_renderer_target->Resize(D2D1::Size(width, height));
    }
}

void MainWindow::Shutdown() noexcept {
    ReleaseDeviceResources();
    SafeReleaseDirect2DObject(m_direct2d_factory);
    SafeReleaseDirect2DObject(m_triangle);
}

void MainWindow::CreateTriangle(int x, int y) noexcept {
    log << "Create triangle on " << x << ' ' << y;
    if (m_triangle) {
        SafeReleaseDirect2DObject(m_triangle);
    }
    m_direct2d_factory->CreatePathGeometry(&m_triangle);
    ID2D1GeometrySink* geometry_sink = nullptr;
    if (FAILED(m_triangle->Open(&geometry_sink))) {
        log << "Failed to open PathGeometry. No GeometrySink instance.";
        return;
    }

    int width = 100;
    int height = 100;
    geometry_sink->BeginFigure(D2D1::Point2F(x, y), D2D1_FIGURE_BEGIN_FILLED);
    D2D1_POINT_2F points[] = {
        D2D1::Point2F(x - width / 2, y + height),
        D2D1::Point2F(x + width / 2, y + height)
    };
    geometry_sink->AddLines(points, ARRAYSIZE(points));
    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);

    if (FAILED(geometry_sink->Close())) {
        log << "Failed to close geometry sink";
    }

    SafeReleaseDirect2DObject(geometry_sink);
}

void MainWindow::DeleteTriangle() noexcept {
    SafeReleaseDirect2DObject(m_triangle);
}

void MainWindow::RunEventLoop() {
    MSG message;

    log << "Run event loop";
    while (GetMessage(&message, m_window_handle, 0, 0) && !m_shutting_down.load(std::memory_order_acquire)) {
        DispatchMessage(&message);
    }
}
