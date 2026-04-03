#include "engine/core/serialalizing/file_manager.h"

vshade::file::FileManager::FileManager(std::string const& root_directory) : root_directory_{root_directory}
{
    //// Find all files with .vspack extension in the given directory
    //ankerl::unordered_dense::map<std::string, std::vector<std::string>> const files{findFilesWithExtension(root_directory, {".vspack"})};

    //// Open each file to build the path map
    //for (auto const& [ext, paths] : files)
    //{
    //    for (std::string const& path : paths)
    //    {
    //        if (File file = File(path, _IN_ | _SKIP_CHECK_SUM_ | _SKIP_BUFFER_USE_IN_, "@vspack", VERSION(0U, 0U, 1U)))
    //        {
    //            // Read the position of the file count
    //            std::uint32_t files_count_position{0U};
    //            file.read(files_count_position);
    //            file.setPosition(files_count_position);

    //            // Read the count of files packed in this file
    //            std::uint32_t files_count{0U};
    //            file.read(files_count);

    //            // Read each file path and position and store them in the path map
    //            for (std::uint32_t i{0U}; i < files_count; ++i)
    //            {
    //                std::string file_path;
    //                file.read(file_path);
    //                std::uint32_t position{0U};
    //                file.read(position);
    //                path_map_.emplace(std::piecewise_construct, std::forward_as_tuple(file_path), std::forward_as_tuple(path, position));
    //            }
    //        }
    //    }
    //}
}

vshade::file::File vshade::file::FileManager::loadFile(std::string const& file_path, magic_t const& magic, flag_t flags)
{
    std::string const file_path_with_root{root_directory_.string() + file_path};

    // Check if the file exists on disk
    if (std::filesystem::exists(file_path_with_root))
    {
        // If the file exists, create and return a File object with specified flags
        return File(file_path_with_root, _IN_ | flags, magic, VERSION(0U, 0U, 1U));
    }
    else
    {
        // Check if the file path is in the packed files map
        auto const packed = path_map_.find(file_path_with_root);

        // If the file path is found in the map
        if (packed != path_map_.end())
        {
            // Open the packed file for reading with appropriate flags
            if (File packed_file = File(packed->second.first, _IN_ | _SKIP_CHECK_SUM_ | _SKIP_BUFFER_USE_IN_, "@vspack", VERSION(0U, 0U, 1U)))
            {
                // Adjust the file handle position to locate the specific file in the packed file
                packed_file.getFileHandle()->seekp(packed->second.second + packed_file.getFileHandle()->tellg());
                // Return a new File object for the specific file within the packed file
                return File(packed_file.getFileHandle(), _IN_, magic, VERSION(0U, 0U, 1U));
            }
        }
    }

    // Return an invalid File object if the file cannot be loaded
    return File(); // Indicates failure to load the file
}

vshade::file::File vshade::file::FileManager::saveFile(std::string const& file_path, magic_t const& magic, flag_t flags)
{
    std::string const file_path_with_root{root_directory_.string() + file_path};

    return File(file_path_with_root, _OUT_ | flags, magic, VERSION(0U, 0U, 1U));
}

void vshade::file::FileManager::packFiles(PackSpecification const& specification)
{
}

ankerl::unordered_dense::map<std::string, std::vector<std::string>>
vshade::file::FileManager::findFilesWithExtension(std::filesystem::path const& directory, std::vector<std::string> const& extensions)
{
    // Result map to store files by extension
    ankerl::unordered_dense::map<std::string, std::vector<std::string>> result;

    // Iterate over each entry in the directory
    for (auto const& entry : std::filesystem::directory_iterator(directory))
    {
        // If the entry is a directory, recursively search its contents
        if (std::filesystem::is_directory(entry.status()))
        {
            ankerl::unordered_dense::map<std::string, std::vector<std::string>> subdirectory_files{findFilesWithExtension(entry.path(), extensions)};
            // Merge subdirectory results into the result map
            for (auto const& [extension, paths] : subdirectory_files)
            {
                result[extension].insert(result[extension].end(), paths.begin(), paths.end());
            }
        }
        // If the entry is a regular file, check its extension
        else if (std::filesystem::is_regular_file(entry.status()))
        {
            for (std::string const& extension : extensions)
            {
                if (entry.path().extension() == extension)
                {
                    // Add the file path to the result map
                    result[extension].push_back(entry.path().generic_string());
                }
            }
        }
    }

    return result;
}
ankerl::unordered_dense::map<std::string, std::vector<std::string>>
vshade::file::FileManager::findFilesWithExtensionExclude(std::filesystem::path const& directory, std::vector<std::string> const& exclude_extensions)
{
    ankerl::unordered_dense::map<std::string, std::vector<std::string>> result; // Result map to store files by extension

    // Iterate over each entry in the directory
    for (auto const& entry : std::filesystem::directory_iterator(directory))
    {
        // If the entry is a directory, recursively search its contents
        if (std::filesystem::is_directory(entry.status()))
        {
            ankerl::unordered_dense::map<std::string, std::vector<std::string>> subdirectory_files{
                findFilesWithExtensionExclude(entry.path(), exclude_extensions)};
            // Merge subdirectory results into the result map
            for (auto const& [ext, paths] : subdirectory_files)
            {
                result[ext].insert(result[ext].end(), paths.begin(), paths.end());
            }
        }
        // If the entry is a regular file, check its extension
        else if (std::filesystem::is_regular_file(entry.status()))
        {
            bool is_exclude_file{false};
            // Check if the file has one of the excluded extensions
            for (std::string const& exclude_extension : exclude_extensions)
            {
                if (entry.path().extension() == exclude_extension)
                {
                    is_exclude_file = true;
                    break; // Exit loop if file should be excluded
                }
            }
            if (!is_exclude_file)
            {
                // Add the file path to the result map
                result[entry.path().extension().string()].push_back(entry.path().generic_string());
            }
        }
    }

    return result; // Return the map of files found
}
