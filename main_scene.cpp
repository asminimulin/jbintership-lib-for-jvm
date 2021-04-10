#include "main_scene.hpp"

#include "event_dispatcher.hpp"
#include "log_stream.hpp"

#include <stdexcept>


// It is #25854b color
const D2D1::ColorF MainScene::BG_COLOR{ 1.0f * 37 / 255, 1.0f * 133 / 255, 1.0f * 75 / 255, 1.0f };


MainScene::MainScene(HWND window_handle, std::weak_ptr<EventDispatcher> event_dispatcher)
    : m_window_handle(window_handle) {
    if (FAILED(CreateDeviceIndependentResources())) {
        log << "Failed to Create device independent Direct2D resources";
        throw std::runtime_error("Failed to create Direct2D resources");
    }

    if (FAILED(CreateDeviceResources())) {
        log << "Failed to Create device dependent Direct2d resources";
        throw std::runtime_error("Failed to create Direct2D resources");
    }

    if (auto dispatcher = event_dispatcher.lock()) {
        dispatcher->Subscribe(this);
    }
}

MainScene::~MainScene() {
    if (auto dispatcher = m_event_dispatcher.lock()) {
        dispatcher->Unsubscribe(this);
    }
}

void MainScene::OnEvent(const Event& event) noexcept {
    switch (event.GetCode()) {
    case Event::Code::Resize:
    {
        ReleaseDeviceResources();
        if (FAILED(CreateDeviceResources())) {
            log << "Failed to recreate device resources after resizing";
        }
        break;
    }
    case Event::Code::Paint:
    {
        Render();
        break;
    }
    case Event::Code::Mouse:
    {
        OnMouseEvent(reinterpret_cast<const MouseEvent&>(event));
        break;
    }
    default:
    {
        log << "Unknown event: " << int(event.GetCode());
        break;
    }
    }
}

inline HRESULT MainScene::CreateDeviceIndependentResources() noexcept {
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_direct2d_factory);;
}

inline HRESULT MainScene::CreateDeviceResources() noexcept {
    if (m_renderer_target && m_triangle_brush) {
        return S_OK;
    }

    if (m_renderer_target || m_triangle_brush) {
        log << "Unexpected inconsistency. Renderer target and triangle brush"
            " must be either both initialized or both uninitialized";
    }

    if (FAILED(CreateRendererTarget())) {
        log << "Failed to create renderer target";
        return S_FALSE;
    }

    if (FAILED(CreateTriangleBrush())) {
        log << "Failed to create triangle brush";
        return S_FALSE;
    }

    return S_OK;
}

inline HRESULT MainScene::CreateRendererTarget() noexcept {
    RECT rect;
    GetClientRect(m_window_handle, &rect);

    auto size = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);

    if (FAILED(m_direct2d_factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                          D2D1::HwndRenderTargetProperties(m_window_handle, size),
                                                          &m_renderer_target))) {
        log << "Failed to create HwndRendererTarget";
        return S_FALSE;
    }

    return S_OK;
}

inline HRESULT MainScene::CreateTriangleBrush() noexcept {
    if (FAILED(m_renderer_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &m_triangle_brush))) {
        log << "Failed to create solid color brush";
        return S_FALSE;
    }
    return S_OK;
}

void MainScene::ReleaseDeviceResources() noexcept {
    SafeReleaseDirect2DObject(m_renderer_target);
    SafeReleaseDirect2DObject(m_triangle_brush);
}

void MainScene::CreateTriangle(int x, int y) {
    if (m_triangle) {
        return;
    }
    log << "Create triangle at " << x << ' ' << y;

    m_direct2d_factory->CreatePathGeometry(&m_triangle);
    ID2D1GeometrySink* geometry_sink = nullptr;
    if (FAILED(m_triangle->Open(&geometry_sink))) {
        log << "Failed to open PathGeometry. No GeometrySink instance.";
        return;
    }

    geometry_sink->BeginFigure(D2D1::Point2F(x, y), D2D1_FIGURE_BEGIN_FILLED);
    D2D1_POINT_2F points[] = {
        D2D1::Point2F(x - TRIANGLE_WIDTH / 2, y + TRIANGLE_HEIGHT),
        D2D1::Point2F(x + TRIANGLE_WIDTH / 2, y + TRIANGLE_HEIGHT)
    };
    geometry_sink->AddLines(points, ARRAYSIZE(points));
    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);

    if (FAILED(geometry_sink->Close())) {
        log << "Failed to close geometry sink";
    }

    SafeReleaseDirect2DObject(geometry_sink);

    m_triangle_creating_position = { x, y };
    m_triangle_translated_vector = { 0, 0 };
}

void MainScene::MoveTriangle(int x, int y) {
    m_triangle_translated_vector.x = x - m_triangle_creating_position.x;
    m_triangle_translated_vector.y = y - m_triangle_creating_position.y;
}

void MainScene::DeleteTriangle() noexcept {
    SafeReleaseDirect2DObject(m_triangle);
    m_triangle_creating_position = { 0, 0 };
    m_triangle_translated_vector = { 0, 0 };
}

void MainScene::Render() noexcept {
    if (FAILED(CreateDeviceResources())) {
        return;
    }

    PAINTSTRUCT paint_struct;
    BeginPaint(m_window_handle, &paint_struct);

    m_renderer_target->BeginDraw();

    m_renderer_target->Clear(BG_COLOR);
    if (m_triangle) {
        auto dx = m_triangle_translated_vector.x;
        auto dy = m_triangle_translated_vector.y;
        m_renderer_target->SetTransform(D2D1::Matrix3x2F::Translation(dx, dy));
        m_renderer_target->DrawGeometry(m_triangle, m_triangle_brush);
    }

    auto result = m_renderer_target->EndDraw();
    if (FAILED(result) || result == D2DERR_RECREATE_TARGET) {
        ReleaseDeviceResources();
        if (result != D2DERR_RECREATE_TARGET) {
            log << "Failed to render scene";
        }
    }

    EndPaint(m_window_handle, &paint_struct);
}

void MainScene::OnMouseEvent(const MouseEvent& event) noexcept {
    switch (event.detailed_code) {
    case MouseEvent::DetailedCode::MOUSEL_DOWN:
    {
        CreateTriangle(event.x, event.y);
        Render();
        break;
    }
    case MouseEvent::DetailedCode::MOUSEL_UP:
    {
        DeleteTriangle();
        Render();
        break;
    }
    case MouseEvent::DetailedCode::MOUSE_MOVE:
    {
        if (event.mouse_pressed) {
            MoveTriangle(event.x, event.y);
        }
        Render();
        break;
    }
    default:
    {
        log << "Unexpected value of MouseEvent::detailed_code: " << int(event.detailed_code);
        break;
    }
    }
}
