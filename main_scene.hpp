#pragma once

#include "event_dispatcher.hpp"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <memory>


class MainScene : public EventDispatcher::Listener {
public:
    MainScene(HWND window_handle, std::weak_ptr<EventDispatcher> event_dispatcher);
    ~MainScene();

protected:
    void OnEvent(const Event& event) noexcept override;

private:

    HRESULT CreateDeviceIndependentResources() noexcept;
    HRESULT CreateDeviceResources() noexcept;
    HRESULT CreateRendererTarget() noexcept;
    HRESULT CreateTriangleBrush() noexcept;
    void ReleaseDeviceResources() noexcept;

    void CreateTriangle(int x, int y);
    void MoveTriangle(int x, int y);
    void DeleteTriangle() noexcept;

    void Render() noexcept;

    void OnMouseEvent(const MouseEvent& event) noexcept;

    ID2D1Factory* m_direct2d_factory{ nullptr };
    ID2D1HwndRenderTarget* m_renderer_target{ nullptr };

    ID2D1PathGeometry* m_triangle{ nullptr };
    ID2D1SolidColorBrush* m_triangle_brush{ nullptr };
    D2D1_POINT_2L m_triangle_creating_position{ 0, 0 };
    D2D1_POINT_2L m_triangle_translated_vector{ 0, 0 };

    static const D2D1::ColorF BG_COLOR;

    HWND const m_window_handle;

    static constexpr int TRIANGLE_WIDTH = 100;
    static constexpr int TRIANGLE_HEIGHT = 100;

    std::weak_ptr<EventDispatcher> m_event_dispatcher;
};
