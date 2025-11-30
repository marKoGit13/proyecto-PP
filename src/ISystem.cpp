#include "ISystem.h"
#include <cmath>
#include <string>
#include <algorithm> // Para std::max

// Definir PI si no está definido
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool ISystem::TooBad = false;

ISystem::ISystem() {}
ISystem::~ISystem() {}

// =========================================================
// FUNCIONES AUXILIARES
// =========================================================
bool CheckOverlap(ColliderComponent* a, ColliderComponent* b, float& overlapX, float& overlapY) {
    float dx = std::get<0>(a->MidPoint) - std::get<0>(b->MidPoint);
    float dy = std::get<1>(a->MidPoint) - std::get<1>(b->MidPoint);
    float combinedHalfW = (std::get<0>(a->Bounds) + std::get<0>(b->Bounds)) / 2.0f;
    float combinedHalfH = (std::get<1>(a->Bounds) + std::get<1>(b->Bounds)) / 2.0f;

    if (std::abs(dx) < combinedHalfW && std::abs(dy) < combinedHalfH) {
        overlapX = combinedHalfW - std::abs(dx);
        overlapY = combinedHalfH - std::abs(dy);
        return true;
    }
    return false;
}

// =========================================================
// PLAYER INPUT SYSTEM
// =========================================================
PlayerInputSystem::PlayerInputSystem(bool& isrunning) : isRunning(isrunning) {}

void PlayerInputSystem::Update(World& world, float dt) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) isRunning = false;
    }

    const bool* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE]) isRunning = false;

    // Input continuo para movimiento suave
    for (auto& entity : world.GetEntities()) {
        if (entity->GetComponent("Player")) {
            auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
            if (transform) {
                float speed = 300.0f; 
                std::get<0>(transform->Velocity) = 0;
                std::get<1>(transform->Velocity) = 0;
                
                if (state[SDL_SCANCODE_W]) std::get<1>(transform->Velocity) = -speed;
                if (state[SDL_SCANCODE_S]) std::get<1>(transform->Velocity) = speed;
                if (state[SDL_SCANCODE_A]) std::get<0>(transform->Velocity) = -speed;
                if (state[SDL_SCANCODE_D]) std::get<0>(transform->Velocity) = speed;
            }
        }
    }
}

// =========================================================
// MOVEMENT SYSTEM (IA y Física)
// =========================================================
MovementSystem::MovementSystem() {}

void MovementSystem::Update(World& world, float dt) 
{
    if (TooBad) return; // Congelar si Game Over

    Entity* player = nullptr;
    TransformComponent* playerTrans = nullptr;
    
    // Buscar jugador
    for (auto& ent : world.GetEntities()) {
        if (ent->GetComponent("Player")) {
            player = ent.get();
            playerTrans = static_cast<TransformComponent*>(ent->GetComponent("Transform"));
            break;
        }
    }

    for (auto& entity : world.GetEntities()) 
    {
        auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
        auto collider = static_cast<ColliderComponent*>(entity->GetComponent("Collider"));
        
        if (!transform) continue;

        // IA de Persecución (Enemigos apuntan al jugador)
        if (playerTrans && entity->GetComponent("Enemy")) {
            float dx = std::get<0>(playerTrans->Position) - std::get<0>(transform->Position);
            float dy = std::get<1>(playerTrans->Position) - std::get<1>(transform->Position);
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 5.0f) { // Evitar división por cero y jittering
                float enemySpeed = 120.0f;
                std::get<0>(transform->Velocity) = (dx / dist) * enemySpeed;
                std::get<1>(transform->Velocity) = (dy / dist) * enemySpeed;
            }
        }

        // Aplicar Movimiento
        std::get<0>(transform->Position) += std::get<0>(transform->Velocity) * dt;
        std::get<1>(transform->Position) += std::get<1>(transform->Velocity) * dt;

        // Sincronizar Collider
        if (collider) {
            collider->MidPoint = {
                std::get<0>(transform->Position) + std::get<0>(collider->Bounds)/2.0f,
                std::get<1>(transform->Position) + std::get<1>(collider->Bounds)/2.0f
            };
        }
    }
}

// =========================================================
// COLLISION SYSTEM (Con rebote y empuje)
// =========================================================
CollisionSystem::CollisionSystem(int w, int h) : WindowWidth(w), WindowHeight(h) {}

