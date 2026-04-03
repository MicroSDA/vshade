#ifndef VSHADE_INCLUDE_ENTRY_POINT_H
#define VSHADE_INCLUDE_ENTRY_POINT_H

#include <engine/core/application/application.h>
#include <filesystem>
namespace vshade
{
extern void createApplication(std::string const& root_directory);
extern void destroyApplication();
} // namespace vshade

int main(int argc, char* argv[])
{
     vshade::System::create<vshade::System>();
     std::string root_path{std::string(argv[0]) + ".runfiles/_main"};
     
     vshade::createApplication(std::filesystem::canonical(root_path).string());
     vshade::Application::instance().initialize();
     vshade::Application::instance().launch();
     vshade::Application::instance().terminate();

     vshade::destroyApplication();

     return 0;
}

#endif VSHADE_INCLUDE_ENTRY_POINT_H