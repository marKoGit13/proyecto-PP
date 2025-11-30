#pragma once
#include "Entity.h"
#include <string>

class Event
{
    public:
        std::string Type;
        Event();
};
class DamageEvent : public Event
{
    public:
        DamageEvent();
};
class SpawnEvent : public Event
{
    public:
        SpawnEvent();
};
class BounceBarrierEvent : public Event
{
    public:
        Entity& EntityToBounce;
        BounceBarrierEvent(Entity&);
};