#include <rainman/rainman.h>
#include <iostream>

class ModernRainMan1 : public rainman::Allocator {
public:
    void run() {
        int *p = rmalloc<int>(30);
        int *q = rnew<int>(1);
        rfree(p);
        rfree(q);
    }
};

class ModernRainMan2 {
public:
    void run() {
        auto x = rainman::ptr<int>(20);

        x[10] = 10;

        {
            auto y = rainman::ptr<double>(10);
        }
    }
};

class ModernRainMan3 {
public:
    struct SomeStruct {
        int x;
        int y;
    };

    class SomeClass {
    private:
        int _x;
    public:
        SomeClass(int x) : _x(x) {};

        [[nodiscard]] int x() const {
            return _x;
        }

        ~SomeClass() {
            std::cout << "SomeClass destroyed." << std::endl;
        }
    };

    void run() {
        auto s = rainman::ptr<SomeStruct>(20);
        auto z = rainman::ptr<SomeClass>(1, 1);
        auto y = rainman::ptr<SomeStruct>();
        auto c = rainman::ptr<SomeClass>(rainman::Allocator(), 4, 1234);


        {
            auto arr2d = rainman::make_ptr2d<int>(10, 10);
            auto arr3d = rainman::make_ptr3d<int>(10, 10, 10);

            // Initialize objects with args.
            auto obj_arr2d = rainman::make_ptr2d<SomeClass>(10, 10, 16);

            // arr2d[row-idx][col-idx] = value;
            arr2d[5][5] = 10;

            // arr3d[depth-idx][row-idx][col-idx] = value;
            arr3d[5][5][5] = 10;
        }


        std::cout << "c[0].x() = " << c[0].x() << std::endl;


        s->x = 10;
        s->y = 20;

        std::cout << "s.x = " << s->x << " and s.y = " << (*s).y << std::endl;

        ++s;
        s->x = 12;
        s->y = 22;

        std::cout << "s.x = " << s->x << " and s.y = " << (*s).y << std::endl;
        --s;

        std::cout << "s.x = " << s->x << " and s.y = " << (*s).y << std::endl;
    }
};

class ModernRainMan4 {
public:
    struct SomeStruct {
        int x;
        char c;
        double d;
        int y;
    };

    void run() {
        remove("cache.rain");
        auto tmp = fopen("cache.rain", "a");
        fclose(tmp);
        auto fp = fopen("cache.rain", "rb+");
        auto cache = rainman::cache(fp, 0x1);
        auto s = rainman::virtual_array<SomeStruct>(cache, 20);
        s.set(SomeStruct{10, 20, 12.567, 30}, 0);

        std::cout << "s.x = " << s[0].x << " and s.c = " << (int) s[0].c << " and s.d = " << s[0].d << " and s.y = "
                  << s[0].y << std::endl;
        std::cout << "size of SomeStruct: " << sizeof(SomeStruct) << " bytes" << std::endl;
    }
};

class ModernRainMan5 {
public:
    void run() {
        auto data = rainman::ptr<int>(10);
        auto valid_result = rainman::result<rainman::ptr<int>>::ok(data);
        auto invalid_result = rainman::result<rainman::ptr<int>>::err("error occured");

        if (valid_result.is_ok()) {
            std::cout << "Result is valid" << std::endl;
        }

        if (invalid_result.is_err()) {
            std::cout << "Result is invalid" << std::endl;
            std::cout << "Error: " << invalid_result.err() << std::endl;
        }
    }
};

class ModernRainMan6 {
public:
    void run() {
        auto data = rainman::ptr<int>(10);
        auto something = rainman::option<rainman::ptr<int>>(data);
        auto nothing = rainman::option<rainman::ptr<int>>();

        if (something.is_some()) {
            std::cout << "Something" << std::endl;
        }

        if (nothing.is_none()) {
            std::cout << "Nothing" << std::endl;
        }
    }
};

int main() {
    auto example1 = ModernRainMan1();
    auto example2 = ModernRainMan2();
    auto example3 = ModernRainMan3();
    auto example4 = ModernRainMan4();
    auto example5 = ModernRainMan5();
    auto example6 = ModernRainMan6();

    example1.run();
    example2.run();
    example3.run();
    example4.run();
    example5.run();
    example6.run();

    return 0;
}
