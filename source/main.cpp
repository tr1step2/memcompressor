#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <memory>

#include "memcomp.hpp"

struct A 
{
    int a;
    int b;
    int c;

    A(int a = 10, int b = 20, int c = 30)
        : a(a), b(b), c(c)
    {
    }

    void printall()
    {
        std::cout << a << b << c << std::endl;
    }

};

template <typename Type>
void foo(memcomp::Pointer<Type> ptr)
{
    ptr->printall();
    ptr.get();
}

int main(int argc, char **argv)
{
    memcomp::Allocator & alloc = memcomp::Allocator::getInst();

    {
        auto pa1 = alloc.create<A>(1);
        foo(pa1);
    }
    
    auto pa2 = alloc.create<A>(1, 2);
    auto pa3 = alloc.create<A>(1, 2, 3);
    
    pa2->printall();
    pa3->printall();

    alloc.compessMem();

    pa2->printall();
    pa3->printall();
}
