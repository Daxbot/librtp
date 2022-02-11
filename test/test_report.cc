#include <gtest/gtest.h>

#include "rtp/rtcp_report.h"

TEST(Report, Size) {
    EXPECT_EQ(sizeof(rtcp_report), 24);
}

TEST(Report, Serialize) {
    rtcp_report report;

    report.ssrc = rand();
    report.fraction = rand() & 0xff;
    report.lost = (rand() & 0xffffff) - 0x7fffff;
    report.last_seq = rand();
    report.jitter = rand();
    report.lsr = rand();
    report.dlsr = rand();

    const size_t size = sizeof(rtcp_report);
    uint8_t *buffer = new uint8_t[size];

    EXPECT_EQ(rtcp_report_serialize(nullptr, buffer, size), -1);
    EXPECT_EQ(rtcp_report_serialize(&report, nullptr, 0), -1);
    EXPECT_EQ(rtcp_report_serialize(&report, buffer, size), size);

    delete[] buffer;
}

TEST(Report, Parse) {
    rtcp_report report;

    report.ssrc = rand();
    report.fraction = rand() & 0xff;
    report.lost = (rand() & 0xffffff) - 0x7fffff;
    report.last_seq = rand();
    report.jitter = rand();
    report.lsr = rand();
    report.dlsr = rand();

    const size_t size = sizeof(rtcp_report);
    uint8_t *buffer = new uint8_t[size];

    rtcp_report_serialize(&report, buffer, size);

    rtcp_report parsed;
    EXPECT_NE(rtcp_report_parse(nullptr, buffer, size), 0);
    EXPECT_NE(rtcp_report_parse(&parsed, nullptr, 0), 0);
    EXPECT_EQ(rtcp_report_parse(&parsed, buffer, size), 0);

    EXPECT_EQ(report.ssrc, parsed.ssrc);
    EXPECT_EQ(report.fraction, parsed.fraction);
    EXPECT_EQ(report.lost, parsed.lost);
    EXPECT_EQ(report.last_seq, parsed.last_seq);
    EXPECT_EQ(report.jitter, parsed.jitter);
    EXPECT_EQ(report.lsr, parsed.lsr);
    EXPECT_EQ(report.dlsr, parsed.dlsr);

    delete[] buffer;
}

TEST(Report, Fraction) {
    rtcp_report report;

    float fraction = 1.0;
    float result;

    EXPECT_EQ(rtcp_report_set_fraction(nullptr, 0), -1);
    EXPECT_NE(rtcp_report_set_fraction(&report, fraction), -1);

    EXPECT_EQ(rtcp_report_get_fraction(nullptr, &result), -1);
    EXPECT_EQ(rtcp_report_get_fraction(&report, nullptr), -1);
    EXPECT_NE(rtcp_report_get_fraction(&report, &result), -1);

    EXPECT_EQ(fraction, result);

    fraction = 0;
    rtcp_report_set_fraction(&report, fraction);
    rtcp_report_get_fraction(&report, &result);

    EXPECT_EQ(fraction, result);
}