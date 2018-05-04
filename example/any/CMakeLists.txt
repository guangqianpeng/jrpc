add_custom_command(
        OUTPUT raw_header
        COMMAND jrpcstub
        ARGS -o -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json
        MAIN_DEPENDENCY spec.json
        DEPENDS jrpcstub
        COMMENT "Generating Server/Client Stub..."
        VERBATIM
)

set(stub_dir ${PROJECT_BINARY_DIR}/example/any)

add_custom_command(
        OUTPUT AnyServiceStub.h AnyClientStub.h
        COMMAND ${CMAKE_STUB_FORMATTER}
        ARGS ${stub_dir}/AnyServiceStub.h ${stub_dir}/AnyClientStub.h
        DEPENDS raw_header
        COMMENT "clang format Stub..."
        VERBATIM
)

add_executable(any_server AnyService.cc AnyServiceStub.h)
target_link_libraries(any_server jrpc)
install(TARGETS any_server DESTINATION bin)

add_executable(any_client AnyClient.cc AnyClientStub.h)
target_link_libraries(any_client jrpc)
install(TARGETS any_client DESTINATION bin)