#include "Event.h"

Event::Event()
{
    Type = "";
}

// Acá solo se les asigna el tipo, ya que el handler se encarga el método Suscribe
DamageEvent::DamageEvent()
{
    Type = "DamageEvent";
}

SpawnEvent::SpawnEvent()
{
    Type = "SpawnEvent";
}

BounceBarrierEvent::BounceBarrierEvent(Entity& entitytobounce)
    : EntityToBounce(entitytobounce)
{
    Type = "BounceBarrierEvent";
}