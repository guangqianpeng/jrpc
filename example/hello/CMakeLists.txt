add_custom_command(
        OUTPUT raw_header
        COMMAND jrpcstub
        ARGS -o -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json
        MAIN_DEPENDENCY spec.json
        DEPENDS jrpcstub
        COMMENT "Generating Server/Client Stub..."
        VERBATIM
)

set(stub_dir ${PROJECT_BINARY_DIR}/example/hello)

add_custom_command(
        OUTPUT HelloServiceStub.h HelloClientStub.h
        COMMAND clang-format
        ARGS -i ${stub_dir}/HelloServiceStub.h ${stub_dir}/HelloClientStub.h
        DEPENDS raw_header
        COMMENT "clang format Stub..."
        VERBATIM
)

add_executable(hello_server HelloServer.cc HelloServiceStub.h)
target_link_libraries(hello_server jrpc)

add_executable(hello_client HelloClient.cc HelloClientStub.h)
target_link_libraries(hello_client jrpc)