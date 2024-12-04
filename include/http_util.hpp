#include <sstream>

#define PROTOCOL_VERSION "HTTP/1.1"

/// @brief Response status codes
enum class ResponseCode {
    BAD_REQUEST = 400,
    OK = 200,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_SERVER_ERROR = 500,
};

/// @brief Converts a response code to a displayable string
/// @param code response code
/// @return code converted to string
std::string statusCodeString(ResponseCode code);

/// @brief Builds a HTTP status line from a given response code
/// @param code response code
/// @return formatted status line
std::string responseStatusLine(ResponseCode code);