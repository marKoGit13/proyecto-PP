#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Component.h"

class Entity
{
    private:
        static int cantidad;
        std::vector<std::unique_ptr<Component>> components; 
    public:
        std::string Id;
        std::string Name;

        Entity();
        ~Entity();
        Entity(std::string name);
        std::string GetName();
        std::string GetId();
        void AddComponent(std::unique_ptr<Component> component);
        Component* GetComponent(std::string type);
};