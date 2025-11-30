#include "Entity.h"

int Entity::cantidad = 0;

// Constructor por defecto con ID autogenerado
Entity::Entity()
{
    Id = "Entity_" + std::to_string(cantidad);
    cantidad++;
}

Entity::~Entity() {
}

// Constructor con nombre personalizado
Entity::Entity(std::string name)
    : Name(name)
{
    Id = "Entity_" + std::to_string(cantidad);
    cantidad++;
}

// Agrega un componente a la lista de la entidad
void Entity::AddComponent(std::unique_ptr<Component> component)
{
    components.push_back(std::move(component));
}

// Busca y devuelve un componente por su tipo (string)
Component* Entity::GetComponent(std::string type)
{
    for (const std::unique_ptr<Component>& c : components)
    {
        if (c->Type == type)
        {
            return c.get();
        }
    }
    return nullptr;
}

std::string Entity::GetName()
{
    return Name;
}

std::string Entity::GetId()
{
    return Id;
}