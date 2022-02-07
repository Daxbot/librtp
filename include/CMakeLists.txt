list(APPEND RTP_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtcp_app.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtcp_bye.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtcp_report.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtcp_rr.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtcp_sdes.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtcp_sr.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtp_header.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/rtp_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp/version.h)

set(RTP_HEADERS ${RTP_HEADERS} PARENT_SCOPE)
