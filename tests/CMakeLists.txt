add_executable(rtsi_tests
    test_rtsp_url.cpp
)

target_link_libraries(rtsi_tests
    PRIVATE
        rtsi_core
        Catch2::Catch2WithMain
)

add_test(NAME rtsi_tests COMMAND rtsi_tests)