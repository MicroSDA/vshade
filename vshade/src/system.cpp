#include "engine/config/system.h"

void vshade::System::setConfiguration(Configuration const& config)
{
    if (!is_set_)
    {
        configuration_ = config;
        is_set_        = true;
    }
    else
    {
        assert(is_set_ && "System configuration can be set once !");
    }
}
