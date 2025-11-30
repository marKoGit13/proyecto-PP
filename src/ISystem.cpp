#include "ISystem.h"
#include <cmath>
#include <string>

// =========================================================
// FUNCIONES AUXILIARES
// =========================================================
bool CheckCollision(ColliderComponent* a, ColliderComponent* b, float& overlapX, float& overlapY) {
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

ISystem::ISystem() {}
ISystem::~ISystem() {}
bool ISystem::TooBad = false;

// =========================================================
// PLAYER INPUT SYSTEM (Arreglado ESC)
// =========================================================
PlayerInputSystem::PlayerInputSystem(bool& isrunning) : isRunning(isrunning) {}

void PlayerInputSystem::Update(World& world, float dt) {
    const bool* state = SDL_GetKeyboardState(nullptr);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) isRunning = false;
    }

    // --- CORRECCIÓN: Salir con ESCAPE ---
    if (state[SDL_SCANCODE_ESCAPE]) {
        isRunning = false;
    }
    
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
// SPAWN SYSTEM (Arreglado Enemigos)
// =========================================================
SpawnSystem::SpawnSystem(SDL_Renderer* r) : Renderer(r) {}

void SpawnSystem::Update(World& world, float dt) {
    static float timer = 0.0f;
    static int enemyCounter = 0;
    float spawnInterval = 2.0f; // Spawnear cada 2 segundos

    timer += dt;

    if (timer >= spawnInterval) {
        timer = 0.0f;

        // Intentar spawnear enemigo
        float enemyW = 64.0f;
        float enemyH = 64.0f;
        
        // Intentar 10 veces encontrar una posicion libre
        for (int i = 0; i < 10; i++) {
            float randX = NumberRandomizer(true, 50.0f, 1800.0f); // Asumiendo pantalla 1920
            float randY = NumberRandomizer(true, 50.0f, 400.0f);  // Parte superior de la pantalla

            if (world.IsAreaFree(randX, randY, enemyW, enemyH)) {
                // Crear Enemigo
                std::unique_ptr<Entity> Enemy = std::make_unique<Entity>("Enemy_" + std::to_string(enemyCounter++));
                
                // NOTA: Asegúrate que la ruta sea correcta. Uso la misma de tus assets.
                world.AddComponentToEntity(Enemy->GetId(), std::unique_ptr<TransformComponent>(new TransformComponent(randX, randY, 0, 0)));
                world.AddComponentToEntity(Enemy->GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent("./assets/EnemySprite.png", Renderer)));
                world.AddComponentToEntity(Enemy->GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(enemyW, enemyH, {randX + enemyW/2, randY + enemyH/2})));
                world.AddComponentToEntity(Enemy->GetId(), std::unique_ptr<EnemyComponent>(new EnemyComponent()));
                
                // Health opcional para enemigos
                world.AddComponentToEntity(Enemy->GetId(), std::unique_ptr<HealthComponent>(new HealthComponent(1)));

                world.AddEntity(std::move(Enemy));
                break; // Éxito, salir del loop de intentos
            }
        }
    }
}

// =========================================================
// MOVEMENT SYSTEM 
// =========================================================
MovementSystem::MovementSystem() {}

void MovementSystem::Update(World& world, float dt) 
{
    Entity* player = nullptr;
    TransformComponent* playerTrans = nullptr;
    
    // Buscar jugador
    for (auto& entity : world.GetEntities()) {
        if (entity->GetComponent("Player")) {
            player = entity.get();
            playerTrans = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
            break;
        }
    }

    for (auto& entity : world.GetEntities()) 
    {
        // 1. IA Simple: Perseguir jugador
        if (playerTrans && entity->GetComponent("Enemy")) {
            auto enemyTrans = static_cast<TransformComponent*>(entity->GetComponent("Transform"));
            if(enemyTrans) {
                float dx = std::get<0>(playerTrans->Position) - std::get<0>(enemyTrans->Position);
                float dy = std::get<1>(playerTrans->Position) - std::get<1>(enemyTrans->Position);
                float length = std::sqrt(dx*dx + dy*dy);
                
                if (length > 0) {
                    float speed = 100.0f;
                    std::get<0>(enemyTrans->Velocity) = (dx / length) * speed;
                    std::get<1>(enemyTrans->Velocity) = (dy / length) * speed;
                }
            }
        }

        // 2. Movimiento Física
        auto transform = entity->GetComponent("Transform");
        if (transform) 
        {
            TransformComponent* trans = static_cast<TransformComponent*>(transform);
            std::get<0>(trans->Position) += std::get<0>(trans->Velocity) * dt;
            std::get<1>(trans->Position) += std::get<1>(trans->Velocity) * dt;

            // Sincronizar Collider
            auto collider = entity->GetComponent("Collider");
            if (collider) {
                ColliderComponent* col = static_cast<ColliderComponent*>(collider);
                float w = std::get<0>(col->Bounds);
                float h = std::get<1>(col->Bounds);
                col->MidPoint = {
                    std::get<0>(trans->Position) + w / 2.0f,
                    std::get<1>(trans->Position) + h / 2.0f
                };
            }
        }
    }
}

// =========================================================
// COLLISION SYSTEM
// =========================================================
CollisionSystem::CollisionSystem(int w, int h) : WindowWidth(w), WindowHeight(h) {}

void CollisionSystem::Update(World& world, float dt) 
{
    auto& entities = world.GetEntities();
    for (auto& entityA : entities) 
    {
        bool isPlayer = entityA->GetComponent("Player") != nullptr;
        bool isEnemy = entityA->GetComponent("Enemy") != nullptr;
        if (!isPlayer && !isEnemy) continue;

        auto transA = static_cast<TransformComponent*>(entityA->GetComponent("Transform"));
        auto colA = static_cast<ColliderComponent*>(entityA->GetComponent("Collider"));
        if (!transA || !colA) continue;

        // Límites pantalla
        float x = std::get<0>(transA->Position);
        float y = std::get<1>(transA->Position);
        float w = std::get<0>(colA->Bounds);
        float h = std::get<1>(colA->Bounds);

        if (x < 0) { std::get<0>(transA->Position) = 0; std::get<0>(transA->Velocity) *= -1; }
        if (y < 0) { std::get<1>(transA->Position) = 0; std::get<1>(transA->Velocity) *= -1; }
        if (x + w > WindowWidth) { std::get<0>(transA->Position) = WindowWidth - w; std::get<0>(transA->Velocity) *= -1; }
        if (y + h > WindowHeight) { std::get<1>(transA->Position) = WindowHeight - h; std::get<1>(transA->Velocity) *= -1; }

        for (auto& entityB : entities) 
        {
            if (entityA.get() == entityB.get()) continue;
            auto colB = static_cast<ColliderComponent*>(entityB->GetComponent("Collider"));
            if (!colB) continue;

            float ox = 0, oy = 0;
            if (CheckCollision(colA, colB, ox, oy)) 
            {
                if (entityB->GetComponent("Barrier")) 
                {
                    float dx = std::get<0>(colA->MidPoint) - std::get<0>(colB->MidPoint);
                    float dy = std::get<1>(colA->MidPoint) - std::get<1>(colB->MidPoint);

                    if (ox < oy) { // X
                        if (dx > 0) std::get<0>(transA->Position) += ox;
                        else        std::get<0>(transA->Position) -= ox;
                        std::get<0>(transA->Velocity) *= -1;
                    } else { // Y
                        if (dy > 0) std::get<1>(transA->Position) += oy;
                        else        std::get<1>(transA->Position) -= oy;
                        std::get<1>(transA->Velocity) *= -1;
                    }
                    colA->MidPoint = { std::get<0>(transA->Position) + w/2.0f, std::get<1>(transA->Position) + h/2.0f };
                }
                else if (isPlayer && entityB->GetComponent("Enemy")) {
                    world.Emit(DamageEvent());
                }
            }
        }
    }
}

// =========================================================
// RENDER SYSTEM
// =========================================================
RenderSystem::RenderSystem(SDL_Renderer* r, float w, float h) : Renderer(r), WindowWidth(w), WindowHeight(h) {}

void RenderSystem::Update(World& world, float dt) 
{
    for (auto& entity : world.GetEntities()) 
    {
        auto sprite = entity->GetComponent("Sprite"); 
        auto transform = entity->GetComponent("Transform");
        
        if (sprite && transform) 
        {
            SpriteComponent* spr = static_cast<SpriteComponent*>(sprite);
            TransformComponent* trans = static_cast<TransformComponent*>(transform);
            
            if (!spr->Texture) continue;

            SDL_FRect destRect;
            destRect.x = std::get<0>(trans->Position);
            destRect.y = std::get<1>(trans->Position);
            float w = 0, h = 0;
            SDL_GetTextureSize(spr->Texture, &w, &h);
            destRect.w = w;
            destRect.h = h;

            // Invencibilidad
            auto health = entity->GetComponent("Health"); 
            if (health) {
                HealthComponent* hp = static_cast<HealthComponent*>(health);
                if (hp->Cooldown > 0) {
                    hp->Cooldown -= dt; // --- IMPORTANTE: BAJAR EL COOLDOWN AQUÍ O EN DAMAGE SYSTEM ---
                    if ((int)(hp->Cooldown * 15) % 2 == 0) SDL_SetTextureAlphaMod(spr->Texture, 100);
                    else SDL_SetTextureAlphaMod(spr->Texture, 255);
                } else {
                    SDL_SetTextureAlphaMod(spr->Texture, 255);
                }
            }
            SDL_RenderTexture(Renderer, spr->Texture, nullptr, &destRect);
            SDL_SetTextureAlphaMod(spr->Texture, 255);
        }
    }
}

DamageSystem::DamageSystem() {}
void DamageSystem::Update(World& world, float dt) {
    // Si manejas el cooldown de daño aqui, asegúrate de restarlo
    for(auto& entity : world.GetEntities()) {
        auto health = entity->GetComponent("Health");
        if(health) {
            static_cast<HealthComponent*>(health)->Cooldown -= dt;
            if(static_cast<HealthComponent*>(health)->Cooldown < 0) 
                static_cast<HealthComponent*>(health)->Cooldown = 0;
        }
    }
}

float TimerSystem::totaltimer = 0;
float TimerSystem::spawntimer = 0;
TimerSystem::TimerSystem(float interval) : Interval(interval) {}
void TimerSystem::Update(World& world, float dt) {
    totaltimer += dt;
}