add_custom_command(
        OUTPUT raw_header
        COMMAND jrpcstub
        ARGS -o -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json
        MAIN_DEPENDENCY spec.json
        DEPENDS jrpcstub
        COMMENT "Generating Server/Client Stub..."
        VERBATIM
)

set(stub_dir ${PROJECT_BINARY_DIR}/example/echo)

add_custom_command(
        OUTPUT EchoServiceStub.h EchoClientStub.h
        COMMAND ${CMAKE_STUB_FORMATTER}
        ARGS -i ${stub_dir}/EchoServiceStub.h ${stub_dir}/EchoClientStub.h
        DEPENDS raw_header
        COMMENT "clang format Stub..."
        VERBATIM
)

add_executable(echo_server_ EchoService.cc EchoServiceStub.h)
target_link_libraries(echo_server_ jrpc)
install(TARGETS echo_server_ DESTINATION bin)

add_executable(echo_client_ EchoClient.cc EchoClientStub.h)
target_link_libraries(echo_client_ jrpc)
install(TARGETS echo_client_ DESTINATION bin)