#include "EditorBridgeImpl.h"

#include <windows.h>

#include "../Entities/World.h"
#include "EventManager.h"

#include "../Components/TransformComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/ShaderComponent.h"
#include "../Components/MaterialComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/SkeletonComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/FlexibleComponent.h"

namespace Zongine {

    // =========================================================================
    // Scene hierarchy
    // =========================================================================

    EntityInfo EditorBridgeImpl::GetEntityInfo(EntityID id) const {
        auto& world = World::GetInstance();
        EntityInfo info;
        info.ID = id;
        info.Name = world.GetName(id);
        info.Children = world.GetChildren(id);
        return info;
    }

    std::vector<EntityInfo> EditorBridgeImpl::GetEntityTree() const {
        auto& world = World::GetInstance();
        EntityID root = world.GetRootEntity();
        auto& rootChildren = world.GetChildren(root);

        std::vector<EntityInfo> tree;
        for (auto childId : rootChildren) {
            BuildEntityTree(childId, tree);
        }
        return tree;
    }

    EntityID EditorBridgeImpl::GetRootEntity() const {
        return World::GetInstance().GetRootEntity();
    }

    void EditorBridgeImpl::BuildEntityTree(EntityID id, std::vector<EntityInfo>& out) const {
        out.push_back(GetEntityInfo(id));
        auto& world = World::GetInstance();
        for (auto childId : world.GetChildren(id)) {
            BuildEntityTree(childId, out);
        }
    }

    // =========================================================================
    // Component inspection -- convert typed components to generic PropertyInfo
    // =========================================================================

    std::vector<ComponentInfo> EditorBridgeImpl::GetComponents(EntityID id) const {
        auto& world = World::GetInstance();
        std::vector<ComponentInfo> components;

        // --- Transform ---
        if (world.Has<TransformComponent>(id)) {
            auto& t = world.Get<TransformComponent>(id);
            ComponentInfo ci;
            ci.Name = "Transform";
            ci.Properties.push_back({ "Position", std::array<float, 3>{ t.Position.x, t.Position.y, t.Position.z } });
            ci.Properties.push_back({ "Rotation", std::array<float, 3>{ t.Rotation.x, t.Rotation.y, t.Rotation.z } });
            ci.Properties.push_back({ "Scale",    std::array<float, 3>{ t.Scale.x, t.Scale.y, t.Scale.z } });
            components.push_back(std::move(ci));
        }

        // --- Mesh ---
        if (world.Has<MeshComponent>(id)) {
            auto& m = world.Get<MeshComponent>(id);
            ComponentInfo ci;
            ci.Name = "Mesh";
            ci.Properties.push_back({ "Path", m.Path, true });
            components.push_back(std::move(ci));
        }

        // --- Shader ---
        if (world.Has<ShaderComponent>(id)) {
            auto& s = world.Get<ShaderComponent>(id);
            ComponentInfo ci;
            ci.Name = "Shader";
            for (size_t i = 0; i < s.Paths.size(); i++) {
                ci.Properties.push_back({ "Path[" + std::to_string(i) + "]", s.Paths[i], true });
            }
            components.push_back(std::move(ci));
        }

        // --- Material ---
        if (world.Has<MaterialComponent>(id)) {
            auto& m = world.Get<MaterialComponent>(id);
            ComponentInfo ci;
            ci.Name = "Material";
            ci.Properties.push_back({ "Path", m.Path, true });
            components.push_back(std::move(ci));
        }

        // --- Camera ---
        if (world.Has<CameraComponent>(id)) {
            auto& c = world.Get<CameraComponent>(id);
            ComponentInfo ci;
            ci.Name = "Camera";
            ci.Properties.push_back({ "AspectRatio", c.Perspective.fAspectRatio });
            ci.Properties.push_back({ "Fov", c.Perspective.fFovAngleY });
            ci.Properties.push_back({ "Distance", c.Distance });
            ci.Properties.push_back({ "MinDistance", c.MinDistance });
            ci.Properties.push_back({ "MaxDistance", c.MaxDistance });
            components.push_back(std::move(ci));
        }

        // --- Skeleton ---
        if (world.Has<SkeletonComponent>(id)) {
            auto& s = world.Get<SkeletonComponent>(id);
            ComponentInfo ci;
            ci.Name = "Skeleton";
            ci.Properties.push_back({ "Path", s.Path, true });
            components.push_back(std::move(ci));
        }

        // --- Animation ---
        if (world.Has<AnimationComponent>(id)) {
            auto& a = world.Get<AnimationComponent>(id);
            ComponentInfo ci;
            ci.Name = "Animation";
            ci.Properties.push_back({ "Path", a.Path, true });
            ci.Properties.push_back({ "Speed", a.Speed });
            ci.Properties.push_back({ "Loop", a.Loop });
            components.push_back(std::move(ci));
        }

        // --- Flexible ---
        if (world.Has<FlexibleComponent>(id)) {
            auto& f = world.Get<FlexibleComponent>(id);
            ComponentInfo ci;
            ci.Name = "Flexible";
            for (size_t i = 0; i < f.Drivers.size(); i++) {
                ci.Properties.push_back({
                    "Driver[" + std::to_string(i) + "].index",
                    f.Drivers[i].index,
                    true
                });
            }
            components.push_back(std::move(ci));
        }

        return components;
    }

