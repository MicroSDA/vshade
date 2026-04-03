#ifndef ENGINE_CORE_SERIALIZING_FILE_MANAGER
#define ENGINE_CORE_SERIALIZING_FILE_MANAGER

#include <ankerl/unordered_dense.h>
#include <engine/core/serialalizing/file.h>
#include <engine/core/utility/singleton.h>
#include <filesystem>
#include <iostream>
#include <vector>

namespace vshade
{
namespace file
{
class VSHADE_API FileManager final : public utility::CRTPSingleton<FileManager>
{
    friend class utility::CRTPSingleton<FileManager>;

public:
    struct PackSpecification
    {
        ankerl::unordered_dense::map<std::string, std::vector<std::string>> format_path;
        ankerl::unordered_dense::map<std::string, std::string>              format_packet_path;
    };

public:
    virtual ~FileManager()                       = default;
    FileManager(FileManager const&)              = delete;
    FileManager(FileManager&&)                   = delete;
    FileManager& operator=(FileManager const&) & = delete;
    FileManager& operator=(FileManager&&) &      = delete;

    File loadFile(std::string const& filePath, magic_t const& magic, flag_t flags = _NONE_);
    File saveFile(std::string const& filePath, magic_t const& magic, flag_t flags = _NONE_);

    void packFiles(PackSpecification const& specification);

    ankerl::unordered_dense::map<std::string, std::vector<std::string>> 
    findFilesWithExtension(std::filesystem::path const&    directory, std::vector<std::string> const& extensions);

    ankerl::unordered_dense::map<std::string, std::vector<std::string>>
    findFilesWithExtensionExclude(std::filesystem::path const& directory, std::vector<std::string> const& excludeExtensions);

    template<typename ...Args>
    std::ifstream getNativeFile(const std::string& file_path, Args&&... args)
    {
        return std::ifstream(root_directory_.string() + file_path, std::forward<Args>(args)...);
    }

    std::string getRootDirectory() const
    {
        return root_directory_.string();
    }
private:
    explicit FileManager(std::string const& root_directory);

    std::filesystem::path root_directory_;
    // TODO: need to refactor ?
    ankerl::unordered_dense::map<std::string, std::pair<std::string, std::uint32_t>> path_map_;
};

} // namespace file
} // namespace vshade

#endif // ENGINE_CORE_SERIALIZING_FILE_MANAGER