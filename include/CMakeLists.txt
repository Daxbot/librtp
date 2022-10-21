list(APPEND RTP_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/ntp.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_app.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_bye.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_header.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_report.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_rr.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_sdes.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_sr.h
    ${CMAKE_CURRENT_LIST_DIR}/rtcp_util.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp_header.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/rtp_source.h
    ${CMAKE_CURRENT_LIST_DIR}/version.h)

set(RTP_HEADERS ${RTP_HEADERS} PARENT_SCOPE)
