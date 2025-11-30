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

void Game::Initialize()
{
    //Creación de pantalla
    this->window = SDL_CreateWindow(
        "Space Invaders?", 
        std::get<0>(this->AnchoAlto),
        std::get<1>(this->AnchoAlto),
        SDL_WINDOW_FULLSCREEN
    );
    this->renderer =  SDL_CreateRenderer(window, nullptr);

    //Control y logeo de errores
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        spdlog::error("Error inicializando SDL");
    }
    if (window == nullptr)
    {
        spdlog::error("Error creando ventana SDL");
    }
    if (renderer == nullptr)
    {
        spdlog::error("Error obteniendo renderer SDL");
    }

    SDL_Surface* surfacefondo = IMG_Load(rutaImagenBackground.c_str());
    if (surfacefondo) {
        this->textureBackground = SDL_CreateTextureFromSurface(this->renderer, surfacefondo);
        SDL_DestroySurface(surfacefondo);
    } else {
        spdlog::error("No se pudo cargar el fondo");
    }

    //Inicialización de sistemas
    this->movementsystem = std::make_unique<MovementSystem>();
    this->playerinputsystem = std::make_unique<PlayerInputSystem>(this->isRunning);
    this->rendersystem = std::make_unique<RenderSystem>(this->renderer,std::get<0>(this->AnchoAlto),std::get<1>(this->AnchoAlto));
    this->collisionsystem = std::make_unique<CollisionSystem>(std::get<0>(this->AnchoAlto),std::get<1>(this->AnchoAlto));
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
    // Lectura del Json y agarre de sus datos
    std::tuple<int, std::string, std::string, std::string, std::string, float, float, float> Information = ReadFromConfigFile("./assets/data.json");
    float ancho = std::get<5>(Information);
    float alto = std::get<6>(Information);
    
    // Configuración inicial del Jugador
    float PosicionX = 960.f;
    float PosicionY = 850.f; //antes 1000
    float VelocityX = 0.f;
    float VelocityY = 0.f;
    int HealthPoints = 3;
    
    // CREAR JUGADOR (Primero, para que ocupe espacio en el mundo)
    std::unique_ptr<Entity> Character = std::make_unique<Entity>("Player_0");
    Entity& refCharacter = *Character;
    this->world.AddEntity(std::move(Character));

    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<TransformComponent>(new TransformComponent(PosicionX, PosicionY, 0.f, 0.f)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenCharacter, renderer)));
    // Centrar collider
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, {PosicionX + ancho/2, PosicionY + alto/2})));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<HealthComponent>(new HealthComponent(3)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<PlayerComponent>(new PlayerComponent()));

    // GENERACIÓN PROCEDIMENTAL DE BARRERAS
    int BarrierCounter = 0;
    for (size_t j = 64; j < std::get<1>(this->AnchoAlto) - 64; j = j + 64) {
        for (size_t i = 64; i < std::get<0>(this->AnchoAlto); i = i + 64) {
            float BarrierProbability = NumberRandomizer(true, 0.f, 100.f);
            if (BarrierProbability < 5.f) {
                int direction = NumberRandomizer(false, 0, 3);
                int count = NumberRandomizer(false, 3, 6);
                float CurX = i; float CurY = j;
                int OffX = 0, OffY = 0;
                switch (direction) { case 0: OffX = 64; break; case 1: OffY = 64; break; case 2: OffX = -64; break; case 3: OffY = -64; break; }

                for (int k = 0; k < count; k++) {
                    // Usar world.IsAreaFree
                    if (this->world.IsAreaFree(CurX, CurY, ancho, alto)) {
                        std::unique_ptr<Entity> Barrier = std::make_unique<Entity>("Barrier_" + std::to_string(BarrierCounter));
                        Entity& refBarrier = *Barrier;
                        this->world.AddEntity(std::move(Barrier));
                        
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<TransformComponent>(new TransformComponent(CurX, CurY, 0, 0)));
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