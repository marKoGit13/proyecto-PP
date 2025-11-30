#include "ISystem.h"    

bool ISystem::TooBad = false;
ISystem::ISystem() {
}

ISystem::~ISystem() {
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
    if (TooBad)
    {
        return;
    }
    for (const std::unique_ptr<Entity>& Character : world.GetEntities())
    {
        Component* c_base_coll = Character->GetComponent("Collider");
        ColliderComponent* c_coll = static_cast<ColliderComponent*>(c_base_coll);
        Component* c_base_trans = Character->GetComponent("Transform");
        TransformComponent* c_trans = static_cast<TransformComponent*>(c_base_trans);
        if(Character->GetComponent("Player"))
        {
            for (const std::unique_ptr<Entity>& Character2 : world.GetEntities())
            {
                if(Character2->GetComponent("Enemy"))
                {
                    Component* c2_base_trans = Character2->GetComponent("Transform");
                    TransformComponent* c2_trans = static_cast<TransformComponent*>(c2_base_trans);
                    float distancebetweenX = std::get<0>(c_trans->Position) - std::get<0>(c2_trans->Position);
                    float distancebetweenY = std::get<1>(c_trans->Position) - std::get<1>(c2_trans->Position);
                    float EPS = 1.f;
                    if (c2_trans->BouncingTimer > 0.f)
                    {
                        c2_trans->BouncingTimer--;
                        continue;
                    }
                    if (distancebetweenX != 0)
                    {
                        float direcX = (distancebetweenX)/std::abs(distancebetweenX);
                        std::get<0>(c2_trans->Velocity) = std::abs(std::get<0>(c2_trans->BaseVelocity)) * direcX;
                    }
                    if (distancebetweenY != 0)
                    {
                        float direcY = (distancebetweenY)/std::abs(distancebetweenY);
                        std::get<1>(c2_trans->Velocity) = std::abs(std::get<1>(c2_trans->BaseVelocity)) * direcY;
                    }
                    if (std::abs(distancebetweenX) < EPS)
                    {
                        std::get<0>(c2_trans->Velocity) = 0;
                    }
                    if (std::abs(distancebetweenY) < EPS)
                    {
                        std::get<1>(c2_trans->Velocity) = 0;
                    }
                }
            }
        }
        float distanciaX = std::get<0>(c_trans->Velocity) * dt;
        float distanciaY = std::get<1>(c_trans->Velocity) * dt;
        std::get<0>(c_trans->Position) = std::get<0>(c_trans->Position) + distanciaX;
        std::get<1>(c_trans->Position) = std::get<1>(c_trans->Position) + distanciaY;
        std::get<0>(c_coll->MidPoint) = std::get<0>(c_coll->MidPoint) + distanciaX;
        std::get<1>(c_coll->MidPoint) = std::get<1>(c_coll->MidPoint) + distanciaY; 
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
    if (TooBad)
    {
        std::string timeText = "Time:" + std::to_string(world.TimeElapsed);

        float rectWidth = 400.f;
        float rectHeight = 80.f;

        float rectX = WindowWidth / 2.f - rectWidth / 2.f;
        float rectY = WindowHeight / 2.f - rectHeight / 2.f;

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
        SDL_FRect debugBg{rectX, rectY, rectWidth, rectHeight};
        SDL_RenderFillRect(Renderer, &debugBg);

        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
        SDL_SetRenderScale(Renderer, 4.0f, 4.0f);

        SDL_RenderDebugText(Renderer, rectX  / 4.f, rectY / 4.f, timeText.c_str());

        SDL_SetRenderScale(Renderer, 1.0f, 1.0f);
    }
    else
    {
        for (const std::unique_ptr<Entity>& Character : world.GetEntities())
        {
            Component* c1_base_coll = Character->GetComponent("Collider");
            ColliderComponent* c1_coll = static_cast<ColliderComponent*>(c1_base_coll);
            Component* c1_base_trans = Character->GetComponent("Transform");
            TransformComponent* c1_trans = static_cast<TransformComponent*>(c1_base_trans);
            Component* c1_base_spr = Character->GetComponent("Sprite");
            SpriteComponent* c1_spr = static_cast<SpriteComponent*>(c1_base_spr);

            SDL_FRect rectMegaman{std::get<0>(c1_trans->Position), std::get<1>(c1_trans->Position), std::get<0>(c1_coll->Bounds), std::get<1>(c1_coll->Bounds)};
            double angle = (std::atan2(std::get<1>(c1_trans->Velocity), std::get<0>(c1_trans->Velocity)) * 180.0 / M_PI) + 90.0;
            SDL_RenderTextureRotated(Renderer, c1_spr->Texture, nullptr, &rectMegaman,angle ,nullptr,SDL_FLIP_NONE);
            SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
            SDL_FRect rectCollider{
                c1_coll->getLeft(),
                c1_coll->getBottom(),
                std::get<0>(c1_coll->Bounds),
                std::get<1>(c1_coll->Bounds)
            };
            SDL_RenderRect(Renderer, &rectCollider);
        }
        std::string timeText = "Time:" + std::to_string(world.TimeElapsed);

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
        SDL_FRect debugBg{ 20, 20, 240, 40 };
        SDL_RenderFillRect(Renderer, &debugBg);

        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(Renderer, 30, 30, timeText.c_str());
    }
}

//Sistema de colisiones entre jugador - enemigos
CollisionSystem::CollisionSystem(int windowwidth, int windowheight){
    WindowWidth = windowwidth;
    WindowHeight = windowheight;
}
void CollisionSystem::Update(World& world, float dt)
{
    if (TooBad)
    {
        return;
    }
    for (const std::unique_ptr<Entity>& Character : world.GetEntities())
    {
        Component* c1_base_coll = Character->GetComponent("Collider");
        ColliderComponent* c1_coll = static_cast<ColliderComponent*>(c1_base_coll);
        Component* c1_base_trans = Character->GetComponent("Transform");
        TransformComponent* c1_trans = static_cast<TransformComponent*>(c1_base_trans);
        if(Character->GetComponent("Player")){
            for (const std::unique_ptr<Entity>& Character2 : world.GetEntities())
            {
                if(Character2->GetComponent("Enemy"))
                {
                    Component* c2_base_coll = Character2->GetComponent("Collider");
                    ColliderComponent* c2_coll = static_cast<ColliderComponent*>(c2_base_coll);
                    if (c1_coll->Collision(c2_coll))
                    {
                        world.Emit(DamageEvent());
                    }
                }
                if(Character2->GetComponent("Barrier"))
                {
                    Component* c2_base_coll = Character2->GetComponent("Collider");
                    ColliderComponent* c2_coll = static_cast<ColliderComponent*>(c2_base_coll);
                    if (c1_coll->Collision(c2_coll))
                    {
                        std::get<0>(c1_trans->Velocity) = std::get<0>(c1_trans->Velocity) * -1.0f;
                        std::get<1>(c1_trans->Velocity) = std::get<1>(c1_trans->Velocity) * -1.0f;
                        world.Emit(DamageEvent());
                        break;
                    }
                }
            }
        }
        if(Character->GetComponent("Enemy")){
            for (const std::unique_ptr<Entity>& Character2 : world.GetEntities())
            {
                if(Character2->GetComponent("Barrier"))
                {
                    Component* c2_base_coll = Character2->GetComponent("Collider");
                    ColliderComponent* c2_coll = static_cast<ColliderComponent*>(c2_base_coll);
                    if (c1_coll->Collision(c2_coll))
                    {
                        std::get<0>(c1_trans->Velocity) = std::get<0>(c1_trans->Velocity) * -1.f;
                        std::get<1>(c1_trans->Velocity) = std::get<1>(c1_trans->Velocity) * -1.f;
                        c1_trans->BouncingTimer = 8.f;
                        break;
                    }
                }
            }
        }
        if(!Character->GetComponent("Barrier")){
            if (std::get<0>(c1_trans->Position) >= WindowWidth - std::get<0>(c1_coll->Bounds) || std::get<0>(c1_trans->Position) <= 0)
            {
                std::get<0>(c1_trans->Velocity) = std::get<0>(c1_trans->Velocity) * -1.0f;
            }
            else if (std::get<1>(c1_trans->Position) >= WindowHeight - std::get<1>(c1_coll->Bounds) || std::get<1>(c1_trans->Position) <= 0)
            {
                std::get<1>(c1_trans->Velocity) = std::get<1>(c1_trans->Velocity) * -1.0f;
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
