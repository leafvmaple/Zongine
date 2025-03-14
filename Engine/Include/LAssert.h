#pragma once

#define FMT_UNICODE 0

#include <assert.h>
// #include "spdlog/spdlog.h"

#define BOOL_ERROR_RETURN(Condition) \
    if (!(Condition)) {              \
        return 0;                    \
    }

#define BOOL_ERROR_LOG_RETURN(Condition, LOG, ...)   \
    if (!(Condition)) {                              \
        printf(LOG, __VA_ARGS__);                    \
        return 0;                                    \
    }

#define BOOL_ERROR_CONTINUE(Condition)   \
    if (!(Condition)) {                  \
        continue;                        \
    }

#define BOOL_ERROR_BREAK(Condition)   \
    if (!(Condition)) {               \
        break;                        \
    }

#define BOOL_ERROR_EXIT(Condition)   \
    if (!(Condition)) {               \
        goto Exit0;                        \
    }

//#define BOOL_ERROR_LOG_EXIT(Condition, FORMAT, ...)   \
//    if (!(Condition)) {                     \
//        spdlog::error(FORMAT, __VA_ARGS__); \
//        goto Exit0;                         \
//    }

#define BOOL_SUCCESS_BREAK(Condition) \
    if ((Condition)) {                \
        hResult = S_OK;               \
        break;                        \
    }

#define BOOL_SUCCESS_EXIT(Condition)  \
    if ((Condition)) {                \
        hResult = S_OK;               \
        goto Exit0;                   \
    }

#define CHECK_BOOL(Condition) \
    if (!(Condition)) {       \
        return false;         \
    }

#define CHECK_HRESULT(hr)     \
    if (!SUCCEEDED(((hr)))) { \
        return false;         \
    }

#define CHECK_HRESULT_RELEASE(hr, com)     \
    if (!SUCCEEDED((hr))) {   \
        (com)->Release();     \
        return false;         \
    }

#define CHECK_HRESULT_RET(hr, Ret) \
    if (!SUCCEEDED((hr))) {        \
        return Ret;                \
    }

#define HRESULT_ERROR_RETURN(hr) \
    if (!SUCCEEDED(hr)) {        \
        return 0;                \
    }

#define HRESULT_ERROR_LOG_RETURN(hr, LOG, ...)   \
    if (!SUCCEEDED(hr)) {                        \
        printf(LOG, __VA_ARGS__);                \
        return 0;                                \
    }

#define HRESULT_ERROR_CONTINUE(hr)   \
    if (!SUCCEEDED(hr)) {            \
        continue;                    \
    }

#define HRESULT_ERROR_BREAK(hr)   \
    if (!SUCCEEDED(hr)) {         \
        break;                    \
    }

#define HRESULT_ERROR_EXIT(hr)    \
    if (!SUCCEEDED(hr)) {         \
        goto Exit0;               \
    }

//#define HRESULT_ERROR_LOG_EXIT(hr, FORMAT, ...)    \
//    if (!SUCCEEDED(hr)) {                          \
//        spdlog::error(FORMAT, __VA_ARGS__);        \
//        goto Exit0;                                \
//    }

#define HRESULT_SUCCESS_BREAK(hr) \
    if (SUCCEEDED(hr)) {          \
        hResult = S_OK;           \
        break;                    \
    }

#define HRESULT_SUCCESS_EXIT(hr) \
    if (SUCCEEDED(hr)) {         \
        hResult = S_OK;          \
        goto Exit0;              \
    }

#define SAFE_FREE(p)    \
    if ((p)) {          \
        free((p));      \
        (p) = nullptr;  \
    }

#define SAFE_DELETE(p) \
    if ((p)) {         \
        delete (p);    \
        (p) = nullptr; \
    }

#define SAFE_DELETE_ARRAY(p) \
    if ((p)) {               \
        delete[] (p);        \
        (p) = nullptr;       \
    }

#define SAFE_RELEASE(p) \
    if ((p)) {          \
        (p)->Release(); \
        (p) = nullptr;  \
    }

#define SAFE_CLOSE_HANDLE(h)    \
    if ((h)) {                  \
        CloseHandle((h));       \
        (h) = nullptr;          \
    }
