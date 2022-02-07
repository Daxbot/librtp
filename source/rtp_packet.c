/**
 * @file rtp_packet.c
 * @brief RTP packet.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 * @ingroup rtp
 */

#include <stdlib.h>
#include <string.h>

#include "rtp/rtp_packet.h"

rtp_packet *rtp_packet_create()
{
    rtp_packet *packet = (rtp_packet*)malloc(sizeof(rtp_packet));

    if(packet) {
        memset(packet, 0, sizeof(rtp_packet));
        packet->header = rtp_header_create();

        if(!packet->header) {
            free(packet);
            packet = NULL;
        }
    }

    return packet;
}

void rtp_packet_free(rtp_packet *packet)
{
    if(!packet)
        return;

    if(packet->header)
        rtp_header_free(packet->header);

    if(packet->payload_data)
        free(packet->payload_data);

    free(packet);
}

int rtp_packet_init(rtp_packet *packet, uint8_t pt)
{
    if(!packet)
        return -1;

    return rtp_header_init(packet->header, pt);
}

int rtp_packet_size(const rtp_packet *packet)
{
    if(!packet)
        return -1;

    return rtp_header_size(packet->header) + packet->payload_size;
}

int rtp_packet_serialize(const rtp_packet *packet, uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    const int packet_size = rtp_packet_size(packet);
    if(size < packet_size)
        return -1;

    // Add header
    int offset = rtp_header_serialize(packet->header, buffer, size);
    if(offset < 0)
        return offset;

    // Add payload
    memcpy(buffer + offset, packet->payload_data, packet->payload_size);

    return packet_size;
}

int rtp_packet_parse(rtp_packet *packet, const uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    if(rtp_header_parse(packet->header, buffer, size) < 0)
        return -1;

    const int header_size = rtp_header_size(packet->header);
    rtp_packet_set_payload(packet, buffer + header_size, size - header_size);

    return 0;
}

int rtp_packet_set_payload(
    rtp_packet *packet, const void *data, int size)
{
    if(!packet || !data || packet->payload_data)
        return -1;

    packet->payload_data = malloc(size);
    if(!packet->payload_data)
        return -1;

    packet->payload_size = size;
    memcpy(packet->payload_data, data, size);

    return 0;
}

int rtp_packet_clear_payload(rtp_packet *packet)
{
    if(!packet)
        return -1;

    if(packet->payload_data) {
        free(packet->payload_data);
        packet->payload_data = NULL;
        packet->payload_size = 0;
    }

    return 0;
}