#pragma once

// Entity is just an ID (EntityID = uint32_t).
// All data and logic lives in World. See World.h.
//
// This header exists for backward compatibility.
// New code should include World.h and use EntityID directly.

#include "../Include/Types.h"
