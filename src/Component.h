#pragma once
#include <string>
#include <tuple>
#include <thread> 
#include <atomic> 
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

// Clase base para todos los componentes del ECS
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

// Componente para manejar posición, velocidad y movimiento físico
class TransformComponent : public Component
{
    public:
        std::tuple<float,float> Position;
        std::tuple<float,float> Velocity;
        std::tuple<float,float> BaseVelocity;
        float BouncingTimer;
        TransformComponent(float x, float y, float vx, float vy);
};

// Componente visual que guarda la textura de la entidad
class SpriteComponent : public Component
{
    public:
        SDL_Texture* Texture;
        SpriteComponent(std::string path, SDL_Renderer* renderer);
};

// Componente para detectar colisiones (Caja delimitadora AABB)
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

// Componente de vida y cooldown para invencibilidad
class HealthComponent : public Component
{
    public:
        int MaxHp;
        int Hp;
        float Cooldown;
        HealthComponent(int HealthPoints);
};

// Enumeración para los tipos de estrategias de la IA
enum class EnemyRole {
    CHASER,   // Persecución directa
    FLANKER   // Flanqueo y rodeo
};

// Componente de Enemigo: Contiene la lógica de IA multihilo
class EnemyComponent : public Component
{
    public:
        // Rol táctico asignado
        EnemyRole role;
        
        // Control de concurrencia para el hilo independiente
        std::atomic<bool> threadActive;  
        std::thread aiThread;            
        
        // Variables atómicas para comunicar el objetivo al hilo principal
        std::atomic<float> targetX; 
        std::atomic<float> targetY;

        EnemyComponent(EnemyRole assignedRole = EnemyRole::CHASER);
        ~EnemyComponent(); 
};

// Etiquetas simples para identificar al Jugador y Barreras
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