/**
 * @file rtcp_header.c
 * @brief RTCP header.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 */

#include <string.h>

#include "rtp/rtcp_header.h"

int rtcp_header_serialize(const rtcp_header *header, uint8_t *buffer, int size)
{
    if(!header || !buffer || size < 4)
        return -1;

    memcpy(buffer, &header->raw, 4);
    return 4;
}

int rtcp_header_parse(rtcp_header *header, const uint8_t *buffer, int size)
{
    if(!header || !buffer || size < 4)
        return -1;

    memcpy(&header->raw, buffer, 4);
    if(header->common.version != 2)
        return -1;

    return header->common.pt;
}