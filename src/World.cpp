#include "World.h"

World::World() {
    enemycounter = 0;
}

World::~World() {
}
//Genera nuevos enemigos automáticamente
Entity& World::createEntity(SDL_Renderer* renderer){
    auto Information = ReadFromConfigFile("./assets/data.json");
    
    std::string rutaImagen = std::get<3>(Information);
    float ancho = std::get<5>(Information);
    float alto = std::get<6>(Information);

    float PosicionX = NumberRandomizer(true, 100.0f, 1800.0f);
    float PosicionY = NumberRandomizer(true, 100.0f, 1000.0f);
    float Velocity = NumberRandomizer(true, 50.f, 100.0f);

    auto Enemy = std::make_unique<Entity>("Enemy_" + std::to_string(enemycounter));
    Entity& refEnemy = *Enemy;
    AddEntity(std::move(Enemy));
    enemycounter++;

    AddComponentToEntity(refEnemy.GetId(), std::make_unique<TransformComponent>(PosicionX, PosicionY, Velocity, Velocity));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<SpriteComponent>(rutaImagen, renderer));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<ColliderComponent>(ancho, alto, std::pair<float,float>{PosicionX + ancho/2, PosicionY - alto/2}));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<EnemyComponent>());

    return refEnemy;
}
//Emite eventos nuevos
void World::Emit(const Event& event) {
    EventBus::Instance().Enqueue(std::make_unique<Event>(event));
}
//Consulta el bus de eventos sin eliminar elementos y copiando la cola
bool World::Poll(Event& out, const std::string& type) {
    auto& queue = EventBus::Instance().GetQueue();
    const size_t size = queue.size();
    bool found = false;

    for (size_t i = 0; i < size; ++i) {
        std::unique_ptr<Event> e = std::move(queue.front());
        queue.pop();

        if (!found && e->Type == type) {
            out = *e;
            found = true;
        }

        queue.push(std::move(e)); 
    }

    return found;
}
void World::AddEntity(std::unique_ptr<Entity> entity){
    entities.push_back(std::move(entity));
}

void World::AddComponentToEntity(const std::string& Id, std::unique_ptr<Component> component) {
    Entity* entity = GetEntityById(Id);
    if (entity)
        entity->AddComponent(std::move(component));
}

Entity* World::GetEntityByName(const std::string& name) {
    for (const auto& entity : entities) {
        if (entity->GetName() == name) {
            return entity.get();
        }
    }
    return nullptr;
}

Entity* World::GetEntityById(const std::string& Id) {
    for (const auto& entity : entities) {
        if (entity->GetId() == Id) {
            return entity.get();
        }
    }
    return nullptr;
}

std::vector<std::unique_ptr<Entity>>& World::GetEntities() {
    return entities;
}