add_library(jrpc STATIC
        ConnectionManager.cc ConnectionManager.h
        common/RpcError.h
        common/BufferWriteStream.h
        RpcServer.cc RpcServer.h common/Exception.h RpcService.cc RpcService.h Procedure.cc Procedure.h HelloServerStub.h common/util.h)

target_link_libraries(jrpc tinyev jackson)