void CollisionSystem::Update(World& world, float dt) 
{
    if (TooBad) return;

    auto& entities = world.GetEntities();

    for (auto& entA : entities) 
    {
        bool isPlayer = entA->GetComponent("Player") != nullptr;
        bool isEnemy = entA->GetComponent("Enemy") != nullptr;
        if (!isPlayer && !isEnemy) continue;

        auto transA = static_cast<TransformComponent*>(entA->GetComponent("Transform"));
        auto colA = static_cast<ColliderComponent*>(entA->GetComponent("Collider"));
        if (!transA || !colA) continue;

        // Rebote en bordes de pantalla
        float x = std::get<0>(transA->Position);
        float y = std::get<1>(transA->Position);
        float w = std::get<0>(colA->Bounds);
        float h = std::get<1>(colA->Bounds);

        if (x < 0) { std::get<0>(transA->Position) = 0; std::get<0>(transA->Velocity) *= -1; }
        if (y < 0) { std::get<1>(transA->Position) = 0; std::get<1>(transA->Velocity) *= -1; }
        if (x + w > WindowWidth) { std::get<0>(transA->Position) = WindowWidth - w; std::get<0>(transA->Velocity) *= -1; }
        if (y + h > WindowHeight) { std::get<1>(transA->Position) = WindowHeight - h; std::get<1>(transA->Velocity) *= -1; }

        // Colisiones Entidad vs Entidad
        for (auto& entB : entities) 
        {
            if (entA.get() == entB.get()) continue;

            auto colB = static_cast<ColliderComponent*>(entB->GetComponent("Collider"));
            if (!colB) continue;

            float ox = 0, oy = 0;
            if (CheckOverlap(colA, colB, ox, oy)) 
            {
                bool hitBarrier = entB->GetComponent("Barrier") != nullptr;
                bool hitEnemy = isPlayer && entB->GetComponent("Enemy") != nullptr;

                if (hitBarrier || hitEnemy) 
                {
                    // Lógica de Empuje (Push Out)
                    float dx = std::get<0>(colA->MidPoint) - std::get<0>(colB->MidPoint);
                    float dy = std::get<1>(colA->MidPoint) - std::get<1>(colB->MidPoint);

                    if (ox < oy) { // Eje X
                        if (dx > 0) std::get<0>(transA->Position) += ox;
                        else        std::get<0>(transA->Position) -= ox;
                        std::get<0>(transA->Velocity) *= -1.0f;
                    } else { // Eje Y
                        if (dy > 0) std::get<1>(transA->Position) += oy;
                        else        std::get<1>(transA->Position) -= oy;
                        std::get<1>(transA->Velocity) *= -1.0f;
                    }

                    // Actualizar Collider
                    colA->MidPoint = {
                        std::get<0>(transA->Position) + w/2.0f,
                        std::get<1>(transA->Position) + h/2.0f
                    };

                    // Daño y separación extra
                    if (hitEnemy) {
                        world.Emit(DamageEvent());
                        float push = 30.0f; // Empuje fuerte
                        if (ox < oy) std::get<0>(transA->Position) += (dx > 0 ? push : -push);
                        else         std::get<1>(transA->Position) += (dy > 0 ? push : -push);
                    }
                }
            }
        }
    }
}

// =========================================================
// DAMAGE SYSTEM (Invencibilidad)
// =========================================================
DamageSystem::DamageSystem() {
    EventBus::Instance().Suscribe("DamageEvent", [&](Event* e) { pendingDamage = true; });
}

void DamageSystem::Update(World& world, float dt) 
{
    // Reducir cooldown siempre
    for (auto& ent : world.GetEntities()) {
        if (ent->GetComponent("Player")) {
            auto health = static_cast<HealthComponent*>(ent->GetComponent("Health"));
            if (health && health->Cooldown > 0) {
                health->Cooldown -= dt;
            }
        }
    }

    if (pendingDamage) 
    {
        for (auto& ent : world.GetEntities()) {
            if (ent->GetComponent("Player")) {
                auto health = static_cast<HealthComponent*>(ent->GetComponent("Health"));
                
                if (health && health->Cooldown <= 0) {
                    health->Hp -= 1;
                    health->Cooldown = 1.5f; // 1.5 segundos de invencibilidad
                    spdlog::info("DAÑO! Vida: {}", health->Hp);
                    
                    if (health->Hp <= 0) {
                        spdlog::error("GAME OVER - PERDISTE");
                        TooBad = true;
                    }
                }
            }
        }
        pendingDamage = false;
    }
}

// =========================================================
// RENDER SYSTEM (Rotación y Game Over UI)
// =========================================================
RenderSystem::RenderSystem(SDL_Renderer* r, float w, float h) : Renderer(r), WindowWidth(w), WindowHeight(h) {}

