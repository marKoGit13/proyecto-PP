#include "ISystem.h"
#include <cmath>
#include <string>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool ISystem::TooBad = false;

ISystem::ISystem() {}
ISystem::~ISystem() {}

// Auxiliares
bool CheckOverlap(ColliderComponent* a, ColliderComponent* b, float& overlapX, float& overlapY) {
    float dx = std::get<0>(a->MidPoint) - std::get<0>(b->MidPoint);
    float dy = std::get<1>(a->MidPoint) - std::get<1>(b->MidPoint);
    float combinedHalfW = (std::get<0>(a->Bounds) + std::get<0>(b->Bounds)) / 2.0f;
    float combinedHalfHeight = (std::get<1>(a->Bounds) + std::get<1>(b->Bounds)) / 2.0f;

    if (std::abs(dx) < combinedHalfW && std::abs(dy) < combinedHalfHeight) {
        overlapX = combinedHalfW - std::abs(dx);
        overlapY = combinedHalfHeight - std::abs(dy);
        return true;
    }
    return false;
}

// ========================
// PLAYER INPUT
// ========================
PlayerInputSystem::PlayerInputSystem(bool& isrunning) : isRunning(isrunning) {}

void PlayerInputSystem::Update(World& world, float dt) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) isRunning = false;
    }

    const bool* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE]) isRunning = false;

    if (ISystem::TooBad) return; // Si perdiste, no te mueves

    for (auto& entity : world.GetEntities()) {
        if (entity->GetComponent("Player")) {
            auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
            if (transform) {
                float speed = 350.0f; // Jugador un poco más rápido para compensar enemigos
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

// ========================
// MOVEMENT SYSTEM (VELOCIDAD + FLOCKING)
// ========================
MovementSystem::MovementSystem() {}

void MovementSystem::Update(World& world, float dt) 
{
    if (TooBad) return;

    auto& entities = world.GetEntities();

    for (auto& entity : entities) 
    {
        auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
        auto collider = static_cast<ColliderComponent*>(entity->GetComponent("Collider"));
        
        if (!transform) continue;

        if (entity->GetComponent("Enemy")) {
            auto enemyComp = static_cast<EnemyComponent*>(entity->GetComponent("Enemy"));
            
            // 1. Ir hacia el objetivo (Calculado por el Hilo)
            float targetX = enemyComp->targetX;
            float targetY = enemyComp->targetY;
            
            float myX = std::get<0>(transform->Position);
            float myY = std::get<1>(transform->Position);

            float dirX = targetX - myX;
            float dirY = targetY - myY;
            float len = std::sqrt(dirX*dirX + dirY*dirY);

            float moveX = 0, moveY = 0;
            
            // --- MEJORA: VELOCIDAD DE ENEMIGOS ---
            float speed = 180.0f; // Mucho más rápidos
            // -------------------------------------

            if (len > 0) {
                moveX = (dirX / len) * speed;
                moveY = (dirY / len) * speed;
            }

            // 2. Flocking: Separación (Evitar amontonarse)
            float sepX = 0, sepY = 0;
            int neighbors = 0;

            for (auto& other : entities) {
                if (entity == other) continue;
                if (!other->GetComponent("Enemy")) continue;

                auto otherTrans = static_cast<TransformComponent*>(other->GetComponent("Transform"));
                float odx = myX - std::get<0>(otherTrans->Position);
                float ody = myY - std::get<1>(otherTrans->Position);
                float distSq = odx*odx + ody*ody;

                // Radio de separación ajustado
                if (distSq < 60.0f * 60.0f && distSq > 0.1f) {
                    float dist = std::sqrt(distSq);
                    sepX += (odx / dist) / dist; 
                    sepY += (ody / dist) / dist;
                    neighbors++;
                }
            }

            if (neighbors > 0) {
                float separationForce = 4000.0f; 
                moveX += sepX * separationForce;
                moveY += sepY * separationForce;
            }
            
            std::get<0>(transform->Velocity) = moveX;
            std::get<1>(transform->Velocity) = moveY;
        }

        // Física
        std::get<0>(transform->Position) += std::get<0>(transform->Velocity) * dt;
        std::get<1>(transform->Position) += std::get<1>(transform->Velocity) * dt;

        // Sync Collider
        if (collider) {
            collider->MidPoint = {
                std::get<0>(transform->Position) + std::get<0>(collider->Bounds)/2.0f,
                std::get<1>(transform->Position) + std::get<1>(collider->Bounds)/2.0f
            };
        }
    }
}

// ========================
// COLLISION SYSTEM
// ========================
CollisionSystem::CollisionSystem(int w, int h) : WindowWidth(w), WindowHeight(h) {}

void CollisionSystem::Update(World& world, float dt) 
{
    if (TooBad) return;
    auto& entities = world.GetEntities();

    for (auto& entA : entities) {
        bool isPlayer = entA->GetComponent("Player") != nullptr;
        bool isEnemy = entA->GetComponent("Enemy") != nullptr;
        if (!isPlayer && !isEnemy) continue;

        auto transA = static_cast<TransformComponent*>(entA->GetComponent("Transform"));
        auto colA = static_cast<ColliderComponent*>(entA->GetComponent("Collider"));
        if (!transA || !colA) continue;

        // Bordes
        float x = std::get<0>(transA->Position);
        float y = std::get<1>(transA->Position);
        
        if (x < 0) { std::get<0>(transA->Position) = 0; std::get<0>(transA->Velocity) *= -1; }
        if (y < 0) { std::get<1>(transA->Position) = 0; std::get<1>(transA->Velocity) *= -1; }
        if (x > WindowWidth - 64) { std::get<0>(transA->Position) = WindowWidth - 64; std::get<0>(transA->Velocity) *= -1; }
        if (y > WindowHeight - 64) { std::get<1>(transA->Position) = WindowHeight - 64; std::get<1>(transA->Velocity) *= -1; }

        // Entidades
        for (auto& entB : entities) {
            if (entA.get() == entB.get()) continue;
            auto colB = static_cast<ColliderComponent*>(entB->GetComponent("Collider"));
            if (!colB) continue;

            float ox, oy;
            if (CheckOverlap(colA, colB, ox, oy)) {
                bool hitBarrier = entB->GetComponent("Barrier") != nullptr;
                bool hitEnemy = isPlayer && entB->GetComponent("Enemy") != nullptr;

                if (hitBarrier || hitEnemy) {
                    float dx = std::get<0>(colA->MidPoint) - std::get<0>(colB->MidPoint);
                    float dy = std::get<1>(colA->MidPoint) - std::get<1>(colB->MidPoint);

                    if (ox < oy) {
                        std::get<0>(transA->Position) += (dx > 0 ? ox : -ox);
                        std::get<0>(transA->Velocity) *= -1.0f;
                    } else {
                        std::get<1>(transA->Position) += (dy > 0 ? oy : -oy);
                        std::get<1>(transA->Velocity) *= -1.0f;
                    }

                    colA->MidPoint = {
                        std::get<0>(transA->Position) + std::get<0>(colA->Bounds)/2.0f,
                        std::get<1>(transA->Position) + std::get<1>(colA->Bounds)/2.0f
                    };

                    if (hitEnemy) {
                        world.Emit(DamageEvent());
                        // Empuje violento al recibir daño
                        float push = 50.0f;
                        if (ox < oy) std::get<0>(transA->Position) += (dx > 0 ? push : -push);
                        else         std::get<1>(transA->Position) += (dy > 0 ? push : -push);
                    }
                }
            }
        }
    }
}

// ========================
// DAMAGE SYSTEM
// ========================
DamageSystem::DamageSystem() {
    EventBus::Instance().Suscribe("DamageEvent", [&](Event* e) { pendingDamage = true; });
}

void DamageSystem::Update(World& world, float dt) {
    // Cooldown baja siempre
    for (auto& ent : world.GetEntities()) {
        if (ent->GetComponent("Player")) {
            auto health = static_cast<HealthComponent*>(ent->GetComponent("Health"));
            if (health && health->Cooldown > 0) health->Cooldown -= dt;
        }
    }

    if (pendingDamage) {
        for (auto& ent : world.GetEntities()) {
            if (ent->GetComponent("Player")) {
                auto health = static_cast<HealthComponent*>(ent->GetComponent("Health"));
                if (health && health->Cooldown <= 0) {
                    health->Hp -= 1;
                    health->Cooldown = 1.5f; 
                    if (health->Hp <= 0) TooBad = true;
                }
            }
        }
        pendingDamage = false;
    }
}

// ========================
// RENDER SYSTEM (GAME OVER & ROTACIÓN)
// ========================
RenderSystem::RenderSystem(SDL_Renderer* r, float w, float h) : Renderer(r), WindowWidth(w), WindowHeight(h) {}

void RenderSystem::Update(World& world, float dt) 
{
    for (const auto& entity : world.GetEntities()) {
        auto sprite = static_cast<SpriteComponent*>(entity->GetComponent("Sprite"));
        auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
        auto health = static_cast<HealthComponent*>(entity->GetComponent("Health"));

        if (sprite && transform && sprite->Texture) {
            SDL_FRect destRect;
            destRect.x = std::get<0>(transform->Position);
            destRect.y = std::get<1>(transform->Position);
            float w, h;
            SDL_GetTextureSize(sprite->Texture, &w, &h);
            destRect.w = w; destRect.h = h;

            // Rotación visual
            double angle = 0.0;
            float vx = std::get<0>(transform->Velocity);
            float vy = std::get<1>(transform->Velocity);
            if (std::abs(vx) > 1.0f || std::abs(vy) > 1.0f) {
                angle = (std::atan2(vy, vx) * 180.0 / M_PI) + 90.0;
            }

            // Parpadeo si hay daño
            if (health && health->Cooldown > 0) {
                if (static_cast<int>(health->Cooldown * 20.0f) % 2 == 0) 
                     SDL_SetTextureAlphaMod(sprite->Texture, 80);
                else SDL_SetTextureAlphaMod(sprite->Texture, 255);
            } else {
                SDL_SetTextureAlphaMod(sprite->Texture, 255);
            }

            SDL_RenderTextureRotated(Renderer, sprite->Texture, nullptr, &destRect, angle, nullptr, SDL_FLIP_NONE);
            SDL_SetTextureAlphaMod(sprite->Texture, 255);
        }
    }

    // --- PANTALLA DE GAME OVER ---
    if (TooBad) {
        // Fondo Oscuro para que se lea bien
        SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Renderer, 20, 0, 0, 200); // Rojo muy oscuro casi negro
        SDL_FRect screen = {0, 0, WindowWidth, WindowHeight};
        SDL_RenderFillRect(Renderer, &screen);

        // Mensaje
        SDL_SetRenderDrawColor(Renderer, 255, 50, 50, 255); // Texto rojo brillante
        SDL_SetRenderScale(Renderer, 4.0f, 4.0f); // Texto MUY grande
        
        std::string msgOver = "GAME OVER";
        // Centrar aprox (ajuste manual)
        SDL_RenderDebugText(Renderer, (WindowWidth/4 - 80)/2, (WindowHeight/4 - 20)/2, msgOver.c_str());
        
        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255); // Texto blanco
        SDL_SetRenderScale(Renderer, 2.0f, 2.0f);
        std::string msgTime = "Sobreviviste: " + std::to_string((int)world.TimeElapsed) + " seg";
        SDL_RenderDebugText(Renderer, (WindowWidth/2 - 140)/2, (WindowHeight/2 + 50)/2, msgTime.c_str());
        
        SDL_SetRenderScale(Renderer, 1.0f, 1.0f);
    }
}

// ========================
// SPAWN & TIMER
// ========================
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