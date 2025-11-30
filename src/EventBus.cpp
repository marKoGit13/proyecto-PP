#include "EventBus.h"

EventBus::EventBus()
{
}

EventBus& EventBus::Instance()
{
    static EventBus bus;
    return bus;
}

void EventBus::Enqueue(std::unique_ptr<Event> e)
{
    Event_Queue.push(std::move(e));
}

void EventBus::Suscribe(std::string eventtype, std::function<void(Event*)> handler)
{
    Event_Handlers.emplace(eventtype, handler);
}

void EventBus::Dispatch()
{
    while (!Event_Queue.empty())
    {
        std::unique_ptr<Event>& e = Event_Queue.front();
        auto handlerEjecutar = Event_Handlers[e->Type];
        handlerEjecutar(e.get());
        Event_Queue.pop();
    }
}

std::queue<std::unique_ptr<Event>>& EventBus::GetQueue() {
    return Event_Queue;
}