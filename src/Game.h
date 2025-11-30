#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <random>
#include <spdlog/spdlog.h>
#include "World.h"
#include <json.hpp>
#include <fstream>
#include <cmath>
#include "SupportFuncs.h"
#include "ISystem.h"

class Game{
    private:
        SDL_Window* window;
        SDL_Renderer* renderer; 

        std::unique_ptr<MovementSystem> movementsystem;
        std::unique_ptr<PlayerInputSystem> playerinputsystem;
        std::unique_ptr<RenderSystem> rendersystem;
        std::unique_ptr<CollisionSystem> collisionsystem;
        std::unique_ptr<DamageSystem> damagesystem;
        std::unique_ptr<SpawnSystem> spawnsystem;
        std::unique_ptr<TimerSystem> timersystem;

        int cantidad;
        std::string rutaImagenCharacter;
        std::string rutaImagenBackground;
        std::string rutaImagenEnemy;
        std::string rutaImagenBarrier;
        float intervalo;
        std::tuple<float,float> AnchoAlto;
        
        bool isRunning;

        World world;
        void Start();
        void ProcessInput(float deltaTime);
        void Update(float deltaTime);
        void Render(float deltaTime);
    public:
        Game();
        void Initialize();
        void Run();
};
