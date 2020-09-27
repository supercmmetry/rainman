#ifndef RAIN_MAN_ERRORS_H
#define RAIN_MAN_ERRORS_H
#ifndef HYBRIDZIP_ERRORS_MEMORY_H
#define HYBRIDZIP_ERRORS_MEMORY_H

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

#endif
