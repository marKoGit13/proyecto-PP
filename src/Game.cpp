#include "Game.h"
#include <cmath> 
#include <string>

Game::Game() : world()
{
    window = nullptr;
    renderer = nullptr;

    //Lectura del Json y agarre de sus datos
    std::tuple<int, std::string, std::string, std::string, std::string, float, float, float> Information = ReadFromConfigFile("./assets/data.json");
    cantidad = std::get<0>(Information);
    rutaImagenCharacter = std::get<1>(Information);
    rutaImagenBackground = std::get<2>(Information);
    rutaImagenEnemy = std::get<3>(Information);
    rutaImagenBarrier = std::get<4>(Information);
    intervalo = std::get<7>(Information);
    AnchoAlto = {1920, 1080};
    isRunning = false;
}

Game::~Game() {
    if(globalFont) TTF_CloseFont(globalFont);
    TTF_Quit();
    SDL_Quit();
}

void Game::Initialize()
{
    this->window = SDL_CreateWindow("Space Invaders WolfPack", std::get<0>(this->AnchoAlto), std::get<1>(this->AnchoAlto), SDL_WINDOW_FULLSCREEN);
    this->renderer =  SDL_CreateRenderer(window, nullptr);

    if (!SDL_Init(SDL_INIT_VIDEO)) spdlog::error("Error SDL Init");
    
    // --- INICIALIZAR TTF ---
    if (!TTF_Init()) {
        spdlog::error("Error TTF Init: {}", SDL_GetError());
    }
    
    // Cargar Fuente (ASEGURATE QUE EXISTA ESTE ARCHIVO en assets/)
    this->globalFont = TTF_OpenFont("./assets/arial.ttf", 24); 
    if (!this->globalFont) {
        spdlog::error("Error cargando fuente: {}", SDL_GetError());
        // Fallback: intenta cargar una por defecto si tienes o deja nulo
    }
    // -----------------------

    SDL_Surface* surfacefondo = IMG_Load(rutaImagenBackground.c_str());
    if (surfacefondo) {
        this->textureBackground = SDL_CreateTextureFromSurface(this->renderer, surfacefondo);
        SDL_DestroySurface(surfacefondo);
    }

    this->movementsystem = std::make_unique<MovementSystem>();
    this->playerinputsystem = std::make_unique<PlayerInputSystem>(this->isRunning);
    
    // --- PASAR LA FUENTE AL RENDER SYSTEM ---
    this->rendersystem = std::make_unique<RenderSystem>(this->renderer, std::get<0>(this->AnchoAlto), std::get<1>(this->AnchoAlto), this->globalFont);
    // ----------------------------------------
    
    this->collisionsystem = std::make_unique<CollisionSystem>(std::get<0>(this->AnchoAlto), std::get<1>(this->AnchoAlto));
    this->damagesystem = std::make_unique<DamageSystem>();
    this->spawnsystem = std::make_unique<SpawnSystem>(this->renderer);
    this->timersystem = std::make_unique<TimerSystem>(this->intervalo);
}

void Game::Run()
{
    Start();
    Uint64 lastFrame = SDL_GetTicks();
    this->isRunning = true;
    
    // Variables para el timer visual
    float totalTime = 0.0f;

    while (isRunning) {
        Uint64 now = SDL_GetTicks();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;
        totalTime += deltaTime;

        // --- CORRECCIÓN: MOSTRAR TIEMPO EN TÍTULO ---
        std::string title = "Space Invaders - Time: " + std::to_string((int)totalTime);
        SDL_SetWindowTitle(window, title.c_str());
        // --------------------------------------------

        ProcessInput(deltaTime);
        Update(deltaTime);
        Render(deltaTime);
    }

}

void Game::Start()
{
    std::tuple<int, std::string, std::string, std::string, std::string, float, float, float> Information = ReadFromConfigFile("./assets/data.json");
    float ancho = std::get<5>(Information);
    float alto= std::get<6>(Information);
    float PosicionX = 960.f;
    float PosicionY = 900.f;
    
    std::unique_ptr<Entity> Character = std::make_unique<Entity>("Player_0");
    Entity& refCharacter = *Character;
    this->world.AddEntity(std::move(Character));

    this->world.AddComponentToEntity(refCharacter.GetId(),std::unique_ptr<TransformComponent>(new TransformComponent(PosicionX, PosicionY, 0.f, 0.f)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenCharacter, renderer)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, {PosicionX + ancho/2, PosicionY + alto/2})));
    
    // --- CAMBIO: 5 PUNTOS DE VIDA ---
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<HealthComponent>(new HealthComponent(5)));
    // --------------------------------
    
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<PlayerComponent>(new PlayerComponent()));

    // (El resto de la generación de barreras se mantiene igual...)
    int BarrierCounter = 0;
    for (size_t j = 64; j < std::get<1>(this->AnchoAlto) - 64; j = j + 64)
    {
        for (size_t i = 64; i < std::get<0>(this->AnchoAlto); i = i + 64) 
        {
            float BarrierProbability = NumberRandomizer(true, 0.f, 100.f);
            if (BarrierProbability < 5.f) 
            {
                int direction = NumberRandomizer(false, 0, 3);
                int count = NumberRandomizer(false, 3, 6);
                float CurX = i; float CurY = j;
                int OffX = 0, OffY = 0;
                switch (direction) { case 0: OffX = 64; break; case 1: OffY = 64; break; case 2: OffX = -64; break; case 3: OffY = -64; break; }
                for (int k = 0; k < count; k++) 
                {
                    if (this->world.IsAreaFree(CurX, CurY, ancho, alto)) {
                        std::unique_ptr<Entity> Barrier = std::make_unique<Entity>("Barrier_" + std::to_string(BarrierCounter));
                        Entity& refBarrier = *Barrier;
                        this->world.AddEntity(std::move(Barrier));
                        this->world.AddComponentToEntity(refBarrier.GetId(),std::unique_ptr<TransformComponent>(new TransformComponent(CurX, CurY, 0, 0)));
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenBarrier, renderer)));
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, {CurX + ancho/2, CurY + alto/2})));
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<BarrierComponent>(new BarrierComponent()));
                        BarrierCounter++;
                    }
                    CurX += OffX; CurY += OffY;
                }
            }
        }
    }
}

void Game::ProcessInput(float deltaTime)
{
    playerinputsystem->Update(world,deltaTime);
}

void Game::Update(float deltaTime)
{
    timersystem->Update(world,deltaTime);
    spawnsystem->Update(world,deltaTime);
    movementsystem->Update(world,deltaTime);
    collisionsystem->Update(world,deltaTime);
    damagesystem->Update(world,deltaTime);
    EventBus::Instance().Dispatch();
}

void Game::Render(float deltaTime)
{
    SDL_RenderClear(renderer);

    // Dibujar Fondo (Usando la textura pre-cargada)
    if (this->textureBackground) {
        SDL_FRect rectFondo{0, 0, 1920, 1080};
        SDL_RenderTexture(renderer, this->textureBackground, nullptr, &rectFondo);
    }

    // Dibujar Entidades
    rendersystem->Update(world, deltaTime);

    SDL_RenderPresent(renderer);
}