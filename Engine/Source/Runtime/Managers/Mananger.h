#pragma once

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"
#include "Utilities/AssetManager.h"
#include "Utilities/StateManager.h"
#include "Utilities/WindowManager.h"

namespace Zongine {
    struct ManagerList {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<WindowManager> windowManager{};
        std::shared_ptr<DeviceManager> deviceManager{};
        std::shared_ptr<AssetManager> assetManager{};
        std::shared_ptr<StateManager> stateManager{};
        std::shared_ptr<EffectManager> effectManager{};
    };
}