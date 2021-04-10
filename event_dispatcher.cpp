#include "event_dispatcher.hpp"

Event::Event() = default;

ResizeEvent::ResizeEvent(unsigned int width, unsigned int height) : new_size(width, height) {}

Event::Code ResizeEvent::GetCode() const noexcept {
    return this->code;
}

PaintEvent::PaintEvent() = default;

Event::Code PaintEvent::GetCode() const noexcept {
    return this->code;
}

void EventDispatcher::PostEvent(const Event& event) {
    for (auto listener : m_listeners) {
        listener->OnEvent(event);
    }
}

void EventDispatcher::Subscribe(Listener* listener) {
    m_listeners.emplace(listener);
}

void EventDispatcher::Unsubscribe(Listener* listener) {
    auto it = m_listeners.find(listener);
    if (it != m_listeners.end()) {
        m_listeners.erase(it);
    }
}

EventDispatcher::EventDispatcher() = default;

MouseEvent::MouseEvent(DetailedCode code, int x, int y, bool mouse_pressed)
    : x(x)
    , y(y)
    , detailed_code(code)
    , mouse_pressed(mouse_pressed) {}

Event::Code MouseEvent::GetCode() const noexcept {
    return MouseEvent::code;
}
