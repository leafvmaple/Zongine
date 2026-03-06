#include "../Include/ComponentReflection.h"

#include <windows.h>

#include "../Entities/World.h"
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
    // Registry singleton
    // =========================================================================
    ComponentReflectionRegistry& ComponentReflectionRegistry::Instance() {
        static ComponentReflectionRegistry s_Instance;
        return s_Instance;
    }

    void ComponentReflectionRegistry::Register(ComponentDescriptor desc) {
        m_Descriptors.push_back(std::move(desc));
    }

    // =========================================================================
    // RegisterAllComponents -- one-time registration of all component types
    //
    // Adding a new component to the editor requires only adding a
    // RegisterComponent<> call here. No changes to EditorBridgeImpl needed.
    // =========================================================================
    void RegisterAllComponents() {
        // --- Transform ---
        RegisterComponent<TransformComponent>("Transform", {
            MakeVec3Property<TransformComponent>("Position", &TransformComponent::Position),
            MakeVec3Property<TransformComponent>("Rotation", &TransformComponent::Rotation),
            MakeVec3Property<TransformComponent>("Scale",    &TransformComponent::Scale),
        });

        // --- Mesh ---
        RegisterComponent<MeshComponent>("Mesh", {
            MakeStringProperty<MeshComponent>("Path", &MeshComponent::Path, true),
        });

        // --- Shader (dynamic array -- custom getter) ---
        {
            ComponentDescriptor desc;
            desc.Name = "Shader";
            desc.TypeIndex = std::type_index(typeid(ShaderComponent));
            desc.HasComponent = [](const World& w, EntityID id) { return w.Has<ShaderComponent>(id); };
            desc.GetComponent = [](World& w, EntityID id) -> void* { return &w.Get<ShaderComponent>(id); };
            desc.GetComponentConst = [](const World& w, EntityID id) -> const void* { return &w.Get<ShaderComponent>(id); };
            // Properties are dynamic (array of paths), so we add them via a custom getter
            // that builds PropertyDescriptors at query time. We handle this in the
            // EditorBridgeImpl override for Shader only.
            // For now, register with empty properties -- the bridge will special-case it.
            ComponentReflectionRegistry::Instance().Register(std::move(desc));
        }

        // --- Material ---
        RegisterComponent<MaterialComponent>("Material", {
            MakeStringProperty<MaterialComponent>("Path", &MaterialComponent::Path, true),
        });

        // --- Camera (has nested struct members -- use custom descriptors) ---
        {
            std::vector<PropertyDescriptor> props;

            // AspectRatio lives inside CameraComponent::Perspective
            props.push_back({
                "AspectRatio", false,
                [](const void* p) -> PropertyValue {
                    return static_cast<const CameraComponent*>(p)->Perspective.fAspectRatio;
                },
                [](void* p, const PropertyValue& v) -> bool {
                    if (auto* fv = std::get_if<float>(&v)) {
                        static_cast<CameraComponent*>(p)->Perspective.fAspectRatio = *fv;
                        return true;
                    }
                    return false;
                }
            });

            props.push_back({
                "Fov", false,
                [](const void* p) -> PropertyValue {
                    return static_cast<const CameraComponent*>(p)->Perspective.fFovAngleY;
                },
                [](void* p, const PropertyValue& v) -> bool {
                    if (auto* fv = std::get_if<float>(&v)) {
                        static_cast<CameraComponent*>(p)->Perspective.fFovAngleY = *fv;
                        return true;
                    }
                    return false;
                }
            });

            props.push_back(MakeFloatProperty<CameraComponent>("Distance",    &CameraComponent::Distance));
            props.push_back(MakeFloatProperty<CameraComponent>("MinDistance",  &CameraComponent::MinDistance));
            props.push_back(MakeFloatProperty<CameraComponent>("MaxDistance",  &CameraComponent::MaxDistance));

            RegisterComponent<CameraComponent>("Camera", std::move(props));
        }

        // --- Skeleton ---
        RegisterComponent<SkeletonComponent>("Skeleton", {
            MakeStringProperty<SkeletonComponent>("Path", &SkeletonComponent::Path, true),
        });

        // --- Animation ---
        RegisterComponent<AnimationComponent>("Animation", {
            MakeStringProperty<AnimationComponent>("Path", &AnimationComponent::Path, true),
            MakeFloatProperty<AnimationComponent>("Speed", &AnimationComponent::Speed),
            MakeBoolProperty<AnimationComponent>("Loop",   &AnimationComponent::Loop),
        });

        // --- Flexible (dynamic array -- custom descriptors) ---
        {
            ComponentDescriptor desc;
            desc.Name = "Flexible";
            desc.TypeIndex = std::type_index(typeid(FlexibleComponent));
            desc.HasComponent = [](const World& w, EntityID id) { return w.Has<FlexibleComponent>(id); };
            desc.GetComponent = [](World& w, EntityID id) -> void* { return &w.Get<FlexibleComponent>(id); };
            desc.GetComponentConst = [](const World& w, EntityID id) -> const void* { return &w.Get<FlexibleComponent>(id); };
            // Dynamic arrays handled in EditorBridgeImpl override
            ComponentReflectionRegistry::Instance().Register(std::move(desc));
        }
    }
}
