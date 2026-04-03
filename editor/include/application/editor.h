#include <engine/core/utility/utils.h>
#include <entry_point.h>
#include <layer/main_layer.h>
#include <renderer/renderer.h>
namespace editor
{
class Application : public vshade::Application
{
public:
    Application(std::string const& root_directory_);
    virtual ~Application() = default;

    virtual void onCreate() override;
    virtual void onDestroy() override;
    virtual void onUpdate() override;
    virtual bool onEvent(std::shared_ptr<vshade::event::Event> const event) override;

private:
};
} // namespace editor