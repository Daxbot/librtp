/**
 * @file rtp_header.c
 * @brief RTP header.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 */

#include <stdlib.h>
#include <string.h>

#include "rtp/rtp_header.h"
#include "util.h"

rtp_header *rtp_header_create()
{
    rtp_header *header = (rtp_header*)malloc(sizeof(rtp_header));
    if(header)
        memset(header, 0, sizeof(rtp_header));

    return header;
}

void rtp_header_free(rtp_header *header)
{
    if(!header)
        return;

    if(header->csrc)
        free(header->csrc);

    if(header->ext_data)
        free(header->ext_data);

    free(header);
}

int rtp_header_init(rtp_header *header, uint8_t pt)
{
    if(!header)
        return -1;

    header->version = 2;
    header->pt = pt;
    header->seq = rand();
    header->ts = rand();
    header->ssrc = rand();

    return 0;
}

int rtp_header_size(const rtp_header *header)
{
    if(!header)
        return -1;

    int size = 12;

    if(header->csrc && header->cc)
        size += 4 * header->cc;

    if(header->x && header->ext_count && header->ext_data)
        size += 4 * (1 + header->ext_count);

    return size;
}

int rtp_header_serialize(const rtp_header *header, uint8_t *buffer, int size)
{
    if(!header || !buffer)
        return -1;

    const int header_size = rtp_header_size(header);
    if(size < header_size)
        return -1;

    // Clear existing data
    memset(buffer, 0, header_size);

    buffer[0] = (2 << 6);
    buffer[1] = ((header->m & 1) << 7) | (header->pt & 0x7f);

    write_u16(buffer + 2, header->seq);
    write_u32(buffer + 4, header->ts);
    write_u32(buffer + 8, header->ssrc);

    if(header->cc && header->csrc) {
        buffer[0] |= (header->cc & 7);

        uint8_t *csrc_start = buffer + 12;
        for(uint8_t i = 0; i < header->cc; i++)
            write_u32(csrc_start + (i * 4), header->csrc[i]);
    }

    // Add extension header
    if(header->x && header->ext_count && header->ext_data) {
        buffer[0] |= (1 << 4);

        uint8_t *ext_hdr = buffer + (12 + (4 * header->cc));
        write_u16(ext_hdr, header->ext_id);
        write_u16(ext_hdr + 2, header->ext_count);

        uint8_t *ext_data = ext_hdr + 4;
        for(uint8_t i = 0; i < header->ext_count; i++)
            write_u32(ext_data + (i * 4), header->ext_data[i]);
    }

    return header_size;
}


int rtp_header_parse(rtp_header *header, const uint8_t *buffer, int size)
{
    // Check initial size
    if(!header || !buffer || size < 12)
        return -1;

    header->version = (buffer[0] >> 6) & 3;
    header->pt = buffer[1] & 0x7f;

    // Sanity check
    if(header->version != 2 || header->pt == 0)
        return -1;

    header->x = (buffer[0] >> 4) & 1;
    header->cc = buffer[0] & 7;
    header->m = (buffer[1] >> 7) & 1;
    header->seq = read_u16(buffer + 2);
    header->ts = read_u32(buffer + 4);
    header->ssrc = read_u32(buffer + 8);

    // Recheck size
    if(size < rtp_header_size(header))
        return -1;

    // Contributing source IDs
    if(header->cc) {
        header->csrc = (uint32_t*)malloc(4 * header->cc);

        const uint8_t *csrc_start = buffer + 12;
        for(uint8_t i = 0; i < header->cc; i++)
            header->csrc[i] = read_u32(csrc_start + (4 * i));
    }

    // Extension header
    if(header->x) {
        const uint8_t *ext_hdr = buffer + (12 + (4 * header->cc));
        header->ext_id = read_u16(ext_hdr);
        header->ext_count = read_u16(ext_hdr + 2);
        header->ext_data = (uint32_t*)malloc(4 * header->ext_count);

        const uint8_t *ext_data = ext_hdr + 4;
        for(uint16_t i = 0; i < header->ext_count; ++i)
            header->ext_data[i] = read_u32(ext_data + (4 * i));
    }

    return 0;
}

int rtp_header_find_csrc(rtp_header *header, uint32_t csrc)
{
    if(header) {
        for(uint8_t i = 0; i < header->cc; ++i) {
            if(header->csrc[i] == csrc)
                return i;
        }
    }

    return -1;
}

int rtp_header_add_csrc(rtp_header *header, uint32_t csrc)
{
    if(!header)
        return -1;

    if(!header->cc) {
        header->csrc = (uint32_t*)malloc(sizeof(uint32_t));
        header->cc = 1;
    }
    else {
        if(header->cc == 0xff)
            return -1;

        if(rtp_header_find_csrc(header, csrc) != -1)
            return -1;

        header->cc += 1;

        const size_t nmemb = header->cc * sizeof(uint32_t);
        header->csrc = (uint32_t*)realloc(header->csrc, nmemb);
    }

    header->csrc[header->cc - 1] = csrc;

    return 0;
}

int rtp_header_remove_csrc(rtp_header *header, uint32_t csrc)
{
    if(!header)
        return -1;

    const int index = rtp_header_find_csrc(header, csrc);
    if(index < 0)
        return 0;

    const size_t size = (header->cc - index) * sizeof(uint32_t);
    if(size)
        memmove(&header->csrc[index], &header->csrc[index + 1], size);

    header->cc -= 1;
    if(header->cc > 0) {
        const size_t nmemb = header->cc * sizeof(uint32_t);
        header->csrc = (uint32_t*)realloc(header->csrc, nmemb);
    }
    else {
        free(header->csrc);
        header->csrc = NULL;
    }

    return 0;
}

int rtp_header_set_ext(
    rtp_header *header, uint16_t id, const uint32_t *data, int size)
{
    if(!header || !data)
        return -1;

    if(header->ext_data)
        free(header->ext_data);

    header->ext_id = id;
    header->ext_count = size;
    header->ext_data = (uint32_t*)malloc(4 * size);

    memcpy(header->ext_data, data, size * 4);
    return 0;
}

int rtp_header_clear_ext(rtp_header *header)
{
    if(!header)
        return -1;

    if(header->ext_data) {
        free(header->ext_data);
        header->ext_id = 0;
        header->ext_count = 0;
    }

    return 0;
}