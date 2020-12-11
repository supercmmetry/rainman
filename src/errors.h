#ifndef RAINMAN_ERRORS_H
#define RAINMAN_ERRORS_H

#include <exception>
#include <string>

namespace MemoryErrors {
    class InvalidOperationException : public std::exception {
    private:
        std::string msg;
    public:
        InvalidOperationException(const std::string &msg) {
            this->msg = "rainman: " + msg;
        }

        [[nodiscard]] const char *what() const noexcept override {
            return msg.c_str();
        }
    };

    class PeakLimitReachedException : public std::exception {
    public:
        PeakLimitReachedException() = default;

        [[nodiscard]] const char *what() const noexcept override {
            return "rainman: Peak limit reached";
        }
    };

    class SegmentationFaultException : public std::exception {
    public:
        SegmentationFaultException() = default;

        [[nodiscard]] const char *what() const noexcept override {
            return "rainman: Segmentation fault";
        }
    };
}


#endif
