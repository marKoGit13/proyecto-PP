#include "Game.h"

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
    while (isRunning) //Esto se ejecuta cada vez por frame
    {
        // Setear el DeltaTime
        Uint64 now = SDL_GetTicks();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;
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
    float PosicionY = 1000.f;
    float VelocityX = 0.f;
    float VelocityY = 0.f;
    int HealthPoints = 3;
    
    // CREAR JUGADOR (Primero, para que ocupe espacio en el mundo)
    std::unique_ptr<Entity> Character = std::make_unique<Entity>("Player_" + std::to_string(0));
    Entity& refCharacter = *Character;
    this->world.AddEntity(std::move(Character));

    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<TransformComponent>(new TransformComponent(PosicionX, PosicionY, VelocityX, VelocityY)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenCharacter, renderer)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, std::pair<float,float>{PosicionX + ancho/2, PosicionY - alto/2}))); // Ajustar collider al centro si es necesario
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<HealthComponent>(new HealthComponent(HealthPoints)));
    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<PlayerComponent>(new PlayerComponent()));

    // GENERACIÓN PROCEDIMENTAL DE BARRERAS
    int BarrierCounter = 0;
    
    // Recorremos la pantalla en cuadrícula
    for (size_t j = 64; j < std::get<1>(this->AnchoAlto) - 64; j = j + 64)
    {
        for (size_t i = 64; i < std::get<0>(this->AnchoAlto); i = i + 64) 
        {
            // Probabilidad baja de iniciar una pared aquí
            float BarrierProbability = NumberRandomizer(true, 0.f, 100.f);
            if (BarrierProbability < 5.f) 
            {
                int direction = NumberRandomizer(false, 0, 3);
                int count = NumberRandomizer(false, 3, 6); // Longitud de la pared
                
                float CurrentX = i;
                float CurrentY = j;
                int OffsetX = 0;
                int OffsetY = 0;

                // Definir dirección de crecimiento
                switch (direction) 
                {
                    case 0: OffsetX = 64; break; // Derecha
                    case 1: OffsetY = 64; break; // Abajo
                    case 2: OffsetX = -64; break; // Izquierda
                    case 3: OffsetY = -64; break; // Arriba
                }

                // Generar los bloques de la pared
                for (int k = 0; k < count; k++) 
                {
                    // --- CORRECCIÓN CLAVE: Verificar si el espacio está libre ---
                    // Solo creamos la barrera si NO choca con el jugador u otra barrera
                    if (this->world.IsAreaFree(CurrentX, CurrentY, ancho, alto))
                    {
                        std::unique_ptr<Entity> Barrier = std::make_unique<Entity>("Barrier_" + std::to_string(BarrierCounter));
                        Entity& refBarrier = *Barrier;
                        this->world.AddEntity(std::move(Barrier));
                        
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<TransformComponent>(new TransformComponent(CurrentX, CurrentY, 0, 0)));
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenBarrier, renderer)));
                        
                        // Centrar el Collider correctamente respecto a la posición (asumiendo origen top-left)
                        float midX = CurrentX + ancho / 2.0f;
                        float midY = CurrentY + alto / 2.0f; 
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, std::pair<float,float>{midX, midY})));
                        
                        this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<BarrierComponent>(new BarrierComponent()));
                        
                        BarrierCounter++;
                    }
                    
                    // Mover al siguiente bloque (incluso si no se pudo poner el actual, intentamos el siguiente)
                    CurrentX += OffsetX;
                    CurrentY += OffsetY;
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
    // 1. Limpiar pantalla
    SDL_RenderClear(renderer);

    // 2. Dibujar Fondo (Usando la textura ya cargada)
    if (this->textureBackground) {
        SDL_FRect rectFondo{0, 0, 1920, 1080}; 
        SDL_RenderTexture(renderer, this->textureBackground, nullptr, &rectFondo);
    }

    // 3. Dibujar Entidades (Aquí incluimos la lógica de parpadeo dentro del sistema)
    rendersystem->Update(world, deltaTime);

    // 4. Mostrar
    SDL_RenderPresent(renderer);
}