#include "Entity.h"

#include "EntityManager.h"

#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"
#include "components/ShaderComponent.h"
#include "components/MaterialComponent.h"

namespace Zongine {
    template<typename ComponentType>
    const ComponentType& Entity::GetComponent() const {
        return m_Manager->GetComponent<ComponentType>(m_ID);
    }

    template<typename ComponentType>
    ComponentType& Entity::GetComponent() {
        return m_Manager->GetComponent<ComponentType>(m_ID);
    }

    template const TransformComponent& Entity::GetComponent<TransformComponent>() const;
    template const MeshComponent& Entity::GetComponent<MeshComponent>() const;
    template const ShaderComponent& Entity::GetComponent<ShaderComponent>() const;
    template const MaterialComponent& Entity::GetComponent<MaterialComponent>() const;

    template TransformComponent& Entity::GetComponent<TransformComponent>();
    template MeshComponent& Entity::GetComponent<MeshComponent>();
    template ShaderComponent& Entity::GetComponent<ShaderComponent>();
    template MaterialComponent& Entity::GetComponent<MaterialComponent>();
}