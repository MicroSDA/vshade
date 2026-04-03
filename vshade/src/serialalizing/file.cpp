#include "engine/core/serialalizing/file.h"
#include <engine/core/application/application.h>

namespace utils
{
static unsigned int const CRC32Table[256] = {
    0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL, 0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L, 0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL,
    0x97D2D988L, 0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L, 0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL, 0x1ADAD47DL, 0x6DDDE4EBL,
    0xF4D4B551L, 0x83D385C7L, 0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL, 0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L, 0x3B6E20C8L,
    0x4C69105EL, 0xD56041E4L, 0xA2677172L, 0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL, 0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
    0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L, 0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L, 0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L,
    0xB8BDA50FL, 0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L, 0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL, 0x76DC4190L, 0x01DB7106L,
    0x98D220BCL, 0xEFD5102AL, 0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L, 0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L, 0x7F6A0DBBL,
    0x086D3D2DL, 0x91646C97L, 0xE6635C01L, 0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL, 0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
    0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL, 0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L, 0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L,
    0xD4BB30E2L, 0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL, 0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L, 0x44042D73L, 0x33031DE5L,
    0xAA0A4C8FL, 0xDD0D7CC9L, 0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L, 0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL, 0x5EDEF90EL,
    0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L, 0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL, 0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
    0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L, 0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L, 0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L,
    0x7D079EB1L, 0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL, 0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L, 0xFED41B76L, 0x89D32BE0L,
    0x10DA7A5AL, 0x67DD4ACCL, 0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L, 0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L, 0xD1BB67F1L,
    0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL, 0xD80D2BDA,  0xAF0A1B4CL, 0x36034AF6,  0x41047A60L, 0xDF60EFC3L, 0xA867DF55L, 0x316E8EEDL, 0x4669BE79L,
    0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L, 0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL, 0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L,
    0x5CB36A04L, 0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL, 0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL, 0x9C0906A9L, 0xEB0E363FL,
    0x72076785L, 0x05005713L, 0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L, 0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L, 0x86D3D2D4L,
    0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL, 0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L, 0x88085AE6L, 0xFF0F6A70L, 0x66063BDEL, 0x11010B5CL,
    0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L, 0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L, 0xA7672661L, 0xD06016F7L, 0x4969474DL,
    0x3E6E77DBL, 0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L, 0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L, 0xBDBDF21CL, 0xCABAC28AL,
    0x53B39330L, 0x24B4A3A6L, 0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL, 0xB3667A2EL, 0xC4614ABCL, 0x5D681B02L, 0x2A6F2B94L, 0xB40BBE37L,
    0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL};

std::uint32_t generateCheckSumCRC32(std::stringstream& buffer, std::uint32_t initialCRC = 0xFFFFFFFF)
{
    std::uint32_t  crc        = initialCRC;
    std::streampos currentPos = buffer.tellg(); // Save current stream position
    buffer.seekg(0, std::ios::beg);             // Move to the beginning of the stream

    char byte;
    while (buffer.get(byte)) // Read each byte
        crc = (crc >> 8) ^ CRC32Table[(crc ^ static_cast<std::uint8_t>(byte)) & 0xFF];

    buffer.clear();           // Clear any flags on the stream
    buffer.seekg(currentPos); // Restore stream position

    return crc ^ 0xFFFFFFFF; // Return the final CRC32 value
}

std::uint32_t generateCheckSumCRC32(std::string const& data, std::uint32_t initialCRC = 0xFFFFFFFF)
{
    std::uint32_t crc = initialCRC;

    for (char byte : data) // Iterate over each byte in the string
        crc = (crc >> 8) ^ CRC32Table[(crc ^ static_cast<std::uint8_t>(byte)) & 0xFF];

    return crc ^ 0xFFFFFFFF; // Return the final CRC32 value
}

template <typename Bitdepth,
          typename = std::enable_if_t<std::is_same<Bitdepth, std::uint32_t>::value || std::is_same<Bitdepth, std::uint64_t>::value>>
static Bitdepth generateCheckSumHash(std::stringstream const& stream)
{
    return static_cast<Bitdepth>(std::hash<std::string>{}(stream.str()));
}

template <typename Bitdepth,
          typename = std::enable_if_t<std::is_same<Bitdepth, std::uint32_t>::value || std::is_same<Bitdepth, std::uint64_t>::value>>
inline static Bitdepth generateCheckSumHash(std::string const& stream)
{
    return static_cast<Bitdepth>(std::hash<std::string>{}(stream));
}
} // namespace utils

