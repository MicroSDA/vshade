#include "engine/core/events/event_manager.h"


void vshade::event::EventManager::create()
{
    CRTPSingleton::create<EventManager>();
}

void vshade::event::EventManager::destroy()
{
    CRTPSingleton::destroy();
}

void vshade::event::EventManager::processEvents()
{
    std::scoped_lock<std::mutex> lock{events_queue_mutex_};
    
    while(events_pool_.size())
    {
        events_pool_.front()();
        events_pool_.pop();
    }
}