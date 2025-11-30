#include "Component.h"

int Component::cantidad = 0;

// Inicialización base de componentes con ID único
Component::Component()
{
    Type = "";
    Id = "Component_" + std::to_string(cantidad);
    cantidad++;
}

Component::~Component() {
}

// Inicializa posición y velocidad
TransformComponent::TransformComponent(float x, float y, float vx, float vy)
{
    Type = "Transform";
    Position = {x, y};
    Velocity = {vx, vy};
    BouncingTimer = 0.f;
    BaseVelocity = Velocity;
}

// Carga la imagen desde disco y crea la textura GPU
SpriteComponent::SpriteComponent(std::string path, SDL_Renderer* renderer)
{
    Type = "Sprite";
    const char* ruta = path.c_str();
    SDL_Surface* surface = IMG_Load(ruta);
    if (surface) {
        Texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }
    else {
        Texture = nullptr; 
    }
}

// Configura las dimensiones y centro de la caja de colisión
ColliderComponent::ColliderComponent(float width, float height, std::tuple<float,float> midPoint)
{
    Type = "Collider";
    Bounds = {width, height};
    MidPoint = midPoint;
}

// Métodos auxiliares para obtener bordes del collider
float ColliderComponent::getRight()
{
    return std::get<0>(MidPoint) + std::get<0>(Bounds)/2.0f;
}
float ColliderComponent::getLeft()
{
    return std::get<0>(MidPoint) - std::get<0>(Bounds)/2.0f;
}
float ColliderComponent::getTop()
{
    return std::get<1>(MidPoint) - std::get<1>(Bounds)/2.0f;
}
float ColliderComponent::getBottom()
{
    return std::get<1>(MidPoint) + std::get<1>(Bounds)/2.0f;
}

// Lógica simple de intersección de rectángulos
bool ColliderComponent::Collision(ColliderComponent* collider){
    bool a = this->getLeft() > collider->getRight();
    bool b = this->getRight() < collider->getLeft();
    bool c = this->getTop() > collider->getBottom();
    bool d = this->getBottom() < collider->getTop();
    return !(a || b || c || d);
}

// Inicializa vida máxima y actual
HealthComponent::HealthComponent(int HealthPoints)
{
    Type = "Health";
    MaxHp = HealthPoints;
    Hp = MaxHp;
    Cooldown = 0.f;
}

// Inicializa el componente enemigo y sus variables atómicas
EnemyComponent::EnemyComponent(EnemyRole assignedRole)
{
    Type = "Enemy";
    role = assignedRole;
    threadActive = true; 
    targetX = 0.0f;
    targetY = 0.0f;
}

// Destructor importante: asegura que el hilo de IA se cierre limpiamente
EnemyComponent::~EnemyComponent()
{
    threadActive = false; 
    if(aiThread.joinable()) {
        aiThread.join();
    }
}

// Constructores de etiquetas simples
PlayerComponent::PlayerComponent()
{
    Type = "Player";
}

BarrierComponent::BarrierComponent()
{
    Type = "Barrier";
}