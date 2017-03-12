#pragma once

#include <map>
#include <vector>
#include <exception>
#include <array>

namespace memcomp
{

namespace constants
{
constexpr size_t Kilobyte = 1024 * sizeof(char);
constexpr size_t Megabyte = 1024 * 1024 * sizeof(char);
} // ns constants

class IPointer
{
public:
    virtual void *getVoidPtr() = 0;
    virtual void resetPointer(char *) = 0;
    virtual size_t getObjSize() = 0;

    virtual ~IPointer() {}
};

template <typename Type>
class Pointer : public IPointer
{
public:
    friend class Allocator;
    ~Pointer();

private:
    Pointer(Type *p);

public:
    Type *get();
    Type *operator ->();

    //from IPointer
    virtual void resetPointer(char *p) override;
    virtual size_t getObjSize() override;
    virtual void *getVoidPtr() override;

    Pointer(const Pointer<Type> &rhs);

private:
    void release();
    void increasePointer(void *p);
    size_t releasePointer(void *p);

private:
    Pointer<Type> operator=(Pointer<Type> &rhs) = delete;

private:
    Type *mPointer;
    bool mReleased;
};

class Allocator
{
public:
    static Allocator & getInst();

    /* Create Pointer<Type> on previus allocated memory.
    */
    template <typename Type, typename... TArgs>
    Pointer<Type> create(TArgs &&... args);

    /* Compress memory: move used bytes to start of memory pool
    */
    void compessMem();

    void registerPointer(void *ptr, IPointer *smartPtr);

    size_t unregisterPointer(void *ptr, IPointer *smartPtr);
    
    ~Allocator();

private:
    //private constructor need for single instance
    Allocator();

private:
    size_t  mBytes;
    char * mPool;
    size_t mShift;
    std::multimap<void *, IPointer *> mPointerMap;
};

} // ns memcomp
