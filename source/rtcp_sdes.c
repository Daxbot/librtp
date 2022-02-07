/**
 * @file rtcp_sdes.h
 * @brief RTCP source description packet.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 */

#include <stdlib.h>
#include <string.h>

#include "rtp/rtcp_sdes.h"
#include "util.h"

/**
 * @brief Return a SDES item's size.
 *
 * @param [in] item - item to check.
 * @return item size in bytes or -1 on failure.
 * @private
 */
static int item_size(const rtcp_sdes_item *item)
{
    if(!item)
        return -1;

    return 2 + item->length;
}

/**
 * @brief Return a SDES entry's size.
 *
 * @param [in] source - source to check.
 * @return source size in bytes or -1 on failure.
 * @private
 */
static int entry_size(const rtcp_sdes_entry *source)
{
    if(!source)
        return -1;

    int size = 4;

    for(uint8_t i = 0; i < source->item_count; ++i)
        size += item_size(&source->items[i]);

    // Entry must end on a 32-bit boundary
    return size + (4 - (size % 4));
}

/**
 * @brief Find an item in a source entry.
 *
 * @param [in] source - source to search.
 * @param [in] type - item to find.
 * @return rtcp_sdes_item*
 * @private
 */
static rtcp_sdes_item *find_item(
    rtcp_sdes_entry *source, rtcp_sdes_type type)
{
    if(source) {
        for(uint8_t i = 0; i < source->item_count; ++i) {
            rtcp_sdes_item *item = &source->items[i];
            if(item->type == type)
                return item;
        }
    }

    return NULL;
}

/**
 * @brief Add an item to a source entry.
 *
 * @param [out] source - source to add to.
 * @param [in] type - item type to add.
 * @param [in] data - item data.
 * @param [in] size - item data size.
 * @return rtcp_sdes_item* - newly created item.
 * @private
 */
static rtcp_sdes_item *add_item(
    rtcp_sdes_entry *source, rtcp_sdes_type type, const void *data, int size)
{
    if(!source || !data)
        return NULL;

    if(find_item(source, type))
        return NULL;

    if(source->item_count == 0) {
        source->items = (rtcp_sdes_item*)malloc(sizeof(rtcp_sdes_item));
        source->item_count = 1;
    }
    else {
        if(source->item_count == 0xff)
            return NULL;

        source->item_count += 1;

        const size_t nmemb = source->item_count * sizeof(rtcp_sdes_item);
        source->items = (rtcp_sdes_item*)realloc(source->items, nmemb);
    }

    rtcp_sdes_item *item = &source->items[source->item_count - 1];
    item->type = type;
    item->length = size;
    item->data = malloc(size);
    memcpy(item->data, data, size);

    return item;
}

/**
 * @brief Remove an item from a source entry.
 *
 * @param [out] source - source to remove from.
 * @param [in] type - item type to remove.
 * @return 0 on success.
 * @private
 */
static int remove_item(rtcp_sdes_entry *source, rtcp_sdes_type type)
{
    if(!source)
        return -1;

    rtcp_sdes_item *item = find_item(source, type);
    if(item == NULL)
        return 0;

    free(item->data);

    const size_t index = (item - source->items) / sizeof(rtcp_sdes_item);
    const size_t size = (source->item_count - index) * sizeof(rtcp_sdes_item);
    if(size)
        memmove(item, item + 1, size);

    source->item_count -= 1;
    if(source->item_count > 0) {
        const size_t nmemb = source->item_count * sizeof(rtcp_sdes_item);
        source->items = (rtcp_sdes_item*)realloc(source->items, nmemb);
    }
    else {
        free(source->items);
        source->items = NULL;
    }

    return 0;
}

/**
 * @brief Write a source entry to a buffer.
 *
 * @param [in] source - source to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 * @private
 */
static int serialize_entry(
    const rtcp_sdes_entry *source, uint8_t *buffer, int size)
{
    if(!source || !buffer)
        return -1;

    const int source_size = entry_size(source);
    if(size < source_size)
        return -1;

    write_u32(buffer, source->id);

    int offset = 4;
    for(uint8_t i = 0; i < source->item_count; ++i) {
        rtcp_sdes_item *item = &source->items[i];
        buffer[offset++] = item->type;
        buffer[offset++] = item->length;
        memcpy(buffer + offset, item->data, item->length);
        offset += item->length;
    }

    while(offset < source_size)
        buffer[offset++] = '\0';

    return source_size;
}

rtcp_sdes *rtcp_sdes_create()
{
    rtcp_sdes *packet = (rtcp_sdes*)malloc(sizeof(rtcp_sdes));
    if(packet)
        memset(packet, 0, sizeof(rtcp_sdes));

    return packet;
}

void rtcp_sdes_free(rtcp_sdes *packet)
{
    if(!packet)
        return;

    if(packet->srcs) {
        // Free sources
        for(uint8_t i = 0; i < packet->header.common.count; ++i) {
            rtcp_sdes_entry *source = &packet->srcs[i];

            // Free source items
            for(uint8_t j = 0; j < source->item_count; ++j) {
                rtcp_sdes_item *item = &source->items[j];
                if(item->data)
                    free(item->data);
            }

            free(source->items);
        }

        free(packet->srcs);
    }

    free(packet);
}

int rtcp_sdes_init(rtcp_sdes *packet)
{
    if(!packet)
        return -1;

    packet->header.common.version = 2;
    packet->header.common.pt = RTCP_SDES;

    return 0;
}

int rtcp_sdes_size(const rtcp_sdes *packet)
{
    if(!packet)
        return -1;

    int size = 4; // Header

    rtcp_sdes_entry *sources = (rtcp_sdes_entry*)packet->srcs;
    for(uint8_t i = 0; i < packet->header.common.count; ++i)
        size += entry_size(&sources[i]);

    return size;
}

