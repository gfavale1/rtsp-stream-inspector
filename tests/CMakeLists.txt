add_executable(rtsi_tests
    test_rtsp_url.cpp
    test_rtsp_request.cpp
    test_rtsp_response.cpp
    test_rtsp_session.cpp
    test_rtsp_auth.cpp
    test_sdp_parser.cpp
    test_anomaly_detector.cpp
    test_h264_nal.cpp
    test_report_writer.cpp
    test_rtp_parser.cpp
    test_rtp_stats.cpp
    test_rtcp_parser.cpp
    test_rtcp_stats.cpp
    test_stream_metrics_tests.cpp
    test_jitter_estimator.cpp
)

target_link_libraries(rtsi_tests
    PRIVATE
        rtsi_core
        Catch2::Catch2WithMain
)

add_test(NAME rtsi_tests COMMAND rtsi_tests)