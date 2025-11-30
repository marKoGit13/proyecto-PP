#include "Entity.h"

int Entity::cantidad = 0;

Entity::Entity()
{
    Id = "Entity_" + std::to_string(cantidad);
    cantidad++;
}

Entity::~Entity() {
}

Entity::Entity(std::string name)
    : Name(name)
{
    Id = "Entity_" + std::to_string(cantidad);
    cantidad++;
}

void Entity::AddComponent(std::unique_ptr<Component> component)
{
    components.push_back(std::move(component));
}

Component* Entity::GetComponent(std::string type)
{
    for (const std::unique_ptr<Component>& c : components)
    {
        // Comparamos el Component::Type (string) con el string solicitado
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