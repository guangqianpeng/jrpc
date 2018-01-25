add_executable(jrpcstub
        StubGenerator.h
        StubGenerator.cc
        ServiceStubGenerator.h
        ServiceStubGenerator.cc
        ClientStubGenerator.h
        ClientStubGenerator.cc
        main.cc
        )

target_link_libraries(jrpcstub jackson)
install(TARGETS jrpcstub DESTINATION bin)