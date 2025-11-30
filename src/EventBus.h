#pragma once
#include <queue>
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include "Event.h"

class EventBus {
    private:
        std::queue<std::unique_ptr<Event>> Event_Queue;
        std::unordered_map<std::string, std::function<void(Event*)>> Event_Handlers;

        EventBus();

    public:
        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;
        static EventBus& Instance();

        void Enqueue(std::unique_ptr<Event> e);
        void Suscribe(std::string eventtype, std::function<void(Event*)> handler);
        void Dispatch();
        std::queue<std::unique_ptr<Event>>& GetQueue();
};