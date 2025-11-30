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
    //Lectura del Json y agarre de sus datos
    std::tuple<int, std::string, std::string, std::string, std::string, float, float, float> Information = ReadFromConfigFile("./assets/data.json");
    float ancho = std::get<5>(Information);
    float alto= std::get<6>(Information);
    float PosicionX = 960.f;
    float PosicionY = 1000.f;
    float VelocityX = 0.f;
    float VelocityY = 0.f;
    int HealthPoints = 3;
    
    std::unique_ptr<Entity> Character = std::make_unique<Entity>("Player_" + std::to_string(0));
    Entity& refCharacter = *Character;
    this->world.AddEntity(std::move(Character));

    this->world.AddComponentToEntity(refCharacter.GetId(),std::unique_ptr<TransformComponent>(new TransformComponent(PosicionX, PosicionY, VelocityX, VelocityY)));

    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenCharacter, renderer)));

    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, std::pair<float,float>{PosicionX + ancho/2, PosicionY - alto/2})));

    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<HealthComponent>(new HealthComponent(HealthPoints)));

    this->world.AddComponentToEntity(refCharacter.GetId(), std::unique_ptr<PlayerComponent>(new PlayerComponent()));

    int BarrierCounter = 0;
    #warning Revisar el collssion system después de esto
    for (size_t j = 64; j < std::get<1>(this->AnchoAlto) - 64; j = j + 64)
    {
        for (size_t i = 64; i < std::get<0>(this->AnchoAlto); i = i + 64) 
        {
            float BarrierProbability = NumberRandomizer(true, 0.f, 100.f);
            if (BarrierProbability < 5.f) 
            {
                int direction = NumberRandomizer(false, 0, 3);
                int count = NumberRandomizer(false, 3, 6);
                float PositionX = i;
                float PositionY = j;
                int OffsetX = 0;
                int OffsetY = 0;
                switch (direction) 
                    {
                        case 0:
                            OffsetX = 64;
                            break;
                        case 1:
                            OffsetY = 64;
                            break;
                        case 2:
                            OffsetX = -64;
                            break;
                        case 3:
                            OffsetY = -64;
                            break;
                    }
                for (int k = 0; k < count; k++) 
                {
                    std::unique_ptr<Entity> Barrier = std::make_unique<Entity>("Barrier_" + std::to_string(BarrierCounter));
                    Entity& refBarrier = *Barrier;
                    this->world.AddEntity(std::move(Barrier));
                    
                    this->world.AddComponentToEntity(refBarrier.GetId(),std::unique_ptr<TransformComponent>(new TransformComponent(PositionX, PositionY, 0, 0)));

                    this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<SpriteComponent>(new SpriteComponent(rutaImagenBarrier, renderer)));

                    this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<ColliderComponent>(new ColliderComponent(ancho, alto, std::pair<float,float>{PositionX + ancho/2, PositionY - alto/2})));

                    this->world.AddComponentToEntity(refBarrier.GetId(), std::unique_ptr<BarrierComponent>(new BarrierComponent()));
                    PositionX = PositionX + OffsetX;
                    PositionY = PositionY + OffsetY;
                    BarrierCounter++;
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
    //Renderer
    //Contenedores de los objetos
    //SDL_FRect {nombre del contenedor}{Ubicación en X, Ubicación en Y, Largo, Alto};
    SDL_FRect rectFondo{0, 0, 1920, 1080}; // Cambiar a relación de cuadrado
    
    SDL_Surface* surfacefondo = IMG_Load(rutaImagenBackground.c_str());
    //Luego crea una textura con ese surface.
    //SDL_Texture* {nombre de la textura} = SDL_CreateTextureFromSurface(renderer, {nombre del surface});
    SDL_Texture* texturebackground = SDL_CreateTextureFromSurface(renderer, surfacefondo);
    //Elimina el surface porque ya no es necesario
    //SDL_DestroySurface({nombre del surface});
    SDL_DestroySurface(surfacefondo);

    //Placement de objetos en el respectivo contenedor.
    //SDL_RenderTexture(renderer, {nombre de la textura}, nullptr, &{nombre del contenedor});
    SDL_RenderTexture(renderer, texturebackground, nullptr, &rectFondo);
    
    // En el futuro se dibujarán las entidades desde world aquí
    rendersystem->Update(world,deltaTime);

    // Renderiza todo lo que has dibujado
    SDL_RenderPresent(renderer);
}