vshade::file::File::File() : internal_buffer_{std::make_shared<std::stringstream>()}, file_handle_{std::make_shared<std::fstream>()}
{
}
vshade::file::File::File(std::string const& file_path, flag_t flags, magic_t const magic, version_t version)
    : file_path_{file_path}, internal_buffer_{std::make_shared<std::stringstream>()}, file_handle_{std::make_shared<std::fstream>()}
{
    openFile(file_path, flags, magic, version);
}
vshade::file::File::File(std::shared_ptr<std::fstream> stream, file::flag_t flags, magic_t const magic, version_t version)
    : file_handle_{stream}, internal_buffer_{std::make_shared<std::stringstream>()}
{
    openFile(stream, flags, magic, version);
}
vshade::file::File::~File()
{
    if (file_handle_.use_count() < 2U)
    {
        closeFile(); 
    }
}

bool vshade::file::File::openFile(std::string const& file_path, flag_t flags, magic_t const magic, version_t version)
{
    file_handle_->open(file_path, flags & _IN_ | flags & _OUT_ | std::ios::binary);
    return openFile(file_handle_, flags, magic, version);
}

bool vshade::file::File::openFile(std::shared_ptr<std::fstream> stream, flag_t flags, magic_t const magic, version_t version)
{
    file_handle_         = stream;
    flags_               = flags;
    file_header_.magic   = magic;
    file_header_.version = version;

    if (flags_ & _IN_ || flags & _OUT_)
    {
        if (!*file_handle_)
        {
            VSHADE_CORE_WARNING("Failed to open file or stream, path '{}'", file_path_);
            return false;
        }

        if (flags_ & _IN_)
        {
            readFileHeader();
            return true;
        }

        return true;
    }
    else
    {
        VSHADE_CORE_ERROR("Failed to open file or stream, _IN_ || _OUT_ flags are not specified, path '{}'", file_path_);
    }
}

void vshade::file::File::readFileHeader()
{
    magic_t magic;
    // Deserialize (read) the magic string from the file
    serializer::Serializer::deserialize(*file_handle_, magic);

    if (!(flags_ & _SKIP_MAGIC_)) // If magic check is not skipped
    {
        if (file_header_.magic != magic)
        {
            VSHADE_CORE_ERROR("Wrong magic value: {} in: {}", magic, file_path_.c_str());
        }
    }

    version_position_ = file_handle_->tellg(); // Save the current position for the version
    version_t version;
    // Deserialize (read) the version from the file
    serializer::Serializer::deserialize(*file_handle_, version);

    if (!(flags_ & _SKIP_VERSION_)) // If version check is not skipped
    {
        if (file_header_.version != version)
        {
            VSHADE_CORE_ERROR("Wrong version value: {} in: {}", version, file_path_.c_str());
        }
    }

    size_posotion_ = file_handle_->tellg(); // Save the current position for the content size
    // Deserialize (read) the content size from the file
    serializer::Serializer::deserialize(*file_handle_, file_header_.content_size);

    check_sum_position_ = file_handle_->tellg(); // Save the current position for the checksum
    checksum_t checksum;
    // Deserialize (read) the checksum from the file
    serializer::Serializer::deserialize(*file_handle_, checksum);

    content_position_ = file_handle_->tellg(); // Save the current position for the file content

    if (!(flags_ & _SKIP_BUFFER_USE_IN_)) // If buffer use for input is not skipped
    {
        // Create a buffer string of the size specified in the header
        std::string buffer(file_header_.content_size, '\0');
        file_handle_->read(buffer.data(), file_header_.content_size); // Read the file content into the buffer

        if (!(flags_ & _SKIP_CHECK_SUM_)) // If checksum check is not skipped
        {
            // Generate the checksum for the buffer using CRC32 or Hash
            file_header_.check_sum = (flags_ & _SUM_SRC_32_) ? utils::generateCheckSumCRC32(buffer) : utils::generateCheckSumHash<checksum_t>(buffer);

            if (file_header_.check_sum != checksum)
            {
                VSHADE_CORE_ERROR("Wrong checksum value: {} != {} in: {}", file_header_.check_sum, checksum, file_path_);
            }
        }

        // Move the buffer content into the internal buffer
        internal_buffer_->str(std::move(buffer));
    }

    // Check if the internal buffer is in a good state
    if (!internal_buffer_->good())
    {
        VSHADE_CORE_ERROR("Failed to read file header in: {}", file_path_);
    }
}

