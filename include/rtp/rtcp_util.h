/**
 * @file rtcp.h
 * @brief RTCP utility functions.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 */

#ifndef LIBRTP_RTCP_UTIL_H_
#define LIBRTP_RTCP_UTIL_H_

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

/**
 * @brief Returns the RTCP packet type
 *
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return packet type or -1 on failure.
 */
int rtcp_type(const uint8_t *buffer, int size);

/**
 * @brief Calculates the RTCP transmission interval in seconds.
 *
 * @see IETF RFC3550 "Computing the RTCP Transmission Interval" (§A.7)
 *
 * @param [in] members - The current estimate for the number of session members.
 * @param [in] senders - The current estimate for the number of session senders.
 * @param [in] rtcp_bw - The target RTCP bandwidth, i.e. the total bandwidth
 *  that will be used by all members of this session (in bits/s).
 * @param [in] we_sent - True if the application has sent data since the 2nd
 *  previous RTCP report was transmitted.
 * @param [in] avg_rtcp_size - The average compound RTCP packet size, in octets,
 *  over all RTCP packets sent and received by this participant. The size
 *  includes lower-layer transport and network protocol headers.
 * @param [in] initial - True if the application has not sent an RTCP packet.
 * @return Deterministic calculated interval in seconds.
 */
double rtcp_interval(
    int members,
    int senders,
    double rtcp_bw,
    bool we_sent,
    double avg_rtcp_size,
    bool initial);

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // LIBRTP_RTCP_UTIL_H_