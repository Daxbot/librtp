/**
 * @file rtcp_report.h
 * @brief RTCP SR/RR report block.
 * @author Wilkins White
 * @copyright 2022 Daxbot
 * @ingroup rtcp
 */

#ifndef LIBRTP_RTCP_REPORT_H_
#define LIBRTP_RTCP_REPORT_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

/**
 * @brief RTCP SR/RR report.
 */
typedef struct rtcp_report {
    uint32_t ssrc;              /**< Source identifier. */
    unsigned int fraction : 8;  /**< Fraction lost since last SR/RR. */
    signed int lost : 24;       /**< Cumulative number of packets lost. */
    uint32_t last_seq;          /**< Highest sequence number received. */
    uint32_t jitter;            /**< Interarrival jitter. */
    uint32_t lsr;               /**< Last SR. */
    uint32_t dlsr;              /**< Delay since last SR. */
} rtcp_report;

/**
 * @brief Write a report to a buffer.
 *
 * @param [in] report - report to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_report_serialize(const rtcp_report *report, uint8_t *buffer, int size);

/**
 * @brief Parse a report from a buffer.
 *
 * @param [out] report - empty report to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_report_parse(rtcp_report *report, const uint8_t *buffer, int size);

/**
 * @brief Set the percentage of packets lost during the last reporting interval.
 *
 * Calculate percent_lost by dividing the count of received packets, including
 * any late or duplicate packets, by the expected packet count.
 *
 * @see IETF RFC3550 "Determining Number of Packets Expected and Lost" (§A.3)
 *
 * @param [out] report - report to set on.
 * @param [in] percent_lost - percentage of packets lost [0.0 - 1.0].
 * @return 0 on success.
 */
int rtcp_report_set_fraction(rtcp_report *report, float percent_lost);

/**
 * @brief Get the percentage of packets lost during the last reporting interval.
 *
 * @param [in] report - report to get on.
 * @param [out] percent_lost - percentage of packets lost [0.0 - 1.0].
 * @return 0 on success.
 */
int rtcp_report_get_fraction(rtcp_report *report, float *percent_lost);

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // LIBRTP_RTCP_REPORT_H_
