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

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    Entity& createEntity(SDL_Renderer* renderer);
    
    // --- CORRECCIÓN: Función pública para verificar espacio ---
    bool IsAreaFree(float x, float y, float w, float h);

    void Emit(const Event& event);
    bool Poll(Event& out, const std::string& type);

    void AddEntity(std::unique_ptr<Entity> entity);
    void AddComponentToEntity(const std::string& Id, std::unique_ptr<Component> component);
    Entity* GetEntityByName(const std::string& name);
    Entity* GetEntityById(const std::string& Id);

    std::vector<std::unique_ptr<Entity>>& GetEntities();
};