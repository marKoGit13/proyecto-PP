#include "World.h"
#include <cmath>
#include <chrono>
#include <iostream>

// Función que ejecuta el hilo de IA para cada enemigo
void EnemyAI_Loop(EnemyComponent* self, TransformComponent* myTrans, TransformComponent* playerTrans) {
    while (self->threadActive) {
        if (playerTrans && myTrans) {
            // Lectura de posiciones actuales
            float pX = std::get<0>(playerTrans->Position);
            float pY = std::get<1>(playerTrans->Position);
            
            float myX = std::get<0>(myTrans->Position);
            float myY = std::get<1>(myTrans->Position);
            
            float destX = pX;
            float destY = pY;

            // Calcular distancia al jugador
            float dx = pX - myX;
            float dy = pY - myY;
            float dist = std::sqrt(dx*dx + dy*dy);

            // Definimos cómo se calcula una posición de flanqueo
            auto calculateFlankPos = [&](float radiusOffset, float yOffset) {
                bool goRight = (reinterpret_cast<uintptr_t>(self) % 2 == 0);
                if (goRight) destX = pX + radiusOffset;
                else         destX = pX - radiusOffset;
                destY = pY + yOffset;
            };

            if (self->role == EnemyRole::CHASER) {
                // Si estás cerca, te ataca directo
                if (dist < 450.0f) {
                    destX = pX;
                    destY = pY;
                } else {
                    calculateFlankPos(250.0f, 50.0f);
                }
            } 
            else if (self->role == EnemyRole::FLANKER) {
                if (dist < 150.0f) {
                    destX = pX;
                    destY = pY;
                } 
                else {
                    // Si no, mantiene su distancia rodeando
                    calculateFlankPos(350.0f, 100.0f);
                }
            }

            // Actualización atómica de coordenadas objetivo
            self->targetX = destX;
            self->targetY = destY;
        }

        // Pausa para simular tiempo de reacción y liberar CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// Inicialización del mundo
World::World() {
    enemycounter = 0;
    TimeElapsed = 0.0f;
}

World::~World() {}

// Creación de enemigo con validación de posición y asignación de hilo
Entity& World::createEntity(SDL_Renderer* renderer){
    auto Information = ReadFromConfigFile("./assets/data.json");
    std::string rutaImagen = std::get<3>(Information);

    /*float ancho = std::get<5>(Information);
    float alto = std::get<6>(Information);*/

    float scale = 0.6f;
    float ancho = std::get<5>(Information) * scale;
    float alto = std::get<6>(Information) * scale;

    // Intenta encontrar una posición libre aleatoria
    // Spawn con margen
    float PosicionX = 0, PosicionY = 0;
    float margin = 100.0f;
    float maxX = 1920.0f - margin - ancho;
    float maxY = 600.0f;

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

    // Crea componentes temporales para pasarlos al hilo
    auto transPtr = new TransformComponent(PosicionX, PosicionY, Velocity, Velocity);
    AddComponentToEntity(refEnemy.GetId(), std::unique_ptr<TransformComponent>(transPtr));
    
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<SpriteComponent>(rutaImagen, renderer));
    AddComponentToEntity(refEnemy.GetId(), std::make_unique<ColliderComponent>(ancho, alto, std::pair<float,float>{PosicionX + ancho/2, PosicionY + alto/2}));
    
    // Asigna rol aleatorio y lanza el hilo de IA
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

// Algoritmo de detección de espacio vacío (AABB)
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

// Métodos estándar de gestión de ECS
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