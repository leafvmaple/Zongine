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

    template const TransformComponent& Entity::GetComponent<TransformComponent>() const;
    template const MeshComponent& Entity::GetComponent<MeshComponent>() const;
    template const ShaderComponent& Entity::GetComponent<ShaderComponent>() const;
    template const MaterialComponent& Entity::GetComponent<MaterialComponent>() const;
    template const CameraComponent& Entity::GetComponent<CameraComponent>() const;
    template const SkeletonComponent& Entity::GetComponent<SkeletonComponent>() const;
    template const AnimationComponent& Entity::GetComponent<AnimationComponent>() const;

    template bool Entity::HasComponent<TransformComponent>() const;
    template bool Entity::HasComponent<MeshComponent>() const;
    template bool Entity::HasComponent<ShaderComponent>() const;
    template bool Entity::HasComponent<MaterialComponent>() const;
    template bool Entity::HasComponent<CameraComponent>() const;
    template bool Entity::HasComponent<SkeletonComponent>() const;
    template bool Entity::HasComponent<AnimationComponent>() const;

    template TransformComponent& Entity::AddComponent(const TransformComponent&);
    template MeshComponent& Entity::AddComponent(const MeshComponent&);
    template ShaderComponent& Entity::AddComponent(const ShaderComponent&);
    template MaterialComponent& Entity::AddComponent(const MaterialComponent&);
    template CameraComponent& Entity::AddComponent(const CameraComponent&);
    template SkeletonComponent& Entity::AddComponent(const SkeletonComponent&);
    template AnimationComponent& Entity::AddComponent(const AnimationComponent&);
}