#pragma once

#include <windows.h>

#include <functional>
#include <map>
#include <set>
#include <utility>


class Event {
public:
    enum class Code {
        Resize = 0,
        Paint = 1,
        Mouse = 2,
        WindowCreated = 3,
        WindowClosed = 4,
    };

    Event();

    virtual Code GetCode() const noexcept = 0;

private:
    static const Code code;
};

class ResizeEvent : public Event {
public:
    ResizeEvent(unsigned int width, unsigned int height);

    const std::pair<unsigned int, unsigned int> new_size;

    Code GetCode() const noexcept override;

    static const Code code = Code::Resize;
};

class PaintEvent : public Event {
public:
    PaintEvent();

    Code GetCode() const noexcept override;

    static const Code code = Code::Paint;
};

class MouseEvent : public Event {
public:
    enum class DetailedCode: uint8_t {
        MOUSEL_DOWN = 0,
        MOUSEL_UP = 1,
        MOUSE_MOVE = 2,
    };

    MouseEvent(DetailedCode code, int x, int y, bool mouse_pressed = false);

    const int x;
    const int y;
    const DetailedCode detailed_code : 4;
    const bool mouse_pressed : 1;

    Code GetCode() const noexcept override;
    static const Code code = Code::Mouse;
};

class EventDispatcher {
public:
    class Listener {
        friend class EventDispatcher;
    protected:
        virtual void OnEvent(const Event&) noexcept = 0;
    };

    EventDispatcher();

    void PostEvent(const Event& event);

    void Subscribe(Listener* listener);

    void Unsubscribe(Listener* listener);

private:

    std::set<Listener*> m_listeners;
};
