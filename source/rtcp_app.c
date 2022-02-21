/**
 * @file rtcp_app.c
 * @brief RTCP application-defined packet.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 */

#include <stdlib.h>
#include <string.h>

#include "rtp/rtcp_app.h"
#include "util.h"

rtcp_app *rtcp_app_create()
{
    rtcp_app *packet = (rtcp_app*)malloc(sizeof(rtcp_app));
    if(packet)
        memset(packet, 0, sizeof(rtcp_app));

    return packet;
}

void rtcp_app_free(rtcp_app *packet)
{
    if(!packet)
        return;

    if(packet->app_data)
        free(packet->app_data);

    free(packet);
}

int rtcp_app_init(rtcp_app *packet, uint8_t subtype)
{
    if(!packet)
        return -1;

    packet->header.app.version = 2;
    packet->header.app.subtype = subtype;
    packet->header.app.pt = RTCP_APP;
    packet->header.app.length = 2;

    return 0;
}

int rtcp_app_size(const rtcp_app *packet)
{
    if(!packet)
        return -1;

    int size = 12;
    if(packet->app_data && packet->app_size) {
        size += packet->app_size;
        size += 4 - (size % 4);
    }

    return size;
}

int rtcp_app_serialize(
    const rtcp_app *packet, uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    const int packet_size = rtcp_app_size(packet);
    if(size < packet_size)
        return -1;

    memset(buffer, 0, packet_size);
    if(rtcp_header_serialize(&packet->header, buffer, size) < 0)
        return -1;

    write_u32(buffer + 4, packet->ssrc);
    write_u32(buffer + 8, packet->name);

    if(packet->app_data)
        memcpy(buffer + 12, packet->app_data, packet->app_size);

    return packet_size;
}

int rtcp_app_parse(rtcp_app *packet, const uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    const int pt = rtcp_header_parse(&packet->header, buffer, size);
    if(pt != RTCP_APP)
        return -1;

    packet->ssrc = read_u32(buffer + 4);
    packet->name = read_u32(buffer + 8);

    const int length = (packet->header.common.length + 1) * 4;
    packet->app_size = length - 12;

    if(packet->app_size) {
        packet->app_data = malloc(packet->app_size);
        memcpy(packet->app_data, buffer + 12, packet->app_size);
    }

    return 0;
}

int rtcp_app_set_data(rtcp_app *packet, const void *data, int size)
{
    if(!packet || !data || packet->app_data)
        return -1;

    packet->app_data = malloc(size);
    if(!packet->app_data)
        return -1;

    packet->app_size = size;
    memcpy(packet->app_data, data, size);

    // Update header length
    packet->header.common.length = (rtcp_app_size(packet) / 4) - 1;

    return 0;
}

int rtcp_app_clear_data(rtcp_app *packet)
{
    if(!packet)
        return -1;

    if(packet->app_data) {
        free(packet->app_data);
        packet->app_data = NULL;
        packet->app_size = 0;
    }

    // Update header length
    packet->header.common.length = (rtcp_app_size(packet) / 4) - 1;

    return 0;
}