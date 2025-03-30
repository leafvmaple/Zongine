#pragma once

#include "Entities/EntityManager.h"

#include "Managers/DeviceManager.h"
#include "Managers/EffectManager.h"
#include "Managers/AssetManager.h"
#include "Managers/StateManager.h"
#include "Managers/WindowManager.h"

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