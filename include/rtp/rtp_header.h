/**
 * @file rtp_header.h
 * @brief RTP header.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 * @ingroup rtp
 */

/**
 * @defgroup rtp RTP
 * @brief Real-time transport protocol.
 * @see IETF RFC3550 "RTP Fixed Header Fields" (§5.1)
 *
 * @verbatim
 *   0               1               2               3
 *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           synchronization source (SSRC) identifier            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |            contributing source (CSRC) identifiers             |
 *  |                              ...                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

#ifndef LIBRTP_RTP_HEADER_H_
#define LIBRTP_RTP_HEADER_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

/**
 * @brief RTP packet.
 */
typedef struct rtp_header {
    // Required
    unsigned int version : 2;   /**< protocol version */
    unsigned int p : 1;         /**< padding flag */
    unsigned int x : 1;         /**< Header extension flag */
    unsigned int cc : 4;        /**< CSRC count */
    unsigned int m : 1;         /**< Marker bit */
    unsigned int pt : 7;        /**< Payload type */
    unsigned int seq : 16;      /**< Sequence number */
    uint32_t ts;                /**< Timestamp */
    uint32_t ssrc;              /**< Synchronization source */

    // Optional
    uint32_t *csrc;             /**< List of contributing sources. */

    uint16_t ext_id;            /**< Extension ID. */
    uint16_t ext_count;         /**< Number of extension entries. */
    uint32_t *ext_data;         /**< Extension data. */
} rtp_header;

/**
 * @brief Allocate a new header.
 *
 * @return header.
 */
rtp_header *rtp_header_create();

/**
 * @brief Free a header.
 *
 * @param [out] header - header to free.
 */
void rtp_header_free(rtp_header *header);

/**
 * @brief Initialize a header with default values.
 *
 * @param [out] header - header to initialize.
 * @param [in] pt - payload type.
 * @return 0 on success.
 */
int rtp_header_init(rtp_header *header, uint8_t pt);

/**
 * @brief Returns the header size.
 *
 * @param [in] header - header to check.
 * @return header size in bytes.
 */
int rtp_header_size(const rtp_header *header);

/**
 * @brief Write an header to a buffer.
 *
 * @param [in] header - header to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtp_header_serialize(const rtp_header *header, uint8_t *buffer, int size);

/**
 * @brief Fill a header from a buffer.
 *
 * @param [out] header - empty header to fill.
 * @param [in] buffer - buffer to read from.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtp_header_parse(rtp_header *header, const uint8_t *buffer, int size);

/**
 * @brief Return the index of a csrc.
 *
 * @param [in] header - header to search.
 * @param [in] csrc - csrc to find.
 * @return csrc index or -1 on failure.
 */
int rtp_header_find_csrc(rtp_header *header, uint32_t csrc);

/**
 * @brief Add a contributing source id.
 *
 * @param [out] header - header to add to.
 * @param [in] csrc - id of the csrc to add.
 * @return 0 on success.
 */
int rtp_header_add_csrc(rtp_header *header, uint32_t csrc);

/**
 * @brief Remove a contributing source id.
 *
 * @param [out] header - header to remove from.
 * @param [in] csrc - id of the csrc to remove.
 * @return 0 on success.
 */
int rtp_header_remove_csrc(rtp_header *header, uint32_t csrc);

/**
 * @brief Set the header extension.
 *
 * @param [out] header - header to set on.
 * @param [in] id - extension id.
 * @param [in] data - extension data.
 * @param [in] size - extension data size.
 * @return 0 on success.
 */
int rtp_header_set_ext(
    rtp_header *header, uint16_t id, const uint32_t *data, int size);

/**
 * @brief Clear the header extension.
 *
 * @param [out] header - header to clear on.
 * @return 0 on success.
 */
int rtp_header_clear_ext(rtp_header *header);

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // LIBRTP_RTP_HEADER_H_