void vshade::file::File::writeFileHeader()
{
    file_handle_->seekp(0U, std::ios::beg); // Set the file pointer to the beginning
    // Serialize (write) the magic string to the file
    serializer::Serializer::serialize(*file_handle_, file_header_.magic);

    version_position_ = file_handle_->tellp(); // Save the current position for the version
    // Serialize (write) the version to the file
    serializer::Serializer::serialize(*file_handle_, file_header_.version);

    size_posotion_ = file_handle_->tellp(); // Save the current position for the content size
    // Serialize (write) a placeholder for content size
    serializer::Serializer::serialize(*file_handle_, checksum_t{0U});

    check_sum_position_ = file_handle_->tellp(); // Save the current position for the checksum
    // Serialize (write) a placeholder for checksum
    serializer::Serializer::serialize(*file_handle_, checksum_t{0U});

    content_position_ = file_handle_->tellp(); // Save the current position for the file content
}

void vshade::file::File::closeFile()
{
    if (file_handle_->is_open())
    {
        if (flags_ & _OUT_) // If the file is opened for output
        {
            writeFileHeader();                          // Write the header to the file
            *file_handle_ << internal_buffer_->rdbuf(); // Write the internal buffer to the file
            updateSize();                               // Update the size in the file header

            if (!(flags_ & _SKIP_CHECK_SUM_)) // If checksum check is not skipped
                updateChecksum();             // Update the checksum in the file header
        }
        file_handle_->close(); // Close the file handle
    }
}

void vshade::file::File::updateChecksum()
{
    // Generate the checksum from the internal buffer using CRC32 or Hash
    checksum_t const checksum =
        (flags_ & _SUM_SRC_32_) ? utils::generateCheckSumCRC32(*internal_buffer_) : utils::generateCheckSumHash<checksum_t>(*internal_buffer_);

    file_handle_->seekp(check_sum_position_); // Move the file pointer to the checksum position
    // Serialize (write) the new checksum to the file
    serializer::Serializer::serialize(*file_handle_, checksum);
}

void vshade::file::File::updateSize()
{
    content_size_t const size = getSize(); // Get the current size of the content
    file_handle_->seekp(size_posotion_);   // Move the file pointer to the size position
    // Serialize (write) the new size to the file
    serializer::Serializer::serialize(*file_handle_, size);
}

std::size_t vshade::file::File::getSize()
{
    internal_buffer_->seekg(0U, std::ios::end);   // Move to the end of the internal buffer
    std::size_t size = internal_buffer_->tellg(); // Get the size from the current position
    internal_buffer_->seekg(0U, std::ios::beg);   // Move back to the beginning of the internal buffer

    return size;
}