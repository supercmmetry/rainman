#ifndef RAINMAN_RAINMAN_H
#define RAINMAN_RAINMAN_H

#include "memmgr.h"

#define rmalloc(type, n_elems) this->_rain_man_memmgr_obj->template r_malloc<type>(n_elems)
#define rnew(type) this->_rain_man_memmgr_obj->template r_malloc<type>(1)
#define rxnew(type, ...) [&]() {                                             \
    auto *t = this->_rain_man_memmgr_obj->template r_malloc<type>(1);           \
    *t = type(__VA_ARGS__);                                                     \
    t->_rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj);              \
    return t;                                                                   \
}()

#define rfree(ptr) this->_rain_man_memmgr_obj->template r_free<typeof ptr>(ptr)
#define rinit(child) child._rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj)
#define rinitptr(child) child->_rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj)
#define rinitfrom(mgr, child) child._rain_man_memmgr_attach_memmgr(mgr)
#define rinitptrfrom(mgr, child) child->_rain_man_memmgr_attach_memmgr(mgr)
#define rmemmgr this->_rain_man_memmgr_obj
#define rmemmgrfrom(obj) obj->_rain_man_memmgr_obj
#define rchildmgr this->_rain_man_memmgr_obj->create_child_mgr()
#define rwipe this->_rain_man_memmgr_obj->wipe<void>()
#define rwipeby(type) this->_rain_man_memmgr_obj->wipe<type>()
#define rainptr_t(type) rainman::pointer<type>(this->_rain_man_memmgr_obj)
#define rainptr(ptr) rainman::pointer(ptr, this->_rain_man_memmgr_obj)
#define rainptr_m(type, n_elems) rainman::pointer(this->_rain_man_memmgr_obj->template r_malloc<type>(n_elems), this->_rain_man_memmgr_obj)

#define rmod(Class, ...) [this](){                                      \
    auto _instance = Class(__VA_ARGS__);                                \
    auto _childmgr = this->_rain_man_memmgr_obj->create_child_mgr();    \
    _instance._rain_man_memmgr_attach_memmgr(_childmgr);                \
    return _instance;                                                   \
}()

#define rscope(Code) {                                                  \
       class _rainman_safe_scope : public rainman::module {             \
       public:                                                          \
            void run() {                                                \
                Code                                                    \
            }                                                           \
       };                                                               \
                                                                        \
       auto _instance = _rainman_safe_scope();                          \
       auto _childmgr = this->_rain_man_memmgr_obj->create_child_mgr(); \
       _instance._rain_man_memmgr_attach_memmgr(_childmgr);             \
       _instance.run();                                                 \
}


namespace rainman {
    // Memory context for using rmalloc and rfree more idiomatically.
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
            *rc = 0;
            mutex = new sem_t;
            sem_init(mutex, 0, 1);
        };

        pointer(memmgr *mgr) {
            _rain_man_memmgr_obj = mgr;
            ptr = rnew(Type);
            rc = rnew(uint64_t);
            *rc = 0;
            mutex = rnew(sem_t);
            sem_init(mutex, 0, 1);
        }

        pointer(Type *ptr) {
            this->ptr = ptr;
            rc = new uint64_t;
            *rc = 0;
            mutex = new sem_t;
            sem_init(mutex, 0, 1);
        }

        pointer(Type *ptr, memmgr *mgr) {
            this->ptr = ptr;
            _rain_man_memmgr_obj = mgr;
            rc = rnew(uint64_t);
            *rc = 0;
            mutex = rnew(sem_t);
            sem_init(mutex, 0, 1);
        }

        pointer(const pointer<Type> &copy) {
            if (mutex != nullptr) {
                uint64_t mrc;

                sem_wait(mutex);
                mrc = *rc;
                if (*rc == 0) {
                    if (_rain_man_memmgr_obj != nullptr) {
                        rfree(ptr);
                        rfree(rc);
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
                        rfree(mutex);
                    } else {
                        delete mutex;
                    }
                }
            }

            ptr = copy.ptr;
            _rain_man_memmgr_obj = copy._rain_man_memmgr_obj;
            mutex = copy.mutex;
            rc = copy.rc;

            sem_wait(mutex);
            (*rc)++;
            sem_post(mutex);
        }

        pointer &operator=(const pointer<Type> &rvalue) {
            if (this == &rvalue) {
                return *this;
            }

            if (mutex != nullptr) {
                uint64_t mrc;

                sem_wait(mutex);
                mrc = *rc;
                if (*rc == 0) {
                    if (_rain_man_memmgr_obj != nullptr) {
                        rfree(ptr);
                        rfree(rc);
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
                        rfree(mutex);
                    } else {
                        delete mutex;
                    }
                }
            }

            ptr = rvalue.ptr;
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

        Type &operator[](int i) {
            return ptr[i];
        }

        ~pointer() {
            uint64_t mrc;

            sem_wait(mutex);
            mrc = *rc;
            if (*rc == 0) {
                if (_rain_man_memmgr_obj != nullptr) {
                    rfree(ptr);
                    rfree(rc);
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
                    rfree(mutex);
                } else {
                    delete mutex;
                }
            }
        }
    };

    // Memory-module (A memory leak free class in which all rainman allocations are freed upon destruction)
    class module : public context {
    protected:
        ~module() {
            rwipe;
            rmemmgr->unregister();
            delete rmemmgr;
        }
    };
}


#endif
