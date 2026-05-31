#ifndef AIZO_PROJECT2_DATETIME_H
#define AIZO_PROJECT2_DATETIME_H

#include <ctime>
#include <string>

inline std::string getCurrentDateTime() {
    const std::time_t now = std::time(nullptr);
    char buffer[32];

    std::strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d %H:%M:%S",
        std::localtime(&now)
    );

    return std::string(buffer);
}

#endif //AIZO_PROJECT2_DATETIME_H
