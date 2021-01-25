#ifndef RAINMAN_CONTEXT_H
#define RAINMAN_CONTEXT_H

#ifdef RAINMAN_USE_DEPRECATED_API

#include "memmgr.h"

#define rmalloc(type, n_elems) this->_rain_man_memmgr_obj->template r_malloc<type>(n_elems)
#define rnew(type) this->_rain_man_memmgr_obj->template r_malloc<type>(1)
#define rxnew(type, ...) [&]() {                                             \
    auto *t = this->_rain_man_memmgr_obj->template r_malloc<type>(1);           \
    *t = type(__VA_ARGS__);                                                     \
    t->_rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj);              \
    return t;                                                                   \
}()

#define ronew(type, ...) [&]() {                                             \
    auto t = type(__VA_ARGS__);                                                     \
    t._rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj);              \
    return t;                                                                   \
}()

#define rfree(ptr) this->_rain_man_memmgr_obj->template r_free<typeof ptr>(ptr)
#define rinit(child) child._rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj)
#define rinitptr(child) child->_rain_man_memmgr_attach_memmgr(this->_rain_man_memmgr_obj)
#define rinitfrom(mgr, child) child._rain_man_memmgr_attach_memmgr(mgr)
#define rinitptrfrom(mgr, child) child->_rain_man_memmgr_attach_memmgr(mgr)
#define rmemmgr this->_rain_man_memmgr_obj
#define rmemtrace this->_rain_man_memmgr_obj->print_mem_trace()
#define rparentmgr this->_rain_man_memmgr_obj->get_parent()
#define rmemmgrfrom(obj) obj->_rain_man_memmgr_obj
#define rchildmgr this->_rain_man_memmgr_obj->create_child_mgr()
#define rwipe this->_rain_man_memmgr_obj->wipe<void>()
#define rwipeby(type) this->_rain_man_memmgr_obj->wipe<type>()

#define rmod(Class, ...) [this](){                                      \
    auto _instance = Class(__VA_ARGS__);                                \
    auto _childmgr = this->_rain_man_memmgr_obj->create_child_mgr();    \
    _instance._rain_man_memmgr_attach_memmgr(_childmgr);                \
    return _instance;                                                   \
}()

#define rarena(Code) {                                                  \
       class _rainman_safe_scope : public rainman::arena {             \
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

    // Memory-module (A memory leak free class in which all rainman allocations are freed upon destruction)
    class arena : public context {
    protected:
        ~arena() {
            rwipe;
            rmemmgr->unregister();
            delete rmemmgr;
        }
    };
}

#endif
#endif
