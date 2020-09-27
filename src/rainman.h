#ifndef HYBRIDZIP_MEM_INTERFACE_H
#define HYBRIDZIP_MEM_INTERFACE_H

#include "memmgr.h"

#define R_MALLOC(type, n_elems) this->_rain_man_memmgr_obj->template r_malloc<type>(n_elems)
#define R_NEW(type) this->_rain_man_memmgr_obj->template r_malloc<type>(1)
#define R_FREE(ptr) this->_rain_man_memmgr_obj->template r_free<typeof ptr>(ptr)
#define R_MEM_INIT(child) child._rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj)
#define R_MEM_INIT_PTR(child) child->_rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj)
#define R_MEM_INIT_FROM(mgr, child) child._rain_man_memmgr_attach_memmgr(mgr)
#define R_MEM_INIT_FROM_PTR(mgr, child) child->_rain_man_memmgr_attach_memmgr(mgr)
#define R_MEM_MGR this->_rain_man_memmgr_obj
#define R_MEM_MGR_FROM(obj) obj->_rain_man_memmgr_obj
#define R_CHILD_MGR this->_rain_man_memmgr_obj->create_child_mgr()
#define R_WIPE_MGR this->_rain_man_memmgr_obj->wipe()
#define R_PTR_T(type) rainman::pointer<type>(this->_rain_man_memmgr_obj)
#define R_PTR(ptr) rainman::pointer(ptr, this->_rain_man_memmgr_obj)

namespace rainman {
    // Memory context for using R_MALLOC and R_FREE more idiomatically.
    class context {
    public:
        memmgr *_rain_man_memmgr_obj;

        context() {
            _rain_man_memmgr_obj = nullptr;
        }

        void _rain_man_memmgr_attach_memmgr(memmgr *mgr) {
            _rain_man_memmgr_obj = mgr;
        }
    };

    // Smart-pointer
    template<typename Type>
    class pointer {
    private:
        Type *ptr;
        memmgr *_rain_man_memmgr_obj = nullptr;
        uint64_t *rc = nullptr;
        sem_t *mutex = nullptr;
    public:
        pointer() {
            ptr = new Type;
            rc = new uint64_t;
            mutex = new sem_t;
            sem_init(mutex, 0, 1);
        };

        pointer(memmgr *mgr) {
            _rain_man_memmgr_obj = mgr;
            ptr = R_NEW(Type);
            rc = R_NEW(uint64_t);
            mutex = R_NEW(sem_t);
            sem_init(mutex, 0, 1);
        }

        pointer(Type *ptr) {
            this->ptr = ptr;
            rc = new uint64_t;
            mutex = new sem_t;
            sem_init(mutex, 0, 1);
        }

        pointer(Type *ptr, memmgr *mgr) {
            this->ptr = ptr;
            _rain_man_memmgr_obj = mgr;
            rc = R_NEW(uint64_t);
            mutex = R_NEW(sem_t);
            sem_init(mutex, 0, 1);
        }

        pointer(const pointer<Type> &copy) {
            uint64_t mrc;

            sem_wait(mutex);
            mrc = *rc;
            if (*rc == 0) {
                if (_rain_man_memmgr_obj != nullptr) {
                    R_FREE(ptr);
                    R_FREE(rc);
                } else {
                    delete[] ptr;
                    delete rc;
                }
            } else {
                (*rc)--;
            }

            sem_post(mutex);

            if (!mrc) {
                if (_rain_man_memmgr_obj != nullptr) {
                    R_FREE(mutex);
                } else {
                    delete mutex;
                }
            }

            ptr = copy;
            _rain_man_memmgr_obj = copy._rain_man_memmgr_obj;
            mutex = copy.mutex;
            rc = copy.rc;

            sem_wait(mutex);
            (*rc)++;
            sem_post(mutex);

            return *this;
        }

        pointer &operator=(const pointer<Type> &rvalue) {
            if (this == &rvalue) {
                return *this;
            }

            uint64_t mrc;

            sem_wait(mutex);
            mrc = *rc;
            if (*rc == 0) {
                if (_rain_man_memmgr_obj != nullptr) {
                    R_FREE(ptr);
                    R_FREE(rc);
                } else {
                    delete[] ptr;
                    delete rc;
                }
            } else {
                (*rc)--;
            }

            sem_post(mutex);

            if (!mrc) {
                if (_rain_man_memmgr_obj != nullptr) {
                    R_FREE(mutex);
                } else {
                    delete mutex;
                }
            }

            ptr = rvalue;
            _rain_man_memmgr_obj = rvalue._rain_man_memmgr_obj;
            mutex = rvalue.mutex;
            rc = rvalue.rc;

            sem_wait(mutex);
            (*rc)++;
            sem_post(mutex);

            return *this;
        }

        Type *operator->() {
            return ptr;
        }

        Type &operator*() {
            return *ptr;
        }

        ~pointer() {
            uint64_t mrc;

            sem_wait(mutex);
            mrc = *rc;
            if (*rc == 0) {
                if (_rain_man_memmgr_obj != nullptr) {
                    R_FREE(ptr);
                    R_FREE(rc);
                } else {
                    delete[] ptr;
                    delete rc;
                }
            } else {
                (*rc)--;
            }

            sem_post(mutex);

            if (!mrc) {
                if (_rain_man_memmgr_obj != nullptr) {
                    R_FREE(mutex);
                } else {
                    delete mutex;
                }
            }
        }
    };
}


#endif
