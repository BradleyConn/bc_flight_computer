#ifndef DRV_FLASH_H
#define DRV_FLASH_H

#include <stddef.h>
#include <stdint.h>

namespace drv
{

// A quick and dirty flash driver for logging
// TODO: Use STL containers instead of C arrays and clean this up
// TODO: This should be split into a flash driver and a data logger
class FlashDriver
{
public:
    // Constructor
    FlashDriver();

    // Destructor
    ~FlashDriver();

    // This is the main function that should be used
    // the data size is flash_page_size - magic_bytes
    // NOTE: This appears to take 639 us to execute
    void write_next_usable_page_size(uint8_t* data);

    void dump_log_this_session();
    void dump_full_log();
    void dump_log_last_session();

    // Erase the entire log section
    void reset_log();

    //--------------------------------------------------
    // Probably don't need these functions

    // Get beginning of log
    uint8_t* get_log_ptr();

    // Get beginning of log this session
    uint8_t* get_log_ptr_this_session();

    // Write a session header
    void write_session_header();

    // offset - is the offset in bytes from the start of the flash, must be a multiple of 4096
    // byte_count - is the number of bytes to erase, must be a multiple of 4096
    void erase(uint32_t offset, size_t byte_count);

    // offset - the offset in bytes from the start of the flash, must be a multiple of 256
    // byte_count - the number of bytes to write, must be a multiple of 256
    void write(uint32_t offset, const uint8_t* data, size_t byte_count);

    // page - the page to read from, must be less than num_pages
    uint8_t* read_ptr(uint32_t page);

    // page - the page to read from, must be less than num_pages
    // offset - the offset in bytes from the start of the page, must be less than flash_page_size
    uint8_t* read_ptr(uint32_t page, uint32_t offset);

    // page - the page to read from, must be less than num_pages
    // offset - the offset in bytes from the start of the page, must be less than flash_page_size
    // buffer - the buffer to read into
    // count - the number of bytes to read
    void read_into_buffer(uint32_t page, uint32_t offset, uint8_t* buffer, size_t count);

    uint32_t get_next_page() const
    {
        return next_page;
    }

    static constexpr uint32_t flash_page_size = 256;
    static constexpr uint32_t flash_sector_size = 4096;
    static constexpr uint32_t bytes_16MB = 0x1000000;
    static constexpr uint32_t num_pages = bytes_16MB / flash_page_size;
    static constexpr uint32_t num_sectors = bytes_16MB / flash_sector_size;
    static constexpr uint32_t num_pages_per_sector = flash_sector_size / flash_page_size;
    static constexpr uint8_t magic_header[] = { 0x12, 0x34 };
    static constexpr uint32_t magic_header_size = sizeof(magic_header);
    static constexpr uint8_t session_header[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    //Randomly select halfway
    static constexpr uint32_t starting_page = num_pages / 2;
    static constexpr uint32_t usable_flash_page_size = flash_page_size - sizeof(magic_header);

private:
    // Constructor should seek to the next available page
    uint32_t next_page = 0;
    // excludes session header
    uint32_t start_page_this_session = 0;

    bool has_magic_and_session_header(uint8_t* data);
    ;
};

};     // namespace drv
#endif // DRV_FLASH_H
