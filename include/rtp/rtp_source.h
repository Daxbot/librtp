/**
 * @file rtp_source.h
 * @brief RTP per-source state information.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 * @ingroup rtp
 */

#ifndef LIBRTP_RTP_SOURCE_H_
#define LIBRTP_RTP_SOURCE_H_

#include <stdint.h>

/**
 * @brief The maximum acceptable gap in sequence numbers.
 */
#ifndef LIBRTP_MAX_DROPOUT
#define LIBRTP_MAX_DROPOUT (3000)
#endif

/**
 * @brief The maximum acceptable number of misordered packets.
 */
#ifndef LIBRTP_MAX_MISORDER
#define LIBRTP_MAX_MISORDER (100)
#endif

/**
 * @brief The minimum number of packets to establish a sequence.
 */
#ifndef LIBRTP_MIN_SEQUENTIAL
#define LIBRTP_MIN_SEQUENTIAL (2)
#endif

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

/**
 * @brief Per-source state information.
 */
typedef struct rtp_source {
    uint32_t id;                /**< Source identifier. */
    uint16_t max_seq;           /**< Highest seq. number seen. */
    uint32_t cycles;            /**< Shifted count of seq. number cycles. */
    uint32_t base_seq;          /**< Base seq number. */
    uint32_t bad_seq;           /**< Last 'bad' seq number + 1 */
#if LIBRTP_MIN_SEQUENTIAL > 0
    uint32_t probation;         /**< Sequ. packets till source is valid. */
#endif
    uint32_t received;          /**< Packets received. */
    uint32_t expected_prior;    /**< Packets expected at last interval. */
    uint32_t received_prior;    /**< Packets received at last interval. */
    int transit;                /**< Relative transit time for prev. pkt. */
    double jitter;              /**< Estimated jitter. */
    unsigned int fraction : 8;  /**< Fraction lost since last SR/RR. */
    signed int lost : 24;       /**< Cumulative number of packets lost. */
} rtp_source;

/**
 * @brief Allocate a new source.
 *
 * @return rtp_source*
 */
rtp_source *rtp_source_create();

/**
 * @brief Free a source.
 *
 * @param [out] s - source to free.
 */
void rtp_source_free(rtp_source *s);

/**
 * @brief Initialize a new source.
 *
 * @param [out] s - source to initialize.
 * @param [in] id - source identifier.
 * @param [in] seq - base sequence number.
 */
void rtp_source_init(rtp_source *s, uint32_t id, uint16_t seq);

/**
 * @brief Reset the sequence number for a source.
 *
 * @param [out] s - source to update.
 * @param [in] seq - base sequence number.
 */
void rtp_source_reset_seq(rtp_source *s, uint16_t seq);

/**
 * @brief Update the sequence number for a source.
 *
 * @see IETF RFC3550 "RTP Data Header Validity Checks" (§A.1)
 *
 * @param [in,out] s - source to update.
 * @param [in] seq - new sequence number.
 * @return 0 sequence validation was successful.
 * @return -1 min sequential packets have not yet been received.
 * @return -2 sequence is not valid.
 */
int rtp_source_update_seq(rtp_source *s, uint16_t seq);

/**
 * @brief Update the packet lost count and fraction.
 *
 * @see IETF RFC3550 "Determining Number of Packets Expected and Lost" (§A.3)
 *
 * This should be called immediately before generating a new RTCP report using
 * this source and at most once per RTCP report interval.
 *
 * @param [in,out] s - source to update.
 */
void rtp_source_update_lost(rtp_source *s);

/**
 * @brief Update the estimated jitter.
 *
 * @see IETF RFC3550 "Estimating the Interarrival Jitter" (§A.8)
 *
 * @param [in,out] s - source to update.
 * @param [in] ts - the timestamp from the last rtp packet.
 * @param [in] arrival - the packet arrival time in the same units as ts.
 */
void rtp_source_update_jitter(rtp_source *s, uint32_t ts, uint32_t arrival);

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // LIBRTP_RTP_SOURCE_H_
