#pragma once

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"
#include "Utilities/ResourceManager.h"
#include "Utilities/ShaderManager.h"
#include "Utilities/StateManager.h"
#include "Utilities/WindowManager.h"

namespace Zongine {
    struct ManagerList {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<WindowManager> windowManager{};
        std::shared_ptr<DeviceManager> deviceManager{};
        std::shared_ptr<ResourceManager> resourceManager{};
        std::shared_ptr<ShaderManager> shaderManager{};
        std::shared_ptr<StateManager> stateManager{};
        std::shared_ptr<EffectManager> effectManager{};
    };
}