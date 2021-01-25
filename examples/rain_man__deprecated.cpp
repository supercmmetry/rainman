#include <rainman/rainman.h>
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
            int *another_array = rmalloc(int, 100);
        }
    };


public:
    void run() {
        // Allocate primitives using rainman macros
        int *array = rmalloc(int, 100);

        // Allocate objects using rainman macros
        SubClass *obj = rnew(SubClass);

        // Attach RainMan1's memory manager to 'obj'.
        rinitptr(obj);

        /*
         * If you need to attach a memory manager to a non-pointer type use rinit.
         */

        obj->run();

        // View allocation information

        auto mgr = rmemmgr; // Gets the memory manager in the current context.

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
            int *another_array = rmalloc(int, 100);
            int *some_array = rmalloc(int, 200);

            // You can also free memory like this.
            rfree(some_array);

            char *some_string = rmalloc(char, 100);
            char *some_other_string = rmalloc(char, 100);


            // You can use rwipeby() to delete objects of the same type/class safely.
            // Since, we specify the type, the destructor is also called accordingly.

            // Let's first allocate some objects.
            for (int i = 0; i < 10; i++) {
                auto *obj = rnew(SubClass);
            }

            // Now, we delete objects of type 'SubClass'
            rwipeby(SubClass);

            // You can also use rwipeby to delete primitve pointers of the same type.
            rwipeby(char); // This de-allocates all pointers of type 'char*' allocated in this context.

            // Notice, that this action does not remove the SubClass in which this is being done. "this" was created by
            // a different memory manager, and the objects that we wiped were created by a child memory manager.
        }

        ~SubClass() {
            std::cout << "The destructor was called" << std::endl;
        }
    };


public:
    void run() {
        // Allocate primitives using rainman macros
        int *array = rmalloc(int, 100);

        // Now we run an instance of SubClass in multiple threads.
        // Here, we use separate memory managers for each thread so
        // that we can wipe them safely.

        std::cout << "--- Start example 2 ---" << std::endl;

        std::vector<std::thread *> threads;
        for (int i = 0; i < 10; i++) {
            auto thread = new std::thread(
                    [this, i]() {
                        // Fetch a child memory manager in "this" context.
                        // Note that rchildmgr returns a new instance of a child memory manager from
                        // "this" memory manager.
                        auto child_mgr = rchildmgr;

                        // This creates a SubClass instance under the memory manager offered by 'this'
                        SubClass *obj = rnew(SubClass);

                        // Attach child_mgr to SubClass.
                        rinitptrfrom(child_mgr, obj);

                        // Now all allocations done under SubClass are handled by the child memory manager.
                        // All allocation updates in child_mgr are propagated to it's parent memory manager.

                        obj->run();

                        rfree(obj);
                    }
            );

            threads.push_back(thread);
        }

        // Make sure all threads finish execution.
        for (auto &thread: threads) {
            thread->join();
            delete thread;
        }

        // Now, we view the allocations under the parent memory manager i.e. (rmemmgr).
        std::cout << "Allocation size in bytes: " << rmemmgr->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << rmemmgr->get_alloc_count() << std::endl;

        // Now, we can wipe the memory manager in this context safely because,
        // all threads using a child memory manager have finished execution.

        // As a caveat, you could also use the same memory manager across multiple threads.
        // But, the only necessity for wipe() to be called safely is that all the threads must
        // have finished execution.

        rwipe; // Equivalent to rmemmgr->wipe<void>()
        std::cout << "--- WIPED memory manager ---" << std::endl;


        // Now let's view the allocation stats.
        std::cout << "Allocation size in bytes: " << rmemmgr->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << rmemmgr->get_alloc_count() << std::endl;

        std::cout << "--- End example 2 ---" << std::endl;
    }
};

/*
 * Create a memory-leak free object using rainman modules. All allocations in a rainman module are de-allocated
 * automatically.
 * NOTE: A rainman module cannot have methods that return rainman allocated data. All allocations are de-allocated in
 * the destructor. This means that any pointer-type returned by a member function might lead to segmentation faults
 * unless they are allocated without rainman.
 *
 */

class RainMan3 : public rainman::context {
private:
    class SubClass : public rainman::arena {
    public:
        SubClass(int x) {
            std::cout << "SubClass constructor called with value: " << x << std::endl;
        }

        void run() {
            int *array = rmalloc(int, 100);
            // Since we are using a rainman module, everything that is allocated in this class is automatically
            // de-allocated.
        }
    };

public:
    void run() {
        std::cout << "--- Start example 3 ---" << std::endl;

        // We use the rmod macro to create a rainman module.
        // Format: rmod(Class, constructor parameters)

        // NOTE: A rainman module uses the child memory manager of the current context.
        {
            SubClass myFirstRainmanModule = rmod(SubClass, 10);
            myFirstRainmanModule.run();
        }

        // Notice that we have zero allocations. No more memory leaks!
        std::cout << "Allocation size in bytes: " << rmemmgr->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << rmemmgr->get_alloc_count() << std::endl;
        std::cout << "--- End example 3 ---" << std::endl;
    }
};

/*
 * Run code in a memory-leak free scope.
 */

class RainMan4 : public rainman::context {
public:
    void run() {
        std::cout << "--- Start example 4 ---" << std::endl;

        rarena(
                int *x = rmalloc(int, 3);

                for (int i = 0; i < 3; i++) {
                    x[i] = i * 2;
                    std::cout << "Safe scope: x[" << i << "] = " << x[i] << std::endl;
                }
        )

        // You can use as many leak-free scopes as you want.
        rarena(
                auto *y = rnew(double);
                auto *h = rmalloc(double, 1048576);
                auto *z = rmalloc(RainMan4, 10);

                // For some STL types, rainman fails to de-allocate the object properly without type information.
                // These objects have to be manually destroyed, otherwise SIGABRT is thrown.

                std::string *s = rnew(std::string);
                rfree(s); // or rwipeby(std::string);
        )

        std::cout << "Allocation size in bytes: " << rmemmgr->get_alloc_size() << std::endl;
        std::cout << "Allocation count        : " << rmemmgr->get_alloc_count() << std::endl;
        std::cout << "--- End example 4 ---" << std::endl;
    }
};

int main() {
    auto mgr = new rainman::memmgr;

    auto example1 = RainMan1();
    rinitfrom(mgr, example1);   // Attach 'mgr' to 'example1'
    example1.run();             // Run example1
    mgr->wipe<void>();          // Wipe all allocated memory in example1

    auto example2 = RainMan2();
    rinitfrom(mgr, example2);
    example2.run();
    mgr->wipe<void>();

    auto example3 = RainMan3();
    rinitfrom(mgr, example3);
    example3.run();
    mgr->wipe<void>();

    auto example4 = RainMan4();
    rinitfrom(mgr, example4);
    example4.run();
    mgr->wipe<void>();

    delete mgr;
}
