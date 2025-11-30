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

// Función auxiliar para cálculo de superposición AABB
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

// PLAYER INPUT SYSTEM
PlayerInputSystem::PlayerInputSystem(bool& isrunning) : isRunning(isrunning) {}

void PlayerInputSystem::Update(World& world, float dt) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) isRunning = false;
    }

    const bool* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE]) isRunning = false;

    // Si perdiste, se desactiva el input
    if (ISystem::TooBad) return; 

    // Movimiento suave basado en teclado
    for (auto& entity : world.GetEntities()) {
        if (entity->GetComponent("Player")) {
            auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
            if (transform) {
                float speed = 350.0f; 
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

// MOVEMENT SYSTEM (FLOCKING + THREADS)
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

        // Lógica específica de enemigos: Integración de Hilo IA y Flocking Local
        if (entity->GetComponent("Enemy")) {
            auto enemyComp = static_cast<EnemyComponent*>(entity->GetComponent("Enemy"));
            
            // Lectura atómica del objetivo dictado por el hilo
            float targetX = enemyComp->targetX;
            float targetY = enemyComp->targetY;
            float myX = std::get<0>(transform->Position);
            float myY = std::get<1>(transform->Position);

            // Cálculo vectorial hacia el objetivo
            float dirX = targetX - myX;
            float dirY = targetY - myY;
            float len = std::sqrt(dirX*dirX + dirY*dirY);

            float moveX = 0, moveY = 0;
            float speed = 180.0f; 

            if (len > 0) {
                moveX = (dirX / len) * speed;
                moveY = (dirY / len) * speed;
            }

            // Flocking: Regla de Separación para evitar superposición de enemigos
            float sepX = 0, sepY = 0;
            int neighbors = 0;

            for (auto& other : entities) {
                if (entity == other) continue;
                if (!other->GetComponent("Enemy")) continue;

                auto otherTrans = static_cast<TransformComponent*>(other->GetComponent("Transform"));
                float odx = myX - std::get<0>(otherTrans->Position);
                float ody = myY - std::get<1>(otherTrans->Position);
                float distSq = odx*odx + ody*ody;

                // Si están demasiado cerca, calcular vector de repulsión
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

        // Aplicación de física básica (Euler integration)
        std::get<0>(transform->Position) += std::get<0>(transform->Velocity) * dt;
        std::get<1>(transform->Position) += std::get<1>(transform->Velocity) * dt;

        // Sincronización del Collider con la nueva posición
        if (collider) {
            collider->MidPoint = {
                std::get<0>(transform->Position) + std::get<0>(collider->Bounds)/2.0f,
                std::get<1>(transform->Position) + std::get<1>(collider->Bounds)/2.0f
            };
        }
    }
}

// COLLISION SYSTEM
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

        // Restricción de bordes de pantalla
        float x = std::get<0>(transA->Position);
        float y = std::get<1>(transA->Position);
        
        if (x < 0) { std::get<0>(transA->Position) = 0; std::get<0>(transA->Velocity) *= -1; }
        if (y < 0) { std::get<1>(transA->Position) = 0; std::get<1>(transA->Velocity) *= -1; }
        if (x > WindowWidth - 64) { std::get<0>(transA->Position) = WindowWidth - 64; std::get<0>(transA->Velocity) *= -1; }
        if (y > WindowHeight - 64) { std::get<1>(transA->Position) = WindowHeight - 64; std::get<1>(transA->Velocity) *= -1; }

        // Resolución de colisiones entidad vs entidad
        for (auto& entB : entities) {
            if (entA.get() == entB.get()) continue;
            auto colB = static_cast<ColliderComponent*>(entB->GetComponent("Collider"));
            if (!colB) continue;

            float ox, oy;
            if (CheckOverlap(colA, colB, ox, oy)) {
                bool hitBarrier = entB->GetComponent("Barrier") != nullptr;
                bool hitEnemy = isPlayer && entB->GetComponent("Enemy") != nullptr;

                if (hitBarrier || hitEnemy) {
                    // Algoritmo Push Out: empujar fuera del objeto en el eje de menor penetración
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
                        // Empuje extra fuerte al recibir daño para feedback visual
                        float push = 50.0f;
                        if (ox < oy) std::get<0>(transA->Position) += (dx > 0 ? push : -push);
                        else         std::get<1>(transA->Position) += (dy > 0 ? push : -push);
                    }
                }
            }
        }
    }
}

// DAMAGE SYSTEM
DamageSystem::DamageSystem() {
    EventBus::Instance().Suscribe("DamageEvent", [&](Event* e) { pendingDamage = true; });
}

