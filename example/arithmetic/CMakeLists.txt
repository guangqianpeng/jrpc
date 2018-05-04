add_custom_command(
        OUTPUT RAW_HEADER
        COMMAND jrpcstub
        ARGS -o -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json
        MAIN_DEPENDENCY spec.json
        DEPENDS jrpcstub
        COMMENT "Generating Server/Client Stub..."
        VERBATIM
)

set(stub_dir ${PROJECT_BINARY_DIR}/example/arithmetic)

add_custom_command(
        OUTPUT HEADER
        COMMAND ${CMAKE_STUB_FORMATTER}
        ARGS -i ${stub_dir}/ArithmeticServiceStub.h ${stub_dir}/ArithmeticClientStub.h
        DEPENDS RAW_HEADER
        COMMENT "clang format Stub..."
        VERBATIM
)

add_executable(arithmetic_server ArithmeticService.cc HEADER)
target_link_libraries(arithmetic_server jrpc)
install(TARGETS arithmetic_server DESTINATION bin)

add_executable(arithmetic_client ArithmeticClient.cc HEADER)
target_link_libraries(arithmetic_client jrpc)
install(TARGETS arithmetic_client DESTINATION bin)