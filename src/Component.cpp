#include "Component.h"

int Component::cantidad = 0;

Component::Component()
{
    Type = "";
    Id = "Component_" + std::to_string(cantidad);
    cantidad++;
}

Component::~Component() {
}

TransformComponent::TransformComponent(float x, float y, float vx, float vy)
{
    Type = "Transform";
    Position = {x, y};
    Velocity = {vx, vy};
    BouncingTimer = 0.f;
    BaseVelocity = Velocity;
}

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
        Texture = nullptr; // Seguridad si falla la carga
    }
}

ColliderComponent::ColliderComponent(float width, float height, std::tuple<float,float> midPoint)
{
    Type = "Collider";
    Bounds = {width, height};
    MidPoint = midPoint;
}

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
bool ColliderComponent::Collision(ColliderComponent* collider){
    bool a = this->getLeft() > collider->getRight();
    bool b = this->getRight() < collider->getLeft();
    bool c = this->getTop() > collider->getBottom();
    bool d = this->getBottom() < collider->getTop();
    return !(a || b || c || d);
}

HealthComponent::HealthComponent(int HealthPoints)
{
    Type = "Health";
    MaxHp = HealthPoints;
    Hp = MaxHp;
    Cooldown = 0.f;
}

// --- ESTA ES LA PARTE QUE TE FALTABA ACTUALIZAR ---
EnemyComponent::EnemyComponent(EnemyRole assignedRole)
{
    Type = "Enemy";
    role = assignedRole;
    
    // Inicializar valores atómicos
    threadActive = true; 
    targetX = 0.0f;
    targetY = 0.0f;
}

EnemyComponent::~EnemyComponent()
{
    // Al destruir el componente, aseguramos cerrar el hilo
    threadActive = false; 
    if(aiThread.joinable()) {
        aiThread.join();
    }
}
// --------------------------------------------------

PlayerComponent::PlayerComponent()
{
    Type = "Player";
}

BarrierComponent::BarrierComponent()
{
    Type = "Barrier";
}