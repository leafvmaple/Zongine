#pragma once

#include <assert.h>

#ifndef CHECK_BLOB
#define CHECK_BLOB(Condition, ErrorBlob) \
    if (FAILED(Condition)) {  \
        OutputDebugStringA(static_cast<char*>(ErrorBlob->GetBufferPointer())); \
        return false;         \
    }
#endif