    // =========================================================================
    // Component editing -- write generic PropertyValue back to typed components
    // =========================================================================

    bool EditorBridgeImpl::SetProperty(EntityID id, const std::string& componentName,
                                       const std::string& propertyName, const PropertyValue& value) {
        auto& world = World::GetInstance();

        if (componentName == "Transform" && world.Has<TransformComponent>(id)) {
            auto& t = world.Get<TransformComponent>(id);

            if (auto* vec = std::get_if<std::array<float, 3>>(&value)) {
                if (propertyName == "Position") {
                    t.Position = { (*vec)[0], (*vec)[1], (*vec)[2] };
                    return true;
                }
                if (propertyName == "Rotation") {
                    t.Rotation = { (*vec)[0], (*vec)[1], (*vec)[2] };
                    return true;
                }
                if (propertyName == "Scale") {
                    t.Scale = { (*vec)[0], (*vec)[1], (*vec)[2] };
                    return true;
                }
            }
        }

        if (componentName == "Camera" && world.Has<CameraComponent>(id)) {
            auto& c = world.Get<CameraComponent>(id);

            if (auto* fVal = std::get_if<float>(&value)) {
                if (propertyName == "AspectRatio") { c.Perspective.fAspectRatio = *fVal; return true; }
                if (propertyName == "Fov")         { c.Perspective.fFovAngleY = *fVal;   return true; }
                if (propertyName == "Distance")    { c.Distance = *fVal;                 return true; }
                if (propertyName == "MinDistance")  { c.MinDistance = *fVal;              return true; }
                if (propertyName == "MaxDistance")  { c.MaxDistance = *fVal;              return true; }
            }
        }

        if (componentName == "Animation" && world.Has<AnimationComponent>(id)) {
            auto& a = world.Get<AnimationComponent>(id);

            if (propertyName == "Speed") {
                if (auto* fVal = std::get_if<float>(&value)) { a.Speed = *fVal; return true; }
            }
            if (propertyName == "Loop") {
                if (auto* bVal = std::get_if<bool>(&value)) { a.Loop = *bVal; return true; }
            }
        }

        // Read-only properties (Mesh.Path, Shader.Paths, etc.) are not writable
        return false;
    }

    // =========================================================================
    // Events -- delegate to EventManager
    // =========================================================================

    void EditorBridgeImpl::Subscribe(const std::string& event, std::function<void()> callback) {
        if (EventManager::IsRegistered()) {
            EventManager::GetInstance().Subscribe(event, std::move(callback));
        }
    }

    void EditorBridgeImpl::Emit(const std::string& event) {
        if (EventManager::IsRegistered()) {
            EventManager::GetInstance().Emit(event);
        }
    }
}
