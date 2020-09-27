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
            int *some_array = R_MALLOC(int, 200);

            // You can also free memory like this.
            R_FREE(some_array);
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

        std::cout << "--- Start example 2 ---" << std::endl;

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

                        // Since, the wipe() method does not call the destructor of the object,
                        // we use R_FREE() to free any memory allocated for 'obj' and also call it's destructor.
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

class RainMan3 : public rainman::context {
private:
    class SubClass : public rainman::context {
    public:
        void run() {
            int *another_array = R_MALLOC(int, 100);

            // We wrap 'another_array' in a rainman smart pointer.
            // This automatically de-allocates 'another_array' when smart_ptr
            // goes out of scope.
            // The memory manager is automatically attached.

            auto sptr = rainptr(another_array);

            // If you are not inheriting rainman::context, then you can create a smart pointer like this.
            // But the memory manager needs to be attached manually.
            auto another_sptr = rainman::pointer<int>(R_MEM_MGR);

            // NOTE: It is unsafe to create more than one smart pointer wrapping a single pointer. This breaks the
            // reference-counting mechanism and can lead to unnecessary de-allocations leading to a classic
            // segmentation fault. To create more smart pointers wrapping the same pointer,
            // please use the copy constructor.

            // Also supports reference-counting.
            {
                auto dptr = sptr;
            }

            // Create an array pointer wrapped with smart pointer in one line.

            sptr = rainptr_m(int, 10);

            // Operator overload for subscripting
            sptr[0] = 100;

            // In addition to this, you can also create smart pointers directly from types.

            auto number = rainptr_t(double);

            *number = 123.456;

            std::cout << "Number: " << *number << std::endl;

            // Last but not the least, these smart-pointers are thread-safe.
            // However, this does not mean that the underlying pointer is thread-safe!

            std::vector<std::thread*> threads;

            for (int i = 0; i < 10; i++) {
                auto thread = new std::thread([&sptr, i]() {
                   sptr[i] = i;
                });

                threads.push_back(thread);
            }

            for (auto &thread : threads) {
                thread->join();
            }

            for (int i = 0; i < 10; i++) {
                std::cout << "sptr[" << i << "] = " << sptr[i] << std::endl;
            }

        }
    };


public:
    void run() {
        // Allocate objects using rainman macros
        SubClass *obj = R_NEW(SubClass);

        std::cout << "--- Start example 3 ---" << std::endl;

        // Attach RainMan1's memory manager to 'obj'.
        R_MEM_INIT_PTR(obj);

        /*
         * If you need to attach a memory manager to a non-pointer type use R_MEM_INIT.
         */

        obj->run();
        R_FREE(obj);

        // View allocation information
        // Notice that the stuff allocated inside obj->run() was de-allocated without
        // any manual intervention.

        auto mgr = R_MEM_MGR; // Gets the memory manager in the current context.


        std::cout << "Allocation size in bytes: " << mgr->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << mgr->get_alloc_count() << std::endl;
        std::cout << "--- End example 3 ---" << std::endl;
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

    auto example3 = RainMan3();

    // Attach 'mgr' to 'example3'
    R_MEM_INIT_FROM(mgr, example3);

    // Run example3
    example3.run();

    // Wipe all allocated memory in example3
    mgr->wipe();
}
