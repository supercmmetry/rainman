#include "memmgr.h"
#include "global.h"
#include "errors.h"

/*
 * Context-less wrappers for memory allocation. Uses the rainman global memory manager.
 * These types are not thread-safe.
 */

namespace rainman {

    template<class T>
    class array {
    private:
        T *inner;
        uint64_t n{};
        uint64_t *refs{};
    public:
        array(uint64_t n_elems) {
            inner = rglobalmgr.r_malloc<T>(n_elems);
            refs = rglobalmgr.r_malloc<uint64_t>(1);
            n = n_elems;
        }

        array(T *_inner, uint64_t n_elems) {
            inner = _inner;
            n = n_elems;
            refs = rglobalmgr.r_malloc<uint64_t>(1);
        }

        array(const array<T> &copy) {
            copy.inner = inner;
            copy.n = n;
            copy.refs = refs;

            *refs = *refs + 1;
        }

        T &operator[](uint64_t index) {
            if (index >= n) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return inner[index];
        }

        ~array() {
            if (*refs == 0) {
                rglobalmgr.r_free(inner);
                rglobalmgr.r_free(refs);
            } else {
                *refs = *refs - 1;
            }
        }
    };


    /*
     * virtual_array only supports primitives. Using other types can introduce undefined behaviour.
     */
    template <class T>
    class virtual_array {

    };
}