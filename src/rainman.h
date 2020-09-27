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

// Memory interface for using R_MALLOC and R_FREE more idiomatically.
namespace rainman {
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
}



#endif
