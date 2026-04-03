#ifndef ENGINE_CORE_SERIALIZING_FILE_H
#define ENGINE_CORE_SERIALIZING_FILE_H

#include <cstdint>
#include <engine/config/vshade_api.h>
#include <engine/core/logs/loger.h>
#include <engine/core/serialalizing/serializer.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace vshade
{
namespace file
{

using version_t = std::uint16_t;
using flag_t    = std::ios_base::openmode;
using magic_t   = std::string;

#if defined(_WIN32)
static constexpr inline flag_t _NONE_               = static_cast<flag_t>(0);
static constexpr inline flag_t _IN_                 = static_cast<flag_t>(1 << 0);
static constexpr inline flag_t _OUT_                = static_cast<flag_t>(1 << 1);
static constexpr inline flag_t _SKIP_MAGIC_         = static_cast<flag_t>(1 << 2);
static constexpr inline flag_t _SKIP_VERSION_       = static_cast<flag_t>(1 << 3);
static constexpr inline flag_t _SKIP_CHECK_SUM_     = static_cast<flag_t>(1 << 4);
static constexpr inline flag_t _SKIP_BUFFER_USE_IN_ = static_cast<flag_t>(1 << 5);
static constexpr inline flag_t _SUM_SRC_32_         = static_cast<flag_t>(1 << 6);
#elif defined(__linux__)
static constexpr inline flag_t _NONE_               = static_cast<flag_t>(0);
static constexpr inline flag_t _SKIP_MAGIC_         = static_cast<flag_t>(1 << 0);
static constexpr inline flag_t _SKIP_VERSION_       = static_cast<flag_t>(1 << 1);
static constexpr inline flag_t _SUM_SRC_32_         = static_cast<flag_t>(1 << 2);
static constexpr inline flag_t _IN_                 = static_cast<flag_t>(1 << 3);
static constexpr inline flag_t _OUT_                = static_cast<flag_t>(1 << 4);
static constexpr inline flag_t _SKIP_CHECK_SUM_     = static_cast<flag_t>(1 << 5);
static constexpr inline flag_t _SKIP_BUFFER_USE_IN_ = static_cast<flag_t>(1 << 6);
#endif //

static version_t VERSION(version_t major, version_t minor, version_t patch)
{
    return (static_cast<version_t>(major) << 10U) | (static_cast<version_t>(minor) << 4U) | static_cast<version_t>(patch);
}

class VSHADE_API File final
{
public:
    using checksum_t     = std::uint32_t;
    using content_size_t = std::uint32_t;

    struct Header
    {
        magic_t        magic;
        version_t      version{0U};
        content_size_t content_size{0U};
        checksum_t     check_sum{0U};
    };

    explicit File();
    explicit File(std::string const& file_path, flag_t flags, magic_t const magic = "@", version_t version = version_t{0U});
    explicit File(std::shared_ptr<std::fstream> stream, flag_t flags, magic_t const magic = "@", version_t version = version_t{0U});
    ~File();

    bool openFile(std::string const& file_path, flag_t flags, magic_t const magic = "@", version_t version = version_t{0U});
    bool openFile(std::shared_ptr<std::fstream> stream, flag_t flags, magic_t const magic = "@", version_t version = version_t{0U});
    void closeFile();

    std::size_t getSize();

    bool IsOpen() const
    {
        return file_handle_->is_open();
    }

    void setPosition(std::size_t pos)
    {
        // If buffer use is skipped and file is open for input
        (flags_ & _SKIP_BUFFER_USE_IN_ && flags_ & _IN_)
            ? file_handle_->seekp(pos + content_position_, std::ios::beg) /* Move the file pointer directly */
            : internal_buffer_->seekp(pos, std::ios::beg);                // Move the internal buffer pointer
    }

    bool eof()
    {
        return (internal_buffer_->peek() == EOF);
    }

    std::shared_ptr<std::stringstream>& getInternalBuffer()
    {
        return internal_buffer_;
    }

    std::size_t tellPosition()
    {
        return internal_buffer_->tellp();
    }

    std::shared_ptr<std::fstream>& getFileHandle()
    {
        return file_handle_;
    }

    Header geFileHeader() const
    {
        return file_header_;
    }

    template <typename T> void write(T const& value)
    {
        assert((flags_ & _OUT_) && "Cannot write into file, 'Out' flag is not set !");
        serializer::Serializer::serialize(*internal_buffer_, value);
    }

    template <typename T> void read(T& value)
    {
        assert((flags_ & _IN_) && "Cannot read from file, 'In' flag is not set !");
        (flags_ & _SKIP_BUFFER_USE_IN_) ? serializer::Serializer::deserialize(*file_handle_, value)
                                        : serializer::Serializer::deserialize(*internal_buffer_, value);
    }

    operator bool() const
    {
        return IsOpen();
    }

private:
    void                               readFileHeader();
    void                               writeFileHeader();
    void                               updateChecksum();
    void                               updateSize();
    file::flag_t                       flags_{file::_NONE_};
    Header                             file_header_;
    std::string                        file_path_;
    std::shared_ptr<std::stringstream> internal_buffer_;
    std::shared_ptr<std::fstream>      file_handle_;
    std::streampos                     version_position_{0U}, size_posotion_{0U}, check_sum_position_{0U}, content_position_{0U};
};
} // namespace file
} // namespace vshade

#endif // ENGINE_CORE_SERIALIZING_FILE_H