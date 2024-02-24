#include "drv_flash.h"
#include "hardware/flash.h"
#include <stdio.h>

namespace drv
{

FlashDriver::FlashDriver()
{
    //find the next available page
    next_page = starting_page;
    while (next_page < num_pages) {
        auto data = read_ptr(next_page);
        if (data[0] == magic_header[0] && data[1] == magic_header[1]) {
            //found a page with the magic header, so increment page and continue
            next_page++;
        } else if (data[0] == 0xff && data[1] == 0xff) {
            //found a page without the magic header, so break
            break;
        } else {
            printf("FlashDriver::FlashDriver() - found a page with an invalid magic header, so erase the current secctor and the next MB\n");
            // find the current sector
            uint32_t current_sector = next_page / num_pages_per_sector;
            // erase the next MB
            erase(current_sector * flash_sector_size, flash_sector_size);
            printf("FlashDriver::FlashDriver() - erased the next MB\n");
            break;
        }
    }
    start_page_this_session = next_page;

    // if the last page was not a session header, then write a session header
    if (next_page > 0) {
        auto data = read_ptr(next_page - 1);
        if (has_magic_and_session_header(data) == false) {
            printf("FlashDriver::FlashDriver() - the last page was not an unused session (not a session header), so start a new session\n");
            write_session_header();
        }
    }
}

FlashDriver::~FlashDriver()
{
}

void FlashDriver::write_next_usable_page_size(uint8_t* data)
{
    //check that next_page is less than the number of pages
    if (next_page >= num_pages) {
        printf("FlashDriver::write_next_usable_page() - next_page is greater than or equal to the number of pages\n");
        return;
    }
    //check that data is not null
    if (data == nullptr) {
        printf("FlashDriver::write_next_usable_page() - data is null\n");
        return;
    }

    uint8_t buffer[flash_page_size];
    //copy the magic header into the buffer
    for (size_t i = 0; i < sizeof(magic_header); i++) {
        buffer[i] = magic_header[i];
    }
    //copy the data into the buffer
    for (size_t i = 0; i < usable_flash_page_size; i++) {
        buffer[i + sizeof(magic_header)] = data[i];
    }
    //write the buffer to the flash
    printf("FlashDriver::write_next_usable_page() - write next usable page size\n");
    write(next_page * flash_page_size, buffer, flash_page_size);
    //increment next_page
    next_page++;
}

void FlashDriver::dump_log_this_session()
{
    printf("FlashDriver::dump_log_this_session() - dump the log this session\n");
    //dump the log this session
    uint8_t buffer[usable_flash_page_size];
    for (uint32_t page = start_page_this_session; page < next_page; page++) {
        read_into_buffer(page, 0, buffer, usable_flash_page_size);
        printf("FlashDriver - session page %lu, log page %lu, raw page %lu\n", page - start_page_this_session, page - starting_page, page);
        for (size_t i = 0; i < usable_flash_page_size; i++) {
            printf("0x%02X ", buffer[i]);
        }
        printf("\n");
    }
}

void FlashDriver::dump_full_log()
{
    printf("FlashDriver::dump_full_log() - dump the full log\n");
    //dump the full log
    uint8_t buffer[usable_flash_page_size];
    for (uint32_t page = starting_page; page < next_page; page++) {
        read_into_buffer(page, 0, buffer, usable_flash_page_size);
        printf("FlashDriver - log page %lu, raw page %lu\n", page - starting_page, page);
        for (size_t i = 0; i < usable_flash_page_size; i++) {
            printf("0x%02X ", buffer[i]);
        }
        printf("\n");
    }
}

void FlashDriver::dump_log_last_session()
{
    printf("FlashDriver::dump_log_last_session() - dump the log last session\n");
    //go backwards and find the last session header
    uint32_t last_session_page = seek_previous_session();
    //dump the log last session
    uint8_t buffer[usable_flash_page_size];
    for (uint32_t page = last_session_page; page < start_page_this_session; page++) {
        read_into_buffer(page, 0, buffer, usable_flash_page_size);
        printf("FlashDriver - session page %lu, log page %lu, raw page %lu\n", page - last_session_page, page - starting_page, page);
        for (size_t i = 0; i < usable_flash_page_size; i++) {
            printf("0x%02X ", buffer[i]);
        }
        printf("\n");
    }
}

void FlashDriver::reset_log()
{
    printf("FlashDriver::reset_log() - erase the entire log section\n");
    //erase the entire log section
    flash_range_erase(starting_page * flash_page_size, bytes_16MB - starting_page * flash_page_size);
}


bool FlashDriver::has_previous_session()
{
    return seek_previous_session() != num_pages;
}

uint8_t* FlashDriver::get_log_ptr_previous_session()
{
    return read_ptr(seek_previous_session()+1);
}

uint8_t* FlashDriver::get_log_ptr()
{
    return read_ptr(starting_page);
}

uint8_t* FlashDriver::get_log_ptr_this_session()
{
    return read_ptr(start_page_this_session);
}

void FlashDriver::write_session_header()
{
    printf("FlashDriver::write_session_header() - write a session header\n");
    //write a session header
    uint8_t buffer[usable_flash_page_size];
    for (size_t i = 0; i < usable_flash_page_size; i++) {
        buffer[i] = 0xff;
    }
    // write session header
    for (size_t i = 0; i < sizeof(session_header); i++) {
        buffer[i] = session_header[i];
    }
    //write the buffer to the flash
    write_next_usable_page_size(buffer);
}

void FlashDriver::erase(uint32_t offset, size_t count)
{
    //check that offset is a multiple of flash_sector_size
    if (offset % flash_sector_size != 0) {
        printf("FlashDriver::erase() - offset is not a multiple of flash_sector_size\n");
        return;
    }
    //check that count is a multiple of flash_sector_size
    if (count % flash_sector_size != 0) {
        printf("FlashDriver::erase() - count is not a multiple of flash_sector_size\n");
        return;
    }
    //check that offset + count is less than the size of the flash
    if (offset + count > bytes_16MB) {
        printf("FlashDriver::erase() - offset + count is greater than the size of the flash\n");
        return;
    }
    //erase the flash
    flash_range_erase(offset, count);
}

void FlashDriver::write(uint32_t offset, const uint8_t* data, size_t count)
{
    //check that offset is a multiple of flash_page_size
    if (offset % flash_page_size != 0) {
        printf("FlashDriver::write() - offset is not a multiple of flash_page_size\n");
        return;
    }
    //check that count is a multiple of flash_page_size
    if (count % flash_page_size != 0) {
        printf("FlashDriver::write() - count is not a multiple of flash_page_size\n");
        return;
    }
    //check that offset + count is less than the size of the flash
    if (offset + count > bytes_16MB) {
        printf("FlashDriver::write() - offset + count is greater than the size of the flash\n");
        return;
    }
    //write to the flash
    puts("FlashDriver::write() - write to the flash");
    printf("FlashDriver::write() - offset = %lu, count = %lu\n", offset, count);
    printf("FlashDriver::write() - data = %08X\n", data);
    flash_range_program(offset, data, count);
}

uint8_t* FlashDriver::read_ptr(uint32_t page)
{
    return read_ptr(page, 0);
}

uint8_t* FlashDriver::read_ptr(uint32_t page, uint32_t offset)
{
    // The XIP_BASE is the base address of the flash memory which does not start at 0x0
    uint8_t* read_ptr = (uint8_t*)XIP_BASE + (page * flash_page_size) + offset;
    return read_ptr;
}

void FlashDriver::read_into_buffer(uint32_t page, uint32_t offset, uint8_t* buffer, size_t count)
{
    //check that offset is a multiple of flash_page_size
    if (offset % flash_page_size != 0) {
        printf("FlashDriver::read_into_buffer() - offset is not a multiple of flash_page_size\n");
        return;
    }
    //check that offset + count is less than the size of the flash
    if (offset + count > bytes_16MB) {
        printf("FlashDriver::read_into_buffer() - offset + count is greater than the size of the flash\n");
        return;
    }
    //read from the flash
    const uint8_t* data = read_ptr(page, offset);
    for (size_t i = 0; i < count; i++) {
        buffer[i] = data[i];
    }
}

uint32_t FlashDriver::seek_previous_session()
{
    return seek_previous_session_from_page(starting_page);
}

uint32_t FlashDriver::seek_previous_session_from_page(uint32_t page)
{
    //go backwards and find the last session header
    uint32_t last_session_page = num_pages;
    for (uint32_t page = start_page_this_session - 1; page >= starting_page; page--) {
        auto data = read_ptr(page);
        if (has_magic_and_session_header(data)) {
            last_session_page = page;
            break;
        }
    }
    return last_session_page;
}

bool FlashDriver::has_magic_and_session_header(uint8_t* data)
{
    bool headersMatch = true;
    for (size_t i = 0; i < sizeof(magic_header); i++) {
        if (data[i] != magic_header[i]) {
            headersMatch = false;
            break;
        }
    }
    if (headersMatch) {
        for (size_t i = 0; i < sizeof(session_header); i++) {
            if (data[i + sizeof(magic_header)] != session_header[i]) {
                headersMatch = false;
                break;
            }
        }
    }
    return headersMatch;
}

} // namespace drv
