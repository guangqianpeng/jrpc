add_library(jrpc STATIC
        RpcError.h
        Exception.h
        util.h
        server/BaseServer.cc server/BaseServer.h
        server/RpcServer.cc server/RpcServer.h
        server/RpcService.cc server/RpcService.h
        server/Procedure.cc server/Procedure.h client/BaseClient.cc client/BaseClient.h)
target_link_libraries(jrpc tinyev jackson)
install(TARGETS jrpc DESTINATION lib)

set(HEADERS
        util.h
        server/RpcServer.h
        server/BaseServer.h
        server/Procedure.h
        server/RpcService.h
        client/BaseClient.h)
install(FILES ${HEADERS} DESTINATION include)

add_subdirectory(stub)