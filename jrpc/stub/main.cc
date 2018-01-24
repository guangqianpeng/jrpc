//
// Created by frank on 18-1-24.
//

#include <unistd.h>

#include <jackson/FileReadStream.h>
#include <jackson/Document.h>

#include <jrpc/stub/ServiceStubGenerator.h>
#include <jrpc/stub/ClientStubGenerator.h>
#include <jrpc/Exception.h>

using namespace jrpc;

static void usage()
{
    fprintf(stderr,
            "usage: stub_generator <-c/s> [-o] [-i input]\n");
    exit(1);
}

static void writeToFile(StubGenerator& generator, bool outputToFile)
{
    FILE* output;
    if (!outputToFile) {
        output = stdout;
    }
    else {
        std::string outputFileName = generator.genStubClassName() + ".h";
        output = fopen(outputFileName.c_str(), "w");
        if (output == nullptr) {
            perror("error");
            exit(1);
        }
    }

    auto stubString = generator.genStub();
    fputs(stubString.c_str(), output);
}

static std::unique_ptr<StubGenerator>
makeGenerator(bool serverSide, json::Value& proto)
{
    if (serverSide)
        return std::make_unique<ServiceStubGenerator>(proto);
    else
        return std::make_unique<ClientStubGenerator>(proto);
}

static void genStub(FILE* input, bool serverSide, bool outputToFile)
{
    json::FileReadStream is(input);
    json::Document proto;
    auto err = proto.parseStream(is);
    if (err != json::PARSE_OK) {
        fprintf(stderr, "%s\n", json::parseErrorStr(err));
        exit(1);
    }

    try {
        auto generator = makeGenerator(serverSide, proto);
        writeToFile(*generator, outputToFile);
    }
    catch (StubException& e) {
        fprintf(stderr, "input error: %s\n", e.what());
        exit(1);
    }
}

int main(int argc, char** argv)
{
    bool serverSide = false;
    bool clientSide = false;
    bool outputToFile = false;
    const char* inputFileName = nullptr;

    int opt;
    while ((opt = getopt(argc, argv, "csi:o")) != -1) {
        switch (opt) {
            case 'c':
                clientSide = true;
                break;
            case 's':
                serverSide = true;
                break;
            case 'o':
                outputToFile = true;
                break;
            case 'i':
                inputFileName = optarg;
                break;
            default:
                fprintf(stderr, "unknown flag %c\n", opt);
                usage();
        }
    }
    if (!serverSide && !clientSide) {
        serverSide = clientSide = true;
    }

    FILE* input = stdin;
    if (inputFileName != nullptr) {
        input = fopen(inputFileName, "r");
        if (input == nullptr) {
            perror("error");
            exit(1);
        }
    }

    try {
        if (serverSide) {
            genStub(input, true, outputToFile);
            rewind(input);
        }
        if (clientSide) {
            genStub(input, false, outputToFile);
        }
    }
    catch (StubException& e) {
        fprintf(stderr, "input error: %s\n", e.what());
        exit(1);
    }
}