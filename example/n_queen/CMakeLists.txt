add_custom_command(
        OUTPUT raw_header
        COMMAND jrpcstub
        ARGS -o -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json
        MAIN_DEPENDENCY spec.json
        DEPENDS jrpcstub
        COMMENT "Generating Server/Client Stub..."
        VERBATIM
)

set(stub_dir ${PROJECT_BINARY_DIR}/example/n_queen)

add_custom_command(
        OUTPUT NQueenServiceStub.h NQueenClientStub.h
        COMMAND ${CMAKE_STUB_FORMATTER}
        ARGS -i ${stub_dir}/NQueenServiceStub.h ${stub_dir}/NQueenClientStub.h
        DEPENDS raw_header
        COMMENT "clang format Stub..."
        VERBATIM
)

add_executable(n_queen_server_ NQueenService.cc NQueenServiceStub.h)
target_link_libraries(n_queen_server_ jrpc)
install(TARGETS n_queen_server_ DESTINATION bin)

add_executable(n_queen_client_ NQueenClient.cc NQueenClientStub.h)
target_link_libraries(n_queen_client_ jrpc)
install(TARGETS n_queen_client_ DESTINATION bin)