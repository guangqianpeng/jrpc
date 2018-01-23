add_library(jrpc STATIC
        RpcError.h
        Exception.h
        util.h
        server/BaseServer.cc server/BaseServer.h
        server/RpcServer.cc server/RpcServer.h
        server/RpcService.cc server/RpcService.h
        server/Procedure.cc server/Procedure.h client/BaseClient.cc client/BaseClient.h)

target_link_libraries(jrpc tinyev jackson)

add_subdirectory(stub_generator)