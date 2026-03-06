#pragma once

#include "IEditorBridge.h"
#include "../Entities/World.h"

#include <string>
#include <vector>
#include <functional>
#include <typeindex>
#include <DirectXMath.h>

namespace Zongine {

    // =========================================================================
    // PropertyDescriptor -- Knows how to read/write a single property
    //
    // Getter returns the current value as a type-erased PropertyValue.
    // Setter writes a PropertyValue back to the component.
    // Both take a void* to the component instance.
    // =========================================================================
    struct PropertyDescriptor {
        std::string Name;
        bool ReadOnly = false;

        std::function<PropertyValue(const void*)> Getter;
        std::function<bool(void*, const PropertyValue&)> Setter;
    };

    // =========================================================================
    // ComponentDescriptor -- All properties for one component type
    //
    // HasComponent / GetComponent / GetComponentConst use type-erased
    // lambdas bound to concrete ComponentStorage<T> operations.
    // =========================================================================
    struct ComponentDescriptor {
        std::string Name;
        std::type_index TypeIndex = std::type_index(typeid(void));
        std::vector<PropertyDescriptor> Properties;

        std::function<bool(const World&, EntityID)> HasComponent;
        std::function<void*(World&, EntityID)> GetComponent;
        std::function<const void*(const World&, EntityID)> GetComponentConst;
    };

    // =========================================================================
    // ComponentReflectionRegistry -- Central registry for all reflected types
    //
    // Components register themselves once (typically at engine startup).
    // EditorBridgeImpl queries this registry to build ComponentInfo lists
    // and perform SetProperty -- no per-component switches needed.
    // =========================================================================
    class ComponentReflectionRegistry {
    public:
        static ComponentReflectionRegistry& Instance();

        void Register(ComponentDescriptor desc);
        const std::vector<ComponentDescriptor>& GetAll() const { return m_Descriptors; }

    private:
        ComponentReflectionRegistry() = default;
        std::vector<ComponentDescriptor> m_Descriptors;
    };

    // =========================================================================
    // Template helpers -- Reduces boilerplate for common property types
    //
    // Usage:
    //   RegisterComponent<TransformComponent>("Transform", {
    //       MakeVec3Property<TransformComponent>("Position", &TransformComponent::Position),
    //       MakeVec3Property<TransformComponent>("Rotation", &TransformComponent::Rotation),
    //       MakeVec3Property<TransformComponent>("Scale",    &TransformComponent::Scale),
    //   });
    // =========================================================================

    template<typename T>
    void RegisterComponent(const std::string& name, std::vector<PropertyDescriptor> props);

    // --- XMFLOAT3 property (read/write) ---
    template<typename T>
    PropertyDescriptor MakeVec3Property(const std::string& name,
                                        DirectX::XMFLOAT3 T::* member,
                                        bool readOnly = false) {
        PropertyDescriptor desc;
        desc.Name = name;
        desc.ReadOnly = readOnly;
        desc.Getter = [member](const void* p) -> PropertyValue {
            auto& c = *static_cast<const T*>(p);
            auto& v = c.*member;
            return std::array<float, 3>{ v.x, v.y, v.z };
        };
        if (!readOnly) {
            desc.Setter = [member](void* p, const PropertyValue& val) -> bool {
                if (auto* vec = std::get_if<std::array<float, 3>>(&val)) {
                    auto& c = *static_cast<T*>(p);
                    (c.*member) = { (*vec)[0], (*vec)[1], (*vec)[2] };
                    return true;
                }
                return false;
            };
        }
        return desc;
    }

    // --- float property ---
    template<typename T>
    PropertyDescriptor MakeFloatProperty(const std::string& name,
                                         float T::* member,
                                         bool readOnly = false) {
        PropertyDescriptor desc;
        desc.Name = name;
        desc.ReadOnly = readOnly;
        desc.Getter = [member](const void* p) -> PropertyValue {
            return static_cast<const T*>(p)->*member;
        };
        if (!readOnly) {
            desc.Setter = [member](void* p, const PropertyValue& val) -> bool {
                if (auto* fv = std::get_if<float>(&val)) {
                    static_cast<T*>(p)->*member = *fv;
                    return true;
                }
                return false;
            };
        }
        return desc;
    }

    // --- bool property ---
    template<typename T>
    PropertyDescriptor MakeBoolProperty(const std::string& name,
                                        bool T::* member,
                                        bool readOnly = false) {
        PropertyDescriptor desc;
        desc.Name = name;
        desc.ReadOnly = readOnly;
        desc.Getter = [member](const void* p) -> PropertyValue {
            return static_cast<const T*>(p)->*member;
        };
        if (!readOnly) {
            desc.Setter = [member](void* p, const PropertyValue& val) -> bool {
                if (auto* bv = std::get_if<bool>(&val)) {
                    static_cast<T*>(p)->*member = *bv;
                    return true;
                }
                return false;
            };
        }
        return desc;
    }

    // --- std::string property (typically read-only for asset paths) ---
    template<typename T>
    PropertyDescriptor MakeStringProperty(const std::string& name,
                                          std::string T::* member,
                                          bool readOnly = true) {
        PropertyDescriptor desc;
        desc.Name = name;
        desc.ReadOnly = readOnly;
        desc.Getter = [member](const void* p) -> PropertyValue {
            return static_cast<const T*>(p)->*member;
        };
        if (!readOnly) {
            desc.Setter = [member](void* p, const PropertyValue& val) -> bool {
                if (auto* sv = std::get_if<std::string>(&val)) {
                    static_cast<T*>(p)->*member = *sv;
                    return true;
                }
                return false;
            };
        }
        return desc;
    }

    // --- uint32_t property ---
    template<typename T>
    PropertyDescriptor MakeUintProperty(const std::string& name,
                                        uint32_t T::* member,
                                        bool readOnly = true) {
        PropertyDescriptor desc;
        desc.Name = name;
        desc.ReadOnly = readOnly;
        desc.Getter = [member](const void* p) -> PropertyValue {
            return static_cast<const T*>(p)->*member;
        };
        if (!readOnly) {
            desc.Setter = [member](void* p, const PropertyValue& val) -> bool {
                if (auto* uv = std::get_if<uint32_t>(&val)) {
                    static_cast<T*>(p)->*member = *uv;
                    return true;
                }
                return false;
            };
        }
        return desc;
    }

    // --- RegisterComponent implementation ---
    template<typename T>
    void RegisterComponent(const std::string& name, std::vector<PropertyDescriptor> props) {
        ComponentDescriptor desc;
        desc.Name = name;
        desc.TypeIndex = std::type_index(typeid(T));
        desc.Properties = std::move(props);

        desc.HasComponent = [](const World& w, EntityID id) -> bool {
            return w.Has<T>(id);
        };

        desc.GetComponent = [](World& w, EntityID id) -> void* {
            return &w.Get<T>(id);
        };

        desc.GetComponentConst = [](const World& w, EntityID id) -> const void* {
            return &w.Get<T>(id);
        };

        ComponentReflectionRegistry::Instance().Register(std::move(desc));
    }

    // =========================================================================
    // RegisterAllComponents -- Called once at engine startup
    // =========================================================================
    void RegisterAllComponents();
}
