#include "ISystem.h"    
#include <cmath>

bool ISystem::TooBad = false;
ISystem::ISystem() {
}

ISystem::~ISystem() {
}

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

//Sistema de input del teclado del jugador
PlayerInputSystem::PlayerInputSystem(bool& isrunning)
    : isRunning(isrunning)
{
    //Controlador del movimiento del jugador
}
void PlayerInputSystem::Update(World& world, float dt)
{
    SDL_Event event;
    Entity* Player = world.GetEntityByName("Player_0");
    Component* player_base_trans = Player->GetComponent("Transform");
    TransformComponent* player_trans = static_cast<TransformComponent*>(player_base_trans);
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {   
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.repeat) break;
                if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_RETURN) 
                {
                    isRunning = false;
                }
                if (event.key.key == SDLK_A) 
                {
                    std::get<0>(player_trans->Velocity) = -150.f;
                }
                if (event.key.key == SDLK_S)
                {
                    std::get<1>(player_trans->Velocity) = 150.f;
                }
                if (event.key.key == SDLK_D)
                {
                    std::get<0>(player_trans->Velocity) = 150.f;
                }
                if (event.key.key == SDLK_W)
                {
                    std::get<1>(player_trans->Velocity) = -150.f;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_A) 
                {
                    std::get<0>(player_trans->Velocity) = 0.f;
                }
                if (event.key.key == SDLK_S)
                {
                    std::get<1>(player_trans->Velocity) = 0.f;
                }
                if (event.key.key == SDLK_D)
                {
                    std::get<0>(player_trans->Velocity) = 0.f;
                }
                if (event.key.key == SDLK_W)
                {
                    std::get<1>(player_trans->Velocity) = 0.f;
                }
                break;
        }
    }
}

