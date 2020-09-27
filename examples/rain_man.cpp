#include <rainman.h>
#include <iostream>
#include <vector>
#include <thread>

/*
 * Attach a rainman interface to a class.
 * Create other objects under the same memory manager.
 * View allocation information.
 */

class RainMan1 : public rainman::context {
private:
    class SubClass : public rainman::context {
    public:
        void run() {
            int *another_array = R_MALLOC(int, 100);
        }
    };


public:
    void run() {
        // Allocate primitives using rainman macros
        int *array = R_MALLOC(int, 100);

        // Allocate objects using rainman macros
        SubClass *obj = R_NEW(SubClass);

        // Attach RainMan1's memory manager to 'obj'.
        R_MEM_INIT_PTR(obj);

        /*
         * If you need to attach a memory manager to a non-pointer type use R_MEM_INIT.
         */

        obj->run();

        // View allocation information

        auto mgr = R_MEM_MGR; // Gets the memory manager in the current context.

        std::cout << "--- Start example 1 ---" << std::endl;
        std::cout << "Allocation size in bytes: " << mgr->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << mgr->get_alloc_count() << std::endl;
        std::cout << "--- End example 1 ---" << std::endl;
    }
};

/*
 * Manage child memory managers across multiple threads.
 * De-allocate everything that was allocated in each thread.
 *
 * NOTE: If you intend to wipe a memory manager, make sure that the threads using
 * the memory manager or it's children have completed their execution, otherwise this
 * might lead to undefined behaviour.
 * Furthermore, the wipe() method does not call the destructor of allocated objects. This
 * will not lead to memory leaks if all allocations in the objects are done through rainman.
 */

class RainMan2 : public rainman::context {
private:
    class SubClass : public rainman::context {
    public:
        void run() {
            int *another_array = R_MALLOC(int, 100);
        }

        ~SubClass() {
            std::cout << "The destructor was called" << std::endl;
        }
    };


public:
    void run() {
        // Allocate primitives using rainman macros
        int *array = R_MALLOC(int, 100);

        // Now we run an instance of SubClass in multiple threads.
        // Here, we use separate memory managers for each thread so
        // that we can wipe them safely.

        std::vector<std::thread *> threads;
        for (int i = 0; i < 10; i++) {
            auto thread = new std::thread(
                    [this]() {
                        // Fetch a child memory manager in "this" context.
                        // Note that R_CHILD_MGR returns a new instance of a child memory manager from
                        // "this" memory manager.
                        auto child_mgr = R_CHILD_MGR;

                        // This creates a SubClass instance under the memory manager offered by 'this'
                        SubClass *obj = R_NEW(SubClass);

                        // Attach child_mgr to SubClass.
                        R_MEM_INIT_FROM_PTR(child_mgr, obj);

                        // Now all allocations done under SubClass are handled by the child memory manager.
                        // All allocation updates in child_mgr are propagated to it's parent memory manager.

                        obj->run();

                        R_FREE(obj);
                    }
            );

            threads.push_back(thread);
        }

        // Make sure all threads finish execution.
        for (auto & thread: threads) {
            thread->join();
        }

        // Now, we view the allocations under the parent memory manager i.e. (R_MEM_MGR).
        std::cout << "--- Start example 2 ---" << std::endl;
        std::cout << "Allocation size in bytes: " << R_MEM_MGR->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << R_MEM_MGR->get_alloc_count() << std::endl;

        // Now, we can wipe the memory manager in this context safely because,
        // all threads using a child memory manager have finished execution.

        // As a caveat, you could also use the same memory manager across multiple threads.
        // But, the only necessity for wipe() to be called safely is that all the threads must
        // have finished execution.

        R_WIPE_MGR; // Equivalent to R_MEM_MGR->wipe()
        std::cout << "--- WIPED memory manager ---" << std::endl;


        // Now let's view the allocation stats.
        std::cout << "Allocation size in bytes: " << R_MEM_MGR->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << R_MEM_MGR->get_alloc_count() << std::endl;

        std::cout << "--- End example 2 ---" << std::endl;
    }
};

int main() {
    auto mgr = new rainman::memmgr;
    auto example1 = RainMan1();

    // Attach 'mgr' to 'example1'
    R_MEM_INIT_FROM(mgr, example1);

    // Run example1
    example1.run();

    // Wipe all allocated memory in example1
    mgr->wipe();

    auto example2 = RainMan2();

    // Attach 'mgr' to 'example2'
    R_MEM_INIT_FROM(mgr, example2);

    // Run example2
    example2.run();

    // Wipe all allocated memory in example2
    mgr->wipe();
}