int rtcp_sdes_serialize(
    const rtcp_sdes *packet, uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    const int packet_size = rtcp_sdes_size(packet);
    if(size < packet_size)
        return -1;

    if(rtcp_header_serialize(&packet->header, buffer, size) < 0)
        return -1;

    int offset = 4;
    for(uint8_t i = 0; i < packet->header.common.count; ++i) {
        offset += serialize_entry(
            &packet->srcs[i], buffer + offset, size - offset);
    }

    return packet_size;
}

int rtcp_sdes_parse(
    rtcp_sdes *packet, const uint8_t *buffer, int size)
{
    // Check initial size
    if(!packet || !buffer)
        return -1;

    const int pt = rtcp_header_parse(&packet->header, buffer, size);
    if(pt != RTCP_SDES)
        return -1;

    // Parse sources
    if(packet->header.common.count) {
        packet->srcs = (rtcp_sdes_entry*)calloc(
            packet->header.common.count, sizeof(rtcp_sdes_entry));

        int offset = 4;
        for(uint8_t i = 0; i < packet->header.common.count; ++i) {
            // Initialize the source
            rtcp_sdes_entry *source = &packet->srcs[i];
            source->id = read_u32(buffer + offset);
            source->item_count = 0;
            source->items = NULL;
            offset += 4;

            // Navigate the list
            while(1) {
                const uint8_t *data = buffer + offset;
                const uint8_t type = data[0];
                if(type == 0) {
                    // End of list - move pointer to next 4 byte boundary
                    offset += 4 - (offset % 4);
                    break;
                }

                const uint8_t length = data[1];
                rtcp_sdes_item *item = add_item(
                    source, type, data + 2, length);

                offset += item_size(item);
            }
        }
    }

    return 0;
}


rtcp_sdes_entry *rtcp_sdes_find_entry(rtcp_sdes *packet, uint32_t id)
{
    if(packet) {
        for(uint8_t i = 0; i < packet->header.common.count; ++i) {
            rtcp_sdes_entry *source = &packet->srcs[i];
            if(source->id == id)
                return source;
        }
    }

    return NULL;
}

rtcp_sdes_entry *rtcp_sdes_add_entry(rtcp_sdes *packet, uint32_t id)
{
    if(!packet)
        return NULL;

    if(!packet->header.common.count) {
        packet->srcs = (rtcp_sdes_entry*)malloc(sizeof(rtcp_sdes_entry));
        packet->header.common.count = 1;
    }
    else {
        if(packet->header.common.count == 0xff)
            return NULL;

        if(rtcp_sdes_find_entry(packet, id))
            return NULL;

        packet->header.common.count += 1;

        const size_t nmemb = packet->header.common.count * sizeof(rtcp_sdes_entry);
        packet->srcs = (rtcp_sdes_entry*)realloc(packet->srcs, nmemb);
    }

    rtcp_sdes_entry *source = &packet->srcs[packet->header.common.count - 1];
    source->id = id;
    source->item_count = 0;
    source->items = NULL;

    return source;
}

int rtcp_sdes_remove_entry(rtcp_sdes *packet, uint32_t id)
{
    if(!packet)
        return -1;

    rtcp_sdes_entry *source = rtcp_sdes_find_entry(packet, id);
    if(!source)
        return 0;

    // Free the source's items
    for(uint8_t i = 0; i < source->item_count; ++i) {
        rtcp_sdes_item *item = &source->items[i];
        if(item->data)
            free(item->data);

        free(item);
    }

    const size_t index = (source - packet->srcs) / sizeof(rtcp_sdes_entry);
    const size_t size = (packet->header.common.count - index) * sizeof(rtcp_sdes_entry);
    if(size)
        memmove(source, source + 1, size);

    packet->header.common.count -= 1;
    if(packet->header.common.count > 0) {
        const size_t nmemb = packet->header.common.count * sizeof(rtcp_sdes_entry);
        packet->srcs = (rtcp_sdes_entry*)realloc(packet->srcs, nmemb);
    }
    else {
        free(packet->srcs);
        packet->srcs = NULL;
    }

    return 0;
}

int rtcp_sdes_get_item(rtcp_sdes *packet, uint32_t src, rtcp_sdes_type type,
    char *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    rtcp_sdes_entry *source = rtcp_sdes_find_entry(packet, src);
    if(source == NULL)
        return -1;

    rtcp_sdes_item *item = find_item(source, type);
    if(size < item->length + 1)
        return -1;

    memcpy(buffer, item->data, item->length);
    buffer[item->length] = '\0';

    return item->length;
}

int rtcp_sdes_set_item(
    rtcp_sdes *packet, uint32_t src, rtcp_sdes_type type, const char *data)
{
    if(!packet || !data)
        return -1;

    rtcp_sdes_entry *source = rtcp_sdes_find_entry(packet, src);
    if(!source)
        return -1;

    if(find_item(source, type))
        remove_item(source, type);

    if(add_item(source, type, data, strlen(data)) == NULL)
        return -1;

    // Update header length
    packet->header.common.length = (rtcp_sdes_size(packet) / 4) - 1;

    return 0;
}

int rtcp_sdes_clear_item(rtcp_sdes *packet, uint32_t src, rtcp_sdes_type type)
{
    if(!packet || type == RTCP_SDES_END)
        return -1;

    rtcp_sdes_entry *source = rtcp_sdes_find_entry(packet, src);
    if(!source)
        return 0;

    remove_item(source, type);

    // Update header length
    packet->header.common.length = (rtcp_sdes_size(packet) / 4) - 1;

    return 0;
}