void RenderSystem::Update(World& world, float dt) 
{
    for (const auto& entity : world.GetEntities()) 
    {
        auto sprite = static_cast<SpriteComponent*>(entity->GetComponent("Sprite"));
        auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
        auto health = static_cast<HealthComponent*>(entity->GetComponent("Health"));

        if (sprite && transform && sprite->Texture) 
        {
            SDL_FRect destRect;
            destRect.x = std::get<0>(transform->Position);
            destRect.y = std::get<1>(transform->Position);
            
            float w, h;
            SDL_GetTextureSize(sprite->Texture, &w, &h);
            destRect.w = w;
            destRect.h = h;

            // --- 1. LOGICA DE ROTACIÓN (Recuperada) ---
            // Calculamos el ángulo en base a la velocidad actual
            double angle = 0.0;
            float vx = std::get<0>(transform->Velocity);
            float vy = std::get<1>(transform->Velocity);

            // Si se está moviendo, calculamos hacia donde apunta
            if (std::abs(vx) > 0.1f || std::abs(vy) > 0.1f) {
                // atan2 devuelve radianes, convertimos a grados.
                // Sumamos 90 grados porque los sprites suelen mirar hacia "arriba" (Norte)
                angle = (std::atan2(vy, vx) * 180.0 / M_PI) + 90.0;
            }
            // ------------------------------------------

            // --- 2. LOGICA DE PARPADEO ---
            if (health && health->Cooldown > 0) {
                if (static_cast<int>(health->Cooldown * 20.0f) % 2 == 0) {
                    SDL_SetTextureAlphaMod(sprite->Texture, 100);
                } else {
                    SDL_SetTextureAlphaMod(sprite->Texture, 255);
                }
            } else {
                SDL_SetTextureAlphaMod(sprite->Texture, 255);
            }

            // Renderizar con rotación
            SDL_RenderTextureRotated(Renderer, sprite->Texture, nullptr, &destRect, angle, nullptr, SDL_FLIP_NONE);
            
            // Restaurar alpha
            SDL_SetTextureAlphaMod(sprite->Texture, 255);
        }
    }

    // --- 3. PANTALLA DE GAME OVER CON TIEMPO ---
    if (TooBad) {
        // Fondo Rojo Transparente
        SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Renderer, 200, 0, 0, 180); 
        SDL_FRect screen = {0, 0, WindowWidth, WindowHeight};
        SDL_RenderFillRect(Renderer, &screen);

        // Cuadro de Texto
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 200);
        SDL_FRect box = {WindowWidth/2 - 200, WindowHeight/2 - 50, 400, 100};
        SDL_RenderFillRect(Renderer, &box);

        // Texto (Usando debug text de SDL3)
        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
        SDL_SetRenderScale(Renderer, 2.0f, 2.0f); // Texto más grande
        
        std::string msg1 = "GAME OVER";
        std::string msg2 = "Sobreviviste: " + std::to_string((int)world.TimeElapsed) + " seg";
        
        // Ajustar coordenadas según la escala (se divide por la escala)
        SDL_RenderDebugText(Renderer, (WindowWidth/2 - 60)/2, (WindowHeight/2 - 30)/2, msg1.c_str());
        SDL_RenderDebugText(Renderer, (WindowWidth/2 - 120)/2, (WindowHeight/2 + 10)/2, msg2.c_str());
        
        SDL_SetRenderScale(Renderer, 1.0f, 1.0f); // Reset escala
    }
}

// =========================================================
// SPAWN Y TIMER
// =========================================================
SpawnSystem::SpawnSystem(SDL_Renderer* r) : Renderer(r) {
    EventBus::Instance().Suscribe("SpawnEvent", [&](Event* e) { pendingSpawn = true; });
}
void SpawnSystem::Update(World& world, float dt) {
    if (TooBad) return;
    if (pendingSpawn) {
        world.createEntity(Renderer);
        pendingSpawn = false;
    }
}

float TimerSystem::totaltimer = 0.f;
float TimerSystem::spawntimer = 0.f;
TimerSystem::TimerSystem(float interval) : Interval(interval) {}
void TimerSystem::Update(World& world, float dt) {
    if (TooBad) return;
    totaltimer += dt;
    spawntimer += dt;
    world.TimeElapsed = totaltimer;

    if (spawntimer >= Interval) {
        world.Emit(SpawnEvent());
        spawntimer = 0.f;
    }
}