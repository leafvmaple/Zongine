#include "Entity.h"

#include "EntityManager.h"

#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"
#include "components/ShaderComponent.h"
#include "components/MaterialComponent.h"
#include "Components/CameraComponent.h"

namespace Zongine {
    template<typename ComponentType>
    const ComponentType& Entity::GetComponent() const {
        return m_Manager->GetComponent<ComponentType>(m_ID);
    }

    template<typename ComponentType>
    ComponentType& Entity::GetComponent() {
        return m_Manager->GetComponent<ComponentType>(m_ID);
    }

    template<typename ComponentType>
    ComponentType& Entity::AddComponent(const ComponentType& component) {
        return m_Manager->AddComponent<ComponentType>(m_ID, component);
    }

    Entity& Entity::AddChild() {
        auto& entity = m_Manager->CreateEntity();
        m_Children.push_back(entity.GetID());
        return entity;
    }

    template const TransformComponent& Entity::GetComponent<TransformComponent>() const;
    template const MeshComponent& Entity::GetComponent<MeshComponent>() const;
    template const ShaderComponent& Entity::GetComponent<ShaderComponent>() const;
    template const MaterialComponent& Entity::GetComponent<MaterialComponent>() const;
    template const CameraComponent& Entity::GetComponent<CameraComponent>() const;

    template TransformComponent& Entity::GetComponent<TransformComponent>();
    template MeshComponent& Entity::GetComponent<MeshComponent>();
    template ShaderComponent& Entity::GetComponent<ShaderComponent>();
    template MaterialComponent& Entity::GetComponent<MaterialComponent>();
    template CameraComponent& Entity::GetComponent<CameraComponent>();

    template TransformComponent& Entity::AddComponent(const TransformComponent&);
    template MeshComponent& Entity::AddComponent(const MeshComponent&);
    template ShaderComponent& Entity::AddComponent(const ShaderComponent&);
    template MaterialComponent& Entity::AddComponent(const MaterialComponent&);
    template CameraComponent& Entity::AddComponent(const CameraComponent&);
}