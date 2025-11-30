#include "World.h"

World::World() {
    enemycounter = 0;
    TimeElapsed = 0.0f;
}

World::~World() {}

// --- CORRECCIÓN: createEntity ahora busca posición segura ---
Entity& World::createEntity(SDL_Renderer* renderer){
    // Leemos el JSON una vez (podrías optimizar esto haciéndolo miembro de clase, pero por ahora está bien)
    auto Information = ReadFromConfigFile("./assets/data.json");
    std::string rutaImagen = std::get<3>(Information);
    float ancho = std::get<5>(Information);
    float alto = std::get<6>(Information);

    float PosicionX = 0;
    float PosicionY = 0;
    
    // Intentar encontrar posición libre (máximo 10 intentos)
    for(int i = 0; i < 10; i++) {
        PosicionX = NumberRandomizer(true, 50.0f, 1800.0f);
        PosicionY = NumberRandomizer(true, 50.0f, 500.0f); // Spawnear en la mitad superior
        if(IsAreaFree(PosicionX, PosicionY, ancho, alto)) {
            break;
        }
    }

    float Velocity = NumberRandomizer(true, 50.f, 100.0f);

    auto Enemy = std::make_unique<Entity>("Enemy_" + std::to_string(enemycounter));
    Entity& refEnemy = *Enemy;
    AddEntity(std::move(Enemy));
    enemycounter++;

    AddComponentToEntity(refEnemy.GetId(), std::make_unique<TransformComponent>(PosicionX, PosicionY, Velocity, Velocity));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<SpriteComponent>(rutaImagen, renderer));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<ColliderComponent>(ancho, alto, std::pair<float,float>{PosicionX + ancho/2, PosicionY + alto/2})); // Collider centrado
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<EnemyComponent>());
    
    // Agregar vida al enemigo para evitar crasheos si el sistema de daño la busca
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<HealthComponent>(1));

    return refEnemy;
}

// --- IMPLEMENTACIÓN DE IsAreaFree ---
bool World::IsAreaFree(float x, float y, float w, float h) {
    float myLeft = x;
    float myRight = x + w;
    float myTop = y;
    float myBottom = y + h;

    for (auto& entity : entities) {
        auto collider = entity->GetComponent("Collider");
        if (!collider) continue;
        
        ColliderComponent* col = static_cast<ColliderComponent*>(collider);
        
        if (myLeft < col->getRight() && myRight > col->getLeft() &&
            myTop < col->getBottom() && myBottom > col->getTop()) {
            return false; // Ocupado
        }
    }
    return true; // Libre
}

void World::Emit(const Event& event) {
    EventBus::Instance().Enqueue(std::make_unique<Event>(event));
}

bool World::Poll(Event& out, const std::string& type) {
    auto& queue = EventBus::Instance().GetQueue();
    if (queue.empty()) return false;

    // Lógica simple: ver si el primero coincide (para no complicar el loop)
    if (queue.front()->Type == type) {
        out = *queue.front();
        queue.pop();
        return true;
    }
    return false;
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
        if (entity->GetName() == name) return entity.get();
    }
    return nullptr;
}

Entity* World::GetEntityById(const std::string& Id) {
    for (const auto& entity : entities) {
        if (entity->GetId() == Id) return entity.get();
    }
    return nullptr;
}

std::vector<std::unique_ptr<Entity>>& World::GetEntities() {
    return entities;
}