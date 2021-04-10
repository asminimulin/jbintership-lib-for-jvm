#pragma once

#include "forward_definitions.hpp"

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <atomic>


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


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

    HRESULT CreateDeviceIndependentResources() noexcept;
    HRESULT CreateDeviceResources() noexcept;
    void ReleaseDeviceResources() noexcept;

    static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) noexcept;

    void OnRender() noexcept;

    void OnResize(UINT width, UINT height) noexcept;

    void Shutdown() noexcept;

    void CreateTriangle(int x, int y) noexcept;
    void DeleteTriangle() noexcept;

    void RunEventLoop();

    HWND m_window_handle{ nullptr };
    std::atomic_bool m_shutting_down{ false };

    ID2D1Factory* m_direct2d_factory{ nullptr };
    ID2D1HwndRenderTarget* m_renderer_target{ nullptr };

    ID2D1PathGeometry* m_triangle{ nullptr };
    ID2D1SolidColorBrush* m_triangle_brush{ nullptr };

    static const D2D1::ColorF BG_COLOR;
};

