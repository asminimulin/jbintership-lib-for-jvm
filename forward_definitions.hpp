#pragma once

#define UNICODE


#define MACRO_STR_IMPL(x) #x
#define MACRO_STR(x) MACRO_STR_IMPL(x)


template<class Interface>
inline void SafeReleaseDirect2DObject(Interface*& interface_ptr) {
    if (interface_ptr) {
        interface_ptr->Release();
        interface_ptr = nullptr;
    }
}
