#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <tuple>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <spdlog/spdlog.h>
#include "Entity.h"
#include "Component.h"
#include "ISystem.h"
#include "World.h"
#include "SupportFuncs.h" 
#include "Event.h"
#include "EventBus.h"

class Game
{
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        bool isRunning;
        
        // --- CORRECCIÓN: Variable para el fondo ---
        SDL_Texture* textureBackground = nullptr; 
        // ----------------------------------------

        std::tuple<int,int> AnchoAlto;
        int cantidad;
        std::string rutaImagenCharacter;
        std::string rutaImagenBackground;
        std::string rutaImagenEnemy;
        std::string rutaImagenBarrier;
        float intervalo;

        std::unique_ptr<PlayerInputSystem> playerinputsystem;
        std::unique_ptr<MovementSystem> movementsystem;
        std::unique_ptr<RenderSystem> rendersystem;
        std::unique_ptr<CollisionSystem> collisionsystem;
        std::unique_ptr<DamageSystem> damagesystem;
        std::unique_ptr<SpawnSystem> spawnsystem;
        std::unique_ptr<TimerSystem> timersystem;

        World world;

    public:
        Game();
        ~Game() = default;
        void Initialize();
        void Run();
        void Start();
        void ProcessInput(float deltaTime);
        void Update(float deltaTime);
        void Render(float deltaTime);
};