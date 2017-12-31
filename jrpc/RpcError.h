//
// Created by frank on 17-12-29.
//

#ifndef JRPC_ERRORCODE_H
#define JRPC_ERRORCODE_H

#include <cassert>

namespace jrpc
{


#define ERROR_MAP(XX) \
  XX(PARSE_ERROR, -32700, "Parse error") \
  XX(INVALID_REQUEST, -32600, "Invalid request") \
  XX(METHOD_NOT_FOUND, -32601,"Method not found") \
  XX(INVALID_PARAMS, -32602, "Invalid params") \
  XX(INTERNAL_ERROR, -32603, "Internal error") \

enum Error
{
#define GEN_ERRNO(e, c, s) RPC_##e,
    ERROR_MAP(GEN_ERRNO)
#undef GEN_ERRNO
};

class RpcError
{
public:
    // implicit conversion is OK
    RpcError(Error err):
            err_(err)
    {}

    explicit RpcError(int32_t errorCode):
            err_(fromErrorCode(errorCode))
    {}

    const char* asString() const
    { return errorString[err_]; }

    int32_t asCode() const
    { return errorCode[err_]; }

private:
    const Error err_;

    static Error fromErrorCode(int32_t code)
    {
        switch (code) {
            case -32700: return RPC_PARSE_ERROR;
            case -32600: return RPC_INVALID_REQUEST;
            case -32601: return RPC_METHOD_NOT_FOUND;
            case -32602: return RPC_INVALID_PARAMS;
            case -32603: return RPC_INTERNAL_ERROR;
            default: assert(false && "bad error code");
        }
    }

    static const int32_t errorCode[];
    static const char* errorString[];
};

inline const int32_t RpcError::errorCode[] = {
#define GEN_ERROR_CODE(e, c, n) c,
        ERROR_MAP(GEN_ERROR_CODE)
#undef GEN_ERROR_CODE
};

inline const char* RpcError::errorString[] = {
#define GEN_STRERR(e, c, n) n,
        ERROR_MAP(GEN_STRERR)
#undef GEN_STRERR
};

#undef ERROR_MAP


}


#endif //JRPC_ERRORCODE_H
