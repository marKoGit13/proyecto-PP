#include "Game.h"

Game::Game() : world()
{
    window = nullptr;
    renderer = nullptr;
    textureBackground = nullptr;
    globalFont = nullptr;

    // Carga de configuración desde JSON
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

// Limpieza de recursos al cerrar
Game::~Game() {
    if(globalFont) TTF_CloseFont(globalFont);
    TTF_Quit();
    SDL_Quit();
}

// Configuración inicial de SDL y subsistemas
void Game::Initialize()
{
    this->window = SDL_CreateWindow("Space Invaders WolfPack", std::get<0>(this->AnchoAlto), std::get<1>(this->AnchoAlto), SDL_WINDOW_FULLSCREEN);
    this->renderer =  SDL_CreateRenderer(window, nullptr);

    if (!SDL_Init(SDL_INIT_VIDEO)) spdlog::error("Error SDL Init");
    
    // Inicializar librería de fuentes
    if (!TTF_Init()) {
        spdlog::error("Error TTF Init: {}", SDL_GetError());
    }
    
    // Cargar fuente principal
    this->globalFont = TTF_OpenFont("./assets/arial.ttf", 24); 
    if (!this->globalFont) {
        spdlog::error("Error cargando fuente: {}", SDL_GetError());
    }

    // Precarga del fondo para optimizar rendimiento
    SDL_Surface* surfacefondo = IMG_Load(rutaImagenBackground.c_str());
    if (surfacefondo) {
        this->textureBackground = SDL_CreateTextureFromSurface(this->renderer, surfacefondo);
        SDL_DestroySurface(surfacefondo);
    } else {
        spdlog::error("No se pudo cargar el fondo: {}", rutaImagenBackground);
    }

    // Instanciación de sistemas ECS
    this->movementsystem = std::make_unique<MovementSystem>();
    this->playerinputsystem = std::make_unique<PlayerInputSystem>(this->isRunning);
    this->rendersystem = std::make_unique<RenderSystem>(this->renderer, std::get<0>(this->AnchoAlto), std::get<1>(this->AnchoAlto), this->globalFont);
    this->collisionsystem = std::make_unique<CollisionSystem>(std::get<0>(this->AnchoAlto), std::get<1>(this->AnchoAlto));
    this->damagesystem = std::make_unique<DamageSystem>();
    this->spawnsystem = std::make_unique<SpawnSystem>(this->renderer);
    this->timersystem = std::make_unique<TimerSystem>(this->intervalo);
}

// Bucle principal del juego
void Game::Run()
{
    Start();
    Uint64 lastFrame = SDL_GetTicks();
    this->isRunning = true;
    while (isRunning)
    {
        Uint64 now = SDL_GetTicks();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;
        ProcessInput(deltaTime);
        Update(deltaTime);
        Render(deltaTime);
    }
}

// Configuración del nivel inicial
void Game::Start()
{
    std::tuple<int, std::string, std::string, std::string, std::string, float, float, float> Information = ReadFromConfigFile("./assets/data.json");
    // Escala global (60%)
    float scale = 0.6f;
    float ancho = std::get<5>(Information) * scale;
    float alto = std::get<6>(Information) * scale;

    // Nave centrada y abajo
    float PosicionX = (1920.0f / 2.0f) - (ancho / 2.0f);
    float PosicionY = 1080.0f - 150.0f - alto;

    // LISTA DE LOS 10 SPRITES PARA BARRERAS
    // Definimos esto aquí para no crearlo mil veces dentro del bucle
    std::vector<std::string> barrierSprites = {
        rutaImagenBarrier,              
        "./assets/BarrierSprite2.png",  
        "./assets/BarrierSprite3.png",  
        "./assets/BarrierSprite4.png",  
        "./assets/BarrierSprite5.png",  
        "./assets/BarrierSprite6.png",  
        "./assets/BarrierSprite7.png",  
        "./assets/BarrierSprite8.png",  
        "./assets/BarrierSprite9.png",  
        "./assets/BarrierSprite10.png",
        "./assets/BarrierSprite11.png"  
    };

    // Creación del jugador
    std::unique_ptr<Entity> Character = std::make_unique<Entity>("Player_0");
    Entity& refCharacter = *Character;
    this->world.AddEntity(std::move(Character));

    this->world.AddComponentToEntity(refCharacter.GetId(),std::unique_ptr<TransformComponent>(new TransformComponent(PosicionX, PosicionY, 0.f, 0.f)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenCharacter, renderer)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, {PosicionX + ancho/2, PosicionY + alto/2})));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<HealthComponent>(new HealthComponent(5))); // HP = 5
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<PlayerComponent>(new PlayerComponent()));

    // Generación procedimental de barreras
    int BarrierCounter = 0;
    int Margin = 100;
    for (size_t j = Margin; j < 1080 - Margin; j = j + 64)
    {
        for (size_t i = Margin; i < 1920 - Margin; i = i + 64) 
        {
            float BarrierProbability = NumberRandomizer(true, 0.f, 100.f);
            if (BarrierProbability < 5.f) 
            {
                int direction = NumberRandomizer(false, 0, 3);
                int count = NumberRandomizer(false, 3, 6);
                float CurX = i; float CurY = j;
                int OffX = 0, OffY = 0;
                
                // Ajustar offset según el nuevo tamaño escalado 
                // Multiplicamos por 0.75f para que se superpongan un 25% 
                int stepW = (int)(ancho * 0.75f); 
                int stepH = (int)(alto * 0.75f); 

                switch (direction) { 
                    case 0: OffX = stepW; break; 
                    case 1: OffY = stepH; break; 
                    case 2: OffX = -stepW; break; 
                    case 3: OffY = -stepH; break; 
                }
                for (int k = 0; k < count; k++) 
                {
                    if (CurX > Margin && CurX < 1920 - Margin && CurY > Margin && CurY < 1080 - Margin) {
                        if (this->world.IsAreaFree(CurX, CurY, ancho, alto)) {
                            std::unique_ptr<Entity> Barrier = std::make_unique<Entity>("Barrier_" + std::to_string(BarrierCounter));
                            Entity& refBarrier = *Barrier;
                            this->world.AddEntity(std::move(Barrier));
                            
                            this->world.AddComponentToEntity(refBarrier.GetId(),std::unique_ptr<TransformComponent>(new TransformComponent(CurX, CurY, 0, 0)));

                            // SELECCIÓN ALEATORIA DE SPRITE (1 de 11) 
                            int rndIdx = NumberRandomizer(false, 0, 10);  
                            std::string selectedSprite = barrierSprites[rndIdx];
                            
                            this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(selectedSprite, renderer)));
                            // -----------------------------------------------

                            this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, {CurX + ancho/2, CurY + alto/2})));
                            this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<BarrierComponent>(new BarrierComponent()));
                            BarrierCounter++;
                        }
                    }
                    CurX += OffX; CurY += OffY;
                }
            }
        }
    }
}

void Game::ProcessInput(float deltaTime)
{
    playerinputsystem->Update(world, deltaTime);
}

void Game::Update(float deltaTime)
{
    timersystem->Update(world, deltaTime);
    spawnsystem->Update(world, deltaTime);
    movementsystem->Update(world, deltaTime);
    collisionsystem->Update(world, deltaTime);
    damagesystem->Update(world, deltaTime);
    EventBus::Instance().Dispatch();
}

void Game::Render(float deltaTime)
{
    SDL_RenderClear(renderer);
    if (textureBackground) {
        SDL_FRect rectFondo{0, 0, 1920, 1080};
        SDL_RenderTexture(renderer, textureBackground, nullptr, &rectFondo);
    }
    rendersystem->Update(world, deltaTime);
    SDL_RenderPresent(renderer);
}