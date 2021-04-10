#include "application.hpp"

#include "log_stream.hpp"

#include <windowsx.h>

#include <stdexcept>

const D2D1::ColorF Application::BG_COLOR{ 1.0f * 37 / 255, 1.0f * 133 / 255, 1.0f * 75 / 255, 1.0f };

namespace {

template<class Interface>
inline void SafeReleaseDirect2DObject(Interface*& interface_ptr) {
    if (interface_ptr) {
        interface_ptr->Release();
        interface_ptr = nullptr;
    }
}

}

Application::Application() {
    if (FAILED(CreateDeviceIndependentResources())) {
        throw std::runtime_error("Failed to create D2D1Factory");
    }
}

Application::~Application() noexcept {
    Shutdown();
}

void Application::Run() {
    if (FAILED(CreateApplicationWindow())) {
        throw std::runtime_error("Failed to create window");
    }

    ShowWindow(m_window_handle, SW_SHOWNORMAL);

    MSG message;

    while (GetMessage(&message, m_window_handle, 0, 0) && !m_shutting_down.load(std::memory_order_acquire)) {
        DispatchMessage(&message);
    }
}

void Application::Stop() noexcept {
    m_shutting_down.store(true, std::memory_order_release);
}

void Application::RegisterWindowClass() noexcept {
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Application::WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    wcex.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassEx(&wcex);
}

HRESULT Application::CreateApplicationWindow() noexcept {
    RegisterWindowClass();

    m_window_handle = CreateWindow(WINDOW_CLASS_NAME,
                                   L"Application",
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

HRESULT Application::CreateDeviceIndependentResources() noexcept {
    HRESULT success = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_direct2d_factory);
    return success;
}

HRESULT Application::CreateDeviceResources() noexcept {
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

void Application::ReleaseDeviceResources() noexcept {
    SafeReleaseDirect2DObject(m_renderer_target);
    SafeReleaseDirect2DObject(m_triangle_brush);
}

LRESULT CALLBACK Application::WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept {
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        Application* app = (Application*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            window_handle,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(app)
        );

        result = 1;
    } else {
        
        auto application = reinterpret_cast<Application*>(::GetWindowLongPtr(window_handle, GWLP_USERDATA));
        bool was_processed = true;
        
        switch (message) {
        case WM_PAINT:
        {
            application->OnRender();
            break;
        }
        case WM_CLOSE:
        {
            application->m_shutting_down.store(true, std::memory_order_release);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(l_param);
            int y = GET_Y_LPARAM(l_param);
            application->CreateTriangle(x, y);
            application->OnRender();
            break;
        }
        case WM_LBUTTONUP:
        {
            application->DeleteTriangle();
            application->OnRender();
            break;
        }
        case WM_SIZE:
        {
            UINT width = LOWORD(l_param);
            UINT height = HIWORD(l_param);
            application->OnResize(width, height);
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (MK_LBUTTON & w_param) {
                application->DeleteTriangle();
                int new_x = GET_X_LPARAM(l_param);
                int new_y = GET_Y_LPARAM(l_param);
                application->CreateTriangle(new_x, new_y);
                application->OnRender();
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

void Application::OnRender() noexcept {
    auto result = CreateDeviceResources();
    if (SUCCEEDED(result)) {
        PAINTSTRUCT paint_struct;
        BeginPaint(m_window_handle, &paint_struct);

        m_renderer_target->BeginDraw();

        m_renderer_target->Clear(BG_COLOR);

        if (m_triangle) {
            m_renderer_target->DrawGeometry(m_triangle, m_triangle_brush);
        }

        result = m_renderer_target->EndDraw();
        if (FAILED(result) || result == D2DERR_RECREATE_TARGET) {
            ReleaseDeviceResources();
            log << "OnRender failed. Last error: " << GetLastError();
        }

        EndPaint(m_window_handle, &paint_struct);
    }
}

void Application::OnResize(UINT width, UINT height) noexcept {
    if (m_renderer_target) {
        m_renderer_target->Resize(D2D1::Size(width, height));
    }
}

void Application::Shutdown() noexcept {
    ReleaseDeviceResources();
    SafeReleaseDirect2DObject(m_direct2d_factory);
    SafeReleaseDirect2DObject(m_triangle);
}

void Application::CreateTriangle(int x, int y) noexcept {
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

void Application::DeleteTriangle() noexcept {
    SafeReleaseDirect2DObject(m_triangle);
}