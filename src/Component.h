#pragma once
#include <string>
#include <tuple>
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

// Al Health Component se le agrega un MaxHp = Vida Total, Hp = Vida Actual y un Cooldown que marca cuanto falta para que pueda recibir la siguiente instancia de daño
class HealthComponent : public Component
{
    public:
        int MaxHp;
        int Hp;
        float Cooldown;
        HealthComponent(int HealthPoints);
};
// Al Enemy Component realmente no se le agrega nada, puesto que solo es un identificador
class EnemyComponent : public Component
{
    public:
        EnemyComponent();
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