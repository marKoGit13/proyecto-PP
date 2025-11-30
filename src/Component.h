#pragma once
#include <string>
#include <tuple>
#include <thread> // IMPORTANTE: Para multithreading
#include <atomic> // IMPORTANTE: Para variables seguras entre hilos
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Component
{   
    protected:
        static int cantidad;
    public:
        std::string Id;
        std::string Type;
        
        Component();
        virtual ~Component();
};

class TransformComponent : public Component
{
    public:
        std::tuple<float,float> Position;
        std::tuple<float,float> Velocity;
        std::tuple<float,float> BaseVelocity;
        float BouncingTimer;
        TransformComponent(float x, float y, float vx, float vy);
};

class SpriteComponent : public Component
{
    public:
        SDL_Texture* Texture;
        SpriteComponent(std::string path, SDL_Renderer* renderer);
};

class ColliderComponent : public Component
{
    public:
        float getRight();
        float getLeft();
        float getTop();
        float getBottom();
        std::tuple<float,float> Bounds;
        std::tuple<float,float> MidPoint;
        ColliderComponent(float width, float height, std::tuple<float,float> midPoint);
        bool Collision(ColliderComponent* collider);    
};

class HealthComponent : public Component
{
    public:
        int MaxHp;
        int Hp;
        float Cooldown;
        HealthComponent(int HealthPoints);
};

// --- NUEVO: Definición de Roles ---
enum class EnemyRole {
    CHASER,   // El agresivo (va directo)
    FLANKER   // El táctico (rodea)
};

class EnemyComponent : public Component
{
    public:
        // Datos de IA
        EnemyRole role;
        
        // Datos de Hilos (Thread Safety)
        std::atomic<bool> threadActive;  // Bandera para detener el hilo
        std::thread aiThread;            // El objeto hilo
        
        // Comunicación entre Hilo IA y Main Thread
        // Usamos atomic para que el hilo escriba y el juego lea sin chocar
        std::atomic<float> targetX; 
        std::atomic<float> targetY;

        EnemyComponent(EnemyRole assignedRole = EnemyRole::CHASER);
        ~EnemyComponent(); // Importante: Destructor para cerrar el hilo
};

class PlayerComponent : public Component
{
    public:
        PlayerComponent();
};

class BarrierComponent : public Component
{
    public:
        BarrierComponent();
};