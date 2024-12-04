#include "http_util.hpp"

std::string statusCodeString(ResponseCode code) {
    switch (code) {
        case ResponseCode::BAD_REQUEST:
            return "Bad Request";
        case ResponseCode::NOT_FOUND:
            return "Not Found";
        case ResponseCode::OK:
            return "Ok";
        case ResponseCode::METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case ResponseCode::INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        default:
            return "";
    }
}

std::string responseStatusLine(ResponseCode code) {
    std::stringstream statusLine{""};
    statusLine << PROTOCOL_VERSION << " " << std::to_string(static_cast<int>(code)) << " " << statusCodeString(code) << "\n";
    return statusLine.str();
}