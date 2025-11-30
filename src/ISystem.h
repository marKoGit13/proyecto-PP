#pragma once
#include <string>
#include <tuple>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <json.hpp>
#include <fstream>
#include <random>
#include <spdlog/spdlog.h>
#include "World.h"
#include "SupportFuncs.h"
#include "Event.h"
#include "EventBus.h"

// Interfaz base para todos los sistemas
class ISystem
{   
    public:
        static bool TooBad; // Estado global de Game Over
        ISystem();
        virtual void Update(World&, float dt) = 0;
        virtual ~ISystem();
};

// Sistema de entrada del jugador
class PlayerInputSystem : public ISystem 
{
    public:
        bool& isRunning;
        PlayerInputSystem(bool& isrunning);
        void Update(World& world, float dt) override;
};

// Sistema de física y movimiento (incluye Flocking)
class MovementSystem : public ISystem 
{
    public:
        MovementSystem();
        void Update(World& world, float dt) override;
};

// Sistema de renderizado visual y UI
class RenderSystem : public ISystem 
{
    public:
        SDL_Renderer* Renderer;
        float WindowWidth;
        float WindowHeight;
        TTF_Font* GameFont; 

        RenderSystem(SDL_Renderer* renderer, float windowwidth, float windowheight, TTF_Font* font);
        void Update(World& world, float dt) override;
};

// Sistema de resolución de colisiones
class CollisionSystem : public ISystem 
{
    public:
        int WindowWidth;
        int WindowHeight;
        CollisionSystem(int windowwidth, int windowheight);
        void Update(World& world, float dt) override;
};

// Sistema de gestión de daño e invencibilidad
class DamageSystem : public ISystem 
{
    public:
        bool pendingDamage = false;
        DamageSystem();
        void Update(World& world, float dt) override;
};

// Sistema de aparición de enemigos
class SpawnSystem : public ISystem 
{
    public:
        SDL_Renderer* Renderer;
        bool pendingSpawn = false;
        SpawnSystem(SDL_Renderer* renderer);
        void Update(World& world, float dt) override;
};

// Sistema de temporizadores globales
class TimerSystem : public ISystem 
{
    private:
        static float totaltimer;
        static float spawntimer;
    public:
        float Interval;
        TimerSystem(float interval);
        void Update(World& world, float dt) override;
};