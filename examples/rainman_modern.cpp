#include <rainman/rainman.h>
#include <iostream>

class ModernRainMan1 : public rainman::Allocator {
public:
    void run() {
        int *p = rmalloc<int>(30);
        int *q = rnew<int>();
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

    void run() {
        auto s = rainman::ptr<SomeStruct>(20);
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
        s.set(SomeStruct {10, 20, 12.567, 30}, 0);

        std::cout << "s.x = " << s[0].x << " and s.c = " << (int)s[0].c << " and s.d = " << s[0].d << " and s.y = " << s[0].y << std::endl;
        std::cout << "size of SomeStruct: " << sizeof(SomeStruct) << " bytes" << std::endl;
    }
};

int main() {
    auto example1 = ModernRainMan1();
    auto example2 = ModernRainMan2();
    auto example3 = ModernRainMan3();
    auto example4 = ModernRainMan4();

    example1.run();
    example2.run();
    example3.run();
    example4.run();

    return 0;
}
