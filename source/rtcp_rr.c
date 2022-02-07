/**
 * @file rtcp_rr.c
 * @brief RTCP receiver report packet.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 */

#include <stdlib.h>
#include <string.h>

#include "rtp/rtcp_rr.h"
#include "util.h"

rtcp_rr *rtcp_rr_create()
{
    rtcp_rr *packet = (rtcp_rr*)malloc(sizeof(rtcp_rr));
    if(packet)
        memset(packet, 0, sizeof(rtcp_rr));

    return packet;
}

void rtcp_rr_free(rtcp_rr *packet)
{
    if(!packet)
        return;

    if(packet->ext_data)
        free(packet->ext_data);

    free(packet);
}

int rtcp_rr_init(rtcp_rr *packet)
{
    if(!packet)
        return -1;

    packet->header.common.version = 2;
    packet->header.common.pt = RTCP_RR;
    packet->header.common.length = 1;

    return 0;
}

int rtcp_rr_size(const rtcp_rr *packet)
{
    if(!packet)
        return -1;

    const int report_size = packet->header.common.count * sizeof(rtcp_report);
    return 8 + report_size + packet->ext_size;
}

int rtcp_rr_serialize(const rtcp_rr *packet, uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    const int packet_size = rtcp_rr_size(packet);
    if(size < packet_size)
        return -1;

    if(rtcp_header_serialize(&packet->header, buffer, size) < 0)
        return -1;

    write_u32(buffer + 4, packet->ssrc);

    int offset = 8;
    for(uint8_t i = 0; i < packet->header.common.count; ++i) {
        offset += rtcp_report_serialize(
            &packet->reports[i], buffer + offset, size - offset);
    }

    if(packet->ext_data)
        memcpy(buffer + offset, packet->ext_data, packet->ext_size);

    return packet_size;
}

int rtcp_rr_parse(rtcp_rr *packet, const uint8_t *buffer, int size)
{
    if(!packet || !buffer)
        return -1;

    const int pt = rtcp_header_parse(&packet->header, buffer, size);
    if(pt != RTCP_RR)
        return -1;

    packet->ssrc = read_u32(buffer + 4);

    int offset = 8;
    if(packet->header.common.count) {
        packet->reports = (rtcp_report*)calloc(
            packet->header.common.count, sizeof(rtcp_report));

        for(uint8_t i = 0; i < packet->header.common.count; ++i) {
            rtcp_report *report = &packet->reports[i];
            if(rtcp_report_parse(report, buffer + offset, size - offset) < 0)
                return -1;

            offset += sizeof(rtcp_report);
        }
    }

    const int length = (packet->header.common.length + 1) * 4;
    packet->ext_size = length - offset;

    if(packet->ext_size) {
        packet->ext_data = malloc(packet->ext_size);
        memcpy(packet->ext_data, buffer + offset, packet->ext_size);
    }

    return 0;
}

rtcp_report *rtcp_rr_find_report(rtcp_rr *packet, uint32_t src_id)
{
    if(packet) {
        for(uint8_t i = 0; i < packet->header.common.count; ++i) {
            rtcp_report *report = &packet->reports[i];
            if(report->ssrc == src_id)
                return report;
        }
    }

    return NULL;
}

int rtcp_rr_add_report(rtcp_rr *packet, const rtcp_report *report)
{
    if(!packet || !report)
        return -1;

    if(!packet->header.common.count) {
        packet->reports = (rtcp_report*)malloc(sizeof(rtcp_report));
        packet->header.common.count = 1;
    }
    else {
        if(packet->header.common.count == 0xff)
            return -1;

        if(rtcp_rr_find_report(packet, report->ssrc))
            return -1;

        packet->header.common.count += 1;

        const size_t nmemb = packet->header.common.count * sizeof(rtcp_report);
        packet->reports = (rtcp_report*)realloc(packet->reports, nmemb);
    }

    rtcp_report *dest = &packet->reports[packet->header.common.count - 1];
    memcpy(dest, report, sizeof(rtcp_report));

    // Update header length
    packet->header.common.length = (rtcp_rr_size(packet) / 4) - 1;

    return 0;
}


int rtcp_rr_remove_report(rtcp_rr *packet, uint32_t src_id)
{
    if(!packet)
        return -1;

    rtcp_report *report = rtcp_rr_find_report(packet, src_id);
    if(!report)
        return 0;

    const size_t index = (report - packet->reports) / sizeof(rtcp_report);
    const size_t size = (packet->header.common.count - index) * sizeof(uint32_t);
    if(size)
        memmove(report, report + 1, size);

    packet->header.common.count -= 1;
    if(packet->header.common.count > 0) {
        const size_t nmemb = packet->header.common.count * sizeof(rtcp_report);
        packet->reports = (rtcp_report*)realloc(packet->reports, nmemb);
    }
    else {
        free(packet->reports);
        packet->reports = NULL;
    }

    // Update header length
    packet->header.common.length = (rtcp_rr_size(packet) / 4) - 1;

    return 0;
}

int rtcp_rr_set_ext(rtcp_rr *packet, const void *data, int size)
{
    if(!packet || packet->ext_data || !data)
        return -1;

    if((size % 4) != 0)
        return -1;

    packet->ext_data = malloc(size);
    if(packet->ext_data == NULL)
        return -1;

    packet->ext_size = size;
    memcpy(packet->ext_data, data, size);

    // Update header length
    packet->header.common.length = (rtcp_rr_size(packet) / 4) - 1;

    return 0;
}

int rtcp_rr_clear_ext(rtcp_rr *packet)
{
    if(!packet)
        return -1;

    if(packet->ext_data) {
        free(packet->ext_data);
        packet->ext_data = NULL;
        packet->ext_size = 0;
    }

    // Update header length
    packet->header.common.length = (rtcp_rr_size(packet) / 4) - 1;

    return 0;
}