//Sistema de movimiento del jugador y de los enemigos
MovementSystem::MovementSystem()
{
    
}
void MovementSystem::Update(World& world, float dt)
{
    // AQUI PONER LOGICA DE IA SIMPLIFICADA (O mover a un EnemySystem aparte)
    // Para que los enemigos te sigan antes de aplicar el movimiento:
    Entity* player = world.GetEntityById("Player_0"); // Asegúrate de tener una forma de obtener al player
    TransformComponent* playerTrans = nullptr;
    if(player) playerTrans = static_cast<TransformComponent*>(player->GetComponent("TransformComponent"));

    for (auto& entity : world.GetEntities()) 
    {
        // 1. Lógica de Persecución (IA Simple integrada aquí por ahora)
        if (playerTrans && entity->GetComponent("EnemyComponent")) {
            auto enemyTrans = static_cast<TransformComponent*>(entity->GetComponent("TransformComponent"));
            float dx = std::get<0>(playerTrans->Position) - std::get<0>(enemyTrans->Position);
            float dy = std::get<1>(playerTrans->Position) - std::get<1>(enemyTrans->Position);
            
            // Normalizar dirección
            float length = std::sqrt(dx*dx + dy*dy);
            if (length > 0) {
                float speed = 100.0f; // Velocidad de los enemigos
                std::get<0>(enemyTrans->Velocity) = (dx / length) * speed;
                std::get<1>(enemyTrans->Velocity) = (dy / length) * speed;
            }
        }

        // 2. Lógica Física de Movimiento (APLICAR VELOCIDAD)
        auto transform = entity->GetComponent("TransformComponent");
        auto collider = entity->GetComponent("ColliderComponent");

        if (transform) 
        {
            TransformComponent* trans = static_cast<TransformComponent*>(transform);
            
            // Mover posición basado en velocidad y tiempo
            std::get<0>(trans->Position) += std::get<0>(trans->Velocity) * dt;
            std::get<1>(trans->Position) += std::get<1>(trans->Velocity) * dt;

            // SINCRONIZAR COLLIDER (ESTO FALTABA Y ES CLAVE)
            if (collider) {
                ColliderComponent* col = static_cast<ColliderComponent*>(collider);
                // Asumiendo que Position es la esquina superior izquierda
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

//Sistema de renderizado del jugador, los enemigos y el timer
RenderSystem::RenderSystem(SDL_Renderer* renderer, float windowwidth, float windowheight)
{
    WindowWidth = windowwidth;
    WindowHeight = windowheight;
    Renderer = renderer;
}
void RenderSystem::Update(World& world, float dt)
{
    for (auto& entity : world.GetEntities()) 
    {
        auto sprite = entity->GetComponent("SpriteComponent");
        auto transform = entity->GetComponent("TransformComponent");
        
        if (sprite && transform) 
        {
            SpriteComponent* spr = static_cast<SpriteComponent*>(sprite);
            TransformComponent* trans = static_cast<TransformComponent*>(transform);
            
            SDL_FRect destRect;
            destRect.x = std::get<0>(trans->Position);
            destRect.y = std::get<1>(trans->Position);
            
            // Consultar tamaño textura
            float w, h;
            SDL_GetTextureSize(spr->Texture, &w, &h);
            destRect.w = w;
            destRect.h = h;

            // Lógica de parpadeo si tiene vida y cooldown
            auto health = entity->GetComponent("HealthComponent");
            if (health) {
                HealthComponent* hp = static_cast<HealthComponent*>(health);
                if (hp->Cooldown > 0) {
                    // Parpadeo rápido
                    if ((int)(hp->Cooldown * 10) % 2 == 0) {
                        SDL_SetTextureAlphaMod(spr->Texture, 128); // Semitransparente
                    } else {
                        SDL_SetTextureAlphaMod(spr->Texture, 255);
                    }
                } else {
                    SDL_SetTextureAlphaMod(spr->Texture, 255);
                }
            }

            SDL_RenderTexture(Renderer, spr->Texture, nullptr, &destRect);
            
            // Restaurar Alpha por si acaso
            SDL_SetTextureAlphaMod(spr->Texture, 255);
        }
    }
}

//Sistema de colisiones entre jugador - enemigos
CollisionSystem::CollisionSystem(int windowwidth, int windowheight){
    WindowWidth = windowwidth;
    WindowHeight = windowheight;
}
void CollisionSystem::Update(World& world, float dt)
{
    auto& entities = world.GetEntities();

    // Solo chequeamos entidades dinámicas (Jugador y Enemigos)
    for (auto& entityA : entities) 
    {
        bool isPlayer = entityA->GetComponent("PlayerComponent") != nullptr;
        bool isEnemy = entityA->GetComponent("EnemyComponent") != nullptr;
        
        if (!isPlayer && !isEnemy) continue; // Si es pared, saltar

        auto transA = static_cast<TransformComponent*>(entityA->GetComponent("TransformComponent"));
        auto colA = static_cast<ColliderComponent*>(entityA->GetComponent("ColliderComponent"));

        if (!transA || !colA) continue;

        // Limites de pantalla (Rebote simple)
        float x = std::get<0>(transA->Position);
        float y = std::get<1>(transA->Position);
        float w = std::get<0>(colA->Bounds);
        float h = std::get<1>(colA->Bounds);

        if (x < 0) { std::get<0>(transA->Position) = 0; std::get<0>(transA->Velocity) *= -1; }
        if (y < 0) { std::get<1>(transA->Position) = 0; std::get<1>(transA->Velocity) *= -1; }
        if (x + w > WindowWidth) { std::get<0>(transA->Position) = WindowWidth - w; std::get<0>(transA->Velocity) *= -1; }
        if (y + h > WindowHeight) { std::get<1>(transA->Position) = WindowHeight - h; std::get<1>(transA->Velocity) *= -1; }

        // Colisiones con otros objetos
        for (auto& entityB : entities) 
        {
            if (entityA.get() == entityB.get()) continue;

            auto colB = static_cast<ColliderComponent*>(entityB->GetComponent("ColliderComponent"));
            if (!colB) continue;

            float ox = 0, oy = 0;
            if (CheckCollision(colA, colB, ox, oy)) 
            {
                // CHOQUE CON BARRERA (RESOLUCIÓN FÍSICA)
                if (entityB->GetComponent("BarrierComponent")) 
                {
                    // Calcular vector dirección desde B hacia A
                    float dx = std::get<0>(colA->MidPoint) - std::get<0>(colB->MidPoint);
                    float dy = std::get<1>(colA->MidPoint) - std::get<1>(colB->MidPoint);

                    if (ox < oy) { // Corregir en X
                        if (dx > 0) std::get<0>(transA->Position) += ox;
                        else        std::get<0>(transA->Position) -= ox;
                        std::get<0>(transA->Velocity) *= -1; // Rebote
                    } else { // Corregir en Y
                        if (dy > 0) std::get<1>(transA->Position) += oy;
                        else        std::get<1>(transA->Position) -= oy;
                        std::get<1>(transA->Velocity) *= -1; // Rebote
                    }
                    
                    // Actualizar Collider tras mover
                    colA->MidPoint = {
                        std::get<0>(transA->Position) + w/2.0f,
                        std::get<1>(transA->Position) + h/2.0f
                    };
                }
                // CHOQUE JUGADOR - ENEMIGO
                else if (isPlayer && entityB->GetComponent("EnemyComponent")) {
                    world.Emit(DamageEvent());
                }
            }
        }
    }
}

//Sistema de daño, llama a los frames de invulnerabilidad antes de poder golpear otra vez
DamageSystem::DamageSystem()
{
    EventBus::Instance().Suscribe("DamageEvent",
    [&](Event* e)
    {
        pendingDamage = true;
    });
}
void DamageSystem::Update(World& world, float dt)
{
    if (TooBad)
    {
        return;
    }
    if (pendingDamage)
    {
        for (const std::unique_ptr<Entity>& Character : world.GetEntities())
        {
            if(Character->GetComponent("Player"))
            {
                Component* c_base_health = Character->GetComponent("Health");
                HealthComponent* c_health = static_cast<HealthComponent*>(c_base_health);
                if (c_health->Cooldown > 0)
                {
                    c_health->Cooldown -= dt;
                    spdlog::info("Frames de inmunidad");
                }
                else{
                    c_health->Hp = c_health->Hp - 1;
                    c_health->Cooldown = 1.0f;
                    spdlog::info("Se recibió daño");
                    if (c_health->Hp <= 0)
                    {
                        TooBad = true;
                    }
                }
                
            }
        }
        pendingDamage = false;
    }
}

//Sistema de spawn de enemigos, los spawnea en base al tiempo guardado en el json
SpawnSystem::SpawnSystem(SDL_Renderer* renderer)
    : Renderer(renderer)
{
    EventBus::Instance().Suscribe("SpawnEvent",
    [&](Event* e)
    {
        pendingSpawn = true;
        spdlog::info("Se spawneó un enemigo");
    });
}
void SpawnSystem::Update(World& world, float dt)
{
    if (TooBad)
    {
        return;
    }
    if (pendingSpawn)
    {
        world.createEntity(Renderer);
        pendingSpawn = false;
    }
}

//Sistema de manejo de tiempos, de spawn, y tiempo total.
float TimerSystem::totaltimer = 0.f;
float TimerSystem::spawntimer = 0.f;
TimerSystem::TimerSystem(float interval)
    : Interval(interval)
{
}
void TimerSystem::Update(World& world, float dt)
{
    if (TooBad)
    {
        return;
    }
    totaltimer += dt;
    spawntimer += dt;
    world.TimeElapsed = totaltimer;

    if (spawntimer >= Interval){
        world.Emit(SpawnEvent());
        spawntimer = 0.f;
    }
}
