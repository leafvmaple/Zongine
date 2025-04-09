#include "Entity.h"

#include "EntityManager.h"

#include "Components/NameComponent.h"
#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"
#include "components/ShaderComponent.h"
#include "components/MaterialComponent.h"
#include "Components/CameraComponent.h"
#include "Components/SkeletonComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/FlexibleComponent.h"
#include "Components/LandscapeComponent.h"
#include "Components/LandscapeRegionComponent.h"

namespace Zongine {
    template<typename ComponentType>
    const ComponentType& Entity::GetComponent() const {
        return m_Manager->GetComponent<ComponentType>(m_ID);
    }

    template<typename ComponentType>
    ComponentType& Entity::AddComponent(const ComponentType& component) {
        return m_Manager->AddComponent<ComponentType>(m_ID, component);
    }

    template<typename ComponentType>
    bool Entity::HasComponent() const {
        return m_Manager->HasComponent<ComponentType>(m_ID);
    }

    std::string Entity::GetName() const {
        return GetComponent<NameComponent>().Name;
    }

    Entity& Entity::AddChild() {
        auto& entity = m_Manager->CreateEntity();
        m_Children.push_back(entity.GetID());
        return entity;
    }

    Entity& Entity::AddChild(std::string name) {
        auto& entity = m_Manager->CreateEntity(name);
        m_Children.push_back(entity.GetID());
        return entity;
    }

#define DECLARE_COMPONENT(type) \
    template const type& Entity::GetComponent<type>() const; \
    template bool Entity::HasComponent<type>() const; \
    template type& Entity::AddComponent(const type&);

    DECLARE_COMPONENT(TransformComponent);
    DECLARE_COMPONENT(MeshComponent);
    DECLARE_COMPONENT(ShaderComponent);
    DECLARE_COMPONENT(MaterialComponent);
    DECLARE_COMPONENT(CameraComponent);
    DECLARE_COMPONENT(SkeletonComponent);
    DECLARE_COMPONENT(AnimationComponent);
    DECLARE_COMPONENT(FlexibleComponent);
    DECLARE_COMPONENT(LandscapeComponent);
    DECLARE_COMPONENT(LandscapeRegionComponent);
}