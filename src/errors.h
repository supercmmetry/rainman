#ifndef RAINMAN_ERRORS_H
#define RAINMAN_ERRORS_H

#include <exception>
#include <string>

namespace MemoryErrors {
    class PeakLimitReachedException: public std::exception {
    public:
        PeakLimitReachedException() = default;

        [[nodiscard]] const char *what() const noexcept override {
            return "rain-man: peak limit reached";
        }
    };
}


#endif