void DamageSystem::Update(World& world, float dt) {
    // Reducción constante del cooldown de invencibilidad
    for (auto& ent : world.GetEntities()) {
        if (ent->GetComponent("Player")) {
            auto health = static_cast<HealthComponent*>(ent->GetComponent("Health"));
            if (health && health->Cooldown > 0) health->Cooldown -= dt;
        }
    }

    // Procesamiento del evento de daño
    if (pendingDamage) {
        for (auto& ent : world.GetEntities()) {
            if (ent->GetComponent("Player")) {
                auto health = static_cast<HealthComponent*>(ent->GetComponent("Health"));
                // Solo aplicar daño si no es invencible
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

// RENDER SYSTEM (UI & EFECTOS)
RenderSystem::RenderSystem(SDL_Renderer* r, float w, float h, TTF_Font* font) 
    : Renderer(r), WindowWidth(w), WindowHeight(h), GameFont(font) {}

void RenderSystem::Update(World& world, float dt) 
{
    for (const auto& entity : world.GetEntities()) 
    {
        auto sprite = static_cast<SpriteComponent*>(entity->GetComponent("Sprite"));
        auto transform = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
        auto health = static_cast<HealthComponent*>(entity->GetComponent("Health"));
        auto enemy = static_cast<EnemyComponent*>(entity->GetComponent("Enemy"));

        if (sprite && transform && sprite->Texture) 
        {
            SDL_FRect destRect;
            destRect.x = std::get<0>(transform->Position);
            destRect.y = std::get<1>(transform->Position);
            float w, h;
            SDL_GetTextureSize(sprite->Texture, &w, &h);
            destRect.w = w;
            destRect.h = h;

            // Cálculo de rotación visual basada en movimiento
            double angle = 0.0;
            float vx = std::get<0>(transform->Velocity);
            float vy = std::get<1>(transform->Velocity);
            if (std::abs(vx) > 1.0f || std::abs(vy) > 1.0f) {
                angle = (std::atan2(vy, vx) * 180.0 / M_PI) + 90.0;
            }

            // Efecto de parpadeo durante invencibilidad
            if (health && health->Cooldown > 0) {
                if (static_cast<int>(health->Cooldown * 20.0f) % 2 == 0) 
                     SDL_SetTextureAlphaMod(sprite->Texture, 100);
                else SDL_SetTextureAlphaMod(sprite->Texture, 255);
            } else {
                SDL_SetTextureAlphaMod(sprite->Texture, 255);
            }

            SDL_RenderTextureRotated(Renderer, sprite->Texture, nullptr, &destRect, angle, nullptr, SDL_FLIP_NONE);
            SDL_SetTextureAlphaMod(sprite->Texture, 255);

            // INTERFAZ DE USUARIO

            // Barra de Vida sobre el jugador
            if (entity->GetComponent("Player") && health) {
                float barW = 64.0f; float barH = 6.0f;
                float barX = destRect.x + (destRect.w - barW)/2;
                float barY = destRect.y - 20;
                
                // Fondo rojo
                SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
                SDL_FRect bg = {barX, barY, barW, barH};
                SDL_RenderFillRect(Renderer, &bg);

                // Barra verde proporcional
                float hpPct = std::max(0.0f, (float)health->Hp / (float)health->MaxHp);
                SDL_SetRenderDrawColor(Renderer, 0, 255, 0, 255);
                SDL_FRect fg = {barX, barY, barW * hpPct, barH};
                SDL_RenderFillRect(Renderer, &fg);

                // Texto de HP
                if (GameFont) {
                    std::string hpTxt = "HP: " + std::to_string(health->Hp);
                    SDL_Color col = {255,255,255,255};
                    SDL_Surface* s = TTF_RenderText_Solid(GameFont, hpTxt.c_str(), 0, col);
                    if(s){
                        SDL_Texture* t = SDL_CreateTextureFromSurface(Renderer, s);
                        SDL_FRect tr = {barX + 10, barY - 15, (float)s->w, (float)s->h};
                        SDL_RenderTexture(Renderer, t, nullptr, &tr);
                        SDL_DestroySurface(s); SDL_DestroyTexture(t);
                    }
                }
            }

            // Etiqueta de Rol sobre enemigos
            if (enemy && GameFont) {
                std::string rTxt = (enemy->role == EnemyRole::CHASER) ? "CHASER" : "FLANKER";
                SDL_Color col = {255, 200, 0, 255};
                if(enemy->role == EnemyRole::CHASER) col = {255, 100, 100, 255};
                
                SDL_Surface* s = TTF_RenderText_Solid(GameFont, rTxt.c_str(), 0, col);
                if(s){
                    SDL_Texture* t = SDL_CreateTextureFromSurface(Renderer, s);
                    SDL_FRect tr = {destRect.x + (destRect.w - s->w)/2, destRect.y - 15, (float)s->w, (float)s->h};
                    SDL_RenderTexture(Renderer, t, nullptr, &tr);
                    SDL_DestroySurface(s); SDL_DestroyTexture(t);
                }
            }
        }
    }

    // PANTALLA DE GAME OVER 
    if (TooBad) {
        // Fondo oscuro semitransparente
        SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Renderer, 20, 0, 0, 220);
        SDL_FRect screen = {0, 0, WindowWidth, WindowHeight};
        SDL_RenderFillRect(Renderer, &screen);
        
        // Mensaje principal
        SDL_SetRenderDrawColor(Renderer, 255, 50, 50, 255);
        SDL_SetRenderScale(Renderer, 4.0f, 4.0f);
        SDL_RenderDebugText(Renderer, (WindowWidth/4 - 80)/2, (WindowHeight/4 - 20)/2, "GAME OVER");
        
        // Mensaje de tiempo sobrevivido
        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
        SDL_SetRenderScale(Renderer, 2.0f, 2.0f);
        std::string msg = "Survived: " + std::to_string((int)world.TimeElapsed) + "s";
        SDL_RenderDebugText(Renderer, (WindowWidth/2 - 80)/2, (WindowHeight/2 + 50)/2, msg.c_str());
        SDL_SetRenderScale(Renderer, 1.0f, 1.0f);
    }
}

// SPAWN & TIMER
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