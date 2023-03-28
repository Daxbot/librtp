/**
 * @file rtcp_fb.h
 * @brief RTCP feedback packet.
 * @author Shijie Zhou
 * @copyright 2022 Daxbot
 * @ingroup fb
 */

 /**
  * @defgroup feedback Feedback packet
  * @brief RTCP Feedback packet description.
  * @ingroup rtcp
  *
  *
  * @verbatim
  *   0               1               2               3
  *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
  *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *  |V=2|P|   fmt   |  PT=205|206   |             length            |
  *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *  |                             SSRC                              |
  *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *  |                     SSRC of media source                      |
  *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  *  |                              FCI                           ...
  *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  * @endverbatim
  */

#ifndef LIBRTP_RTCP_FB_H_
#define LIBRTP_RTCP_FB_H_



#include "rtcp_header.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus



typedef enum rtcp_rtpfb_fmt
{
	RTCP_RTPFB_FMT_NACK=1,
}rtcp_rtpfb_fmt;

typedef enum rtcp_psfb_fmt
{
	RTCP_PSFB_FMT_PLI = 1,
	RTCP_PSFB_FMT_FIR = 4,
}rtcp_psfb_fmt;


typedef struct rtcp_fb
{
	rtcp_header header;        /**< RTCP header. */
	uint32_t ssrc;             /**< Source identifier. */
	uint32_t dst_src;          /**< Dst source identifier. */
	uint8_t* fci;              /**< Feedback Control Information */
	uint16_t fci_size;         /**< FCI data size */
}rtcp_fb;


typedef struct rtcp_psfb_fir_item
{
	uint32_t ssrc;
	uint32_t seq_nr : 8;
	uint32_t reserved : 24;
}rtcp_psfb_fir_item;

/**
 * @brief Allocate a new Feedback packet.
 *
 * @return rtcp_fb_packet*
 */
rtcp_fb* rtcp_fb_create();

/**
 * @brief Initialize an Feedback packet with default values.
 *
 * @param [out] packet - packet to initialize.
 * @param [in] pt - packet payload type.
 * @param [in] fmt - packet fmt.
 */
void rtcp_fb_init(rtcp_fb* packet,uint16_t pt,uint8_t fmt);

/**
 * @brief Free a Feedback packet.
 *
 * @param [out] packet - packet to free.
 */
void rtcp_fb_free(rtcp_fb* packet);

/**
 * @brief Returns the Feedback packet size.
 *
 * @param [in] packet - packet to check.
 * @return packet size in bytes.
 */
size_t rtcp_fb_size(const rtcp_fb* packet);

/**
 * @brief Set the Feedback packet FCI data.
 *
 * Allocates a new buffer and initializes it with the passed in data. If a
 * payload buffer already exists then this method will free it and realloc.
 *
 * @param [out] packet - packet to set on.
 * @param [in] fci - fci data.
 * @param [in] fci_size - fci data size.
 * @return 0 on success.
 */
int rtcp_fb_set_fci(rtcp_fb* packet, const uint8_t* fci, uint16_t fci_size);

/**
 * @brief Clear the FCI data.
 *
 * @param [out] packet - packet to clear on.
 */
void rtcp_fb_free_fci(rtcp_fb* packet);

/**
 * @brief Write an Feedback packet to a buffer.
 *
 * @param [in] packet - packet to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_fb_serialize(const rtcp_fb* packet, uint8_t* buffer, size_t size);

/**
 * @brief Parse an Feedback packet from a buffer.
 *
 * @param [out] packet - empty packet to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_fb_parse(rtcp_fb* packet, const uint8_t* buffer, size_t size);



/**
 * @brief Set the nack parameters in an Feedback packet.
 *
 * @param [out] packet - packet to set.
 * @param [in] fst_pid - the first lost packet id.
 * @param [in] nxt16_pid - bit for the next 16 lost packets.
 * @return 0 on success.
 */
int rtcp_rtpfb_nack_set(rtcp_fb* packet,uint16_t fst_pid, uint16_t nxt16_pid);

/**
 * @brief Get the nack parameters in an Feedback packet.
 *
 * @param [in] packet - packet to set.
 * @param [out] fst_pid - the first lost packet id.
 * @param [out] nxt16_pid - bit for the next 16 lost packets.
 * @return 0 on success.
 */
int rtcp_rtpfb_nack_get(const rtcp_fb* packet, uint16_t* fst_pid, uint16_t* nxt16_pid);


int rtcp_psfb_fir_find_item(const rtcp_fb* packet, uint32_t src, rtcp_psfb_fir_item* item);
int rtcp_psfb_fir_set_items(rtcp_fb* packet, const rtcp_psfb_fir_item* items, size_t count);
int rtcp_psfb_fir_get_item(const rtcp_fb* packet, int idx, rtcp_psfb_fir_item* item);
int rtcp_psfb_fir_item_count(const rtcp_fb* packet);


#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // LIBRTP_RTCP_FB_H_
