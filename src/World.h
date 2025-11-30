#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Entity.h"
#include "Component.h"
#include "SupportFuncs.h"
#include "Event.h"
#include "EventBus.h"

class World
{
private:
    int enemycounter;
    std::vector<std::unique_ptr<Entity>> entities;

public:
    float TimeElapsed;

    World();
    ~World();

    // Evitar copias del mundo
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // Crea enemigos con IA y los agrega al mundo
    Entity& createEntity(SDL_Renderer* renderer);
    
    // Verifica si un área rectangular está libre de obstáculos
    bool IsAreaFree(float x, float y, float w, float h);

    // Gestión de eventos
    void Emit(const Event& event);
    bool Poll(Event& out, const std::string& type);

    // Gestión de entidades y componentes
    void AddEntity(std::unique_ptr<Entity> entity);
    void AddComponentToEntity(const std::string& Id, std::unique_ptr<Component> component);
    Entity* GetEntityByName(const std::string& name);
    Entity* GetEntityById(const std::string& Id);

    std::vector<std::unique_ptr<Entity>>& GetEntities();
};