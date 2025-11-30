#include "World.h"
#include <cmath>
#include <chrono>
#include <iostream>

// =====================================================
// LÓGICA DEL HILO DE INTELIGENCIA ARTIFICIAL (CEREBRO)
// =====================================================
void EnemyAI_Loop(EnemyComponent* self, TransformComponent* myTrans, TransformComponent* playerTrans) {
    while (self->threadActive) {
        if (playerTrans && myTrans) {
            // 1. Obtener posiciones
            float pX = std::get<0>(playerTrans->Position);
            float pY = std::get<1>(playerTrans->Position);
            
            float myX = std::get<0>(myTrans->Position);
            float myY = std::get<1>(myTrans->Position);
            
            float destX = pX;
            float destY = pY;

            // 2. TOMA DE DECISIONES
            if (self->role == EnemyRole::CHASER) {
                // Perseguidor: Va directo siempre
                destX = pX;
                destY = pY;
            } 
            else if (self->role == EnemyRole::FLANKER) {
                // --- MEJORA: INTELIGENCIA DE CERCANÍA ---
                float dx = pX - myX;
                float dy = pY - myY;
                float dist = std::sqrt(dx*dx + dy*dy);

                // Si el jugador está MUY CERCA (< 300 px), ¡ATACAR!
                // Ya no "huyen" para rodear, sino que van a por ti.
                if (dist < 300.0f) {
                    destX = pX;
                    destY = pY;
                } 
                else {
                    // Si están lejos, siguen con su estrategia de rodear
                    bool goRight = (reinterpret_cast<uintptr_t>(self) % 2 == 0);
                    float offset = 350.0f; // Abren más el círculo
                    
                    if (goRight) destX += offset;
                    else         destX -= offset;
                    
                    // Intentan predecir un poco tu altura o quedarse un poco atrás
                    destY += 100.0f; 
                }
            }

            // 3. COMUNICACIÓN
            self->targetX = destX;
            self->targetY = destY;
        }

        // Pequeña pausa para no saturar CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// =====================================================
// IMPLEMENTACIÓN DE WORLD
// =====================================================

World::World() {
    enemycounter = 0;
    TimeElapsed = 0.0f;
}

World::~World() {}

Entity& World::createEntity(SDL_Renderer* renderer){
    auto Information = ReadFromConfigFile("./assets/data.json");
    std::string rutaImagen = std::get<3>(Information);
    float ancho = std::get<5>(Information);
    float alto = std::get<6>(Information);

    // Spawn seguro
    float PosicionX = 0, PosicionY = 0;
    for(int i=0; i<10; i++) {
        PosicionX = NumberRandomizer(true, 50.0f, 1800.0f);
        PosicionY = NumberRandomizer(true, 50.0f, 400.0f); 
        if(IsAreaFree(PosicionX, PosicionY, ancho, alto)) break;
    }

    float Velocity = NumberRandomizer(true, 50.f, 100.0f);

    auto Enemy = std::make_unique<Entity>("Enemy_" + std::to_string(enemycounter));
    Entity& refEnemy = *Enemy;
    AddEntity(std::move(Enemy));
    enemycounter++;

    // Componentes
    auto transPtr = new TransformComponent(PosicionX, PosicionY, Velocity, Velocity);
    AddComponentToEntity(refEnemy.GetId(), std::unique_ptr<TransformComponent>(transPtr));
    
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<SpriteComponent>(rutaImagen, renderer));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<ColliderComponent>(ancho, alto, std::pair<float,float>{PosicionX + ancho/2, PosicionY + alto/2}));
    
    // IA y Hilo
    EnemyRole rol = (NumberRandomizer(true, 0, 100) < 50) ? EnemyRole::CHASER : EnemyRole::FLANKER;
    auto enemyCompPtr = new EnemyComponent(rol);
    
    Entity* player = GetEntityByName("Player_0");
    TransformComponent* playerTrans = nullptr;
    if(player) {
        playerTrans = static_cast<TransformComponent*>(player->GetComponent("Transform"));
    }

    if(playerTrans) {
        enemyCompPtr->aiThread = std::thread(EnemyAI_Loop, enemyCompPtr, transPtr, playerTrans);
    }

    AddComponentToEntity(refEnemy.GetId(), std::unique_ptr<EnemyComponent>(enemyCompPtr));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<HealthComponent>(1));

    return refEnemy;
}

bool World::IsAreaFree(float x, float y, float w, float h) {
    float myLeft = x; float myRight = x + w;
    float myTop = y; float myBottom = y + h;

    for (auto& entity : entities) {
        auto collider = entity->GetComponent("Collider");
        if (!collider) continue;
        ColliderComponent* col = static_cast<ColliderComponent*>(collider);
        if (myLeft < col->getRight() && myRight > col->getLeft() &&
            myTop < col->getBottom() && myBottom > col->getTop()) {
            return false; 
        }
    }
    return true; 
}

void World::Emit(const Event& event) {
    EventBus::Instance().Enqueue(std::make_unique<Event>(event));
}

bool World::Poll(Event& out, const std::string& type) {
    auto& queue = EventBus::Instance().GetQueue();
    if (queue.empty()) return false;
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
    if (entity) entity->AddComponent(std::move(component));
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