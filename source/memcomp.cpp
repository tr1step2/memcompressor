#include <cstring>
#include "memcomp.hpp"

template <typename Type>
memcomp::Pointer<Type>::~Pointer()
{
    release();
}

template <typename Type>
memcomp::Pointer<Type>::Pointer(Type *p)
    : mPointer(p)
    , mReleased(false)
{
    increasePointer(mPointer);
}

template <typename Type>
Type * memcomp::Pointer<Type>::get()
{
    return mPointer;
}

template <typename Type>
Type * memcomp::Pointer<Type>::operator ->()
{
    return mPointer;
}

template <typename Type>
void memcomp::Pointer<Type>::resetPointer(char * p)
{
    mPointer = (Type *)p;
}

template <typename Type>
size_t memcomp::Pointer<Type>::getObjSize()
{
    return sizeof(Type);
}

template <typename Type>
void * memcomp::Pointer<Type>::getVoidPtr()
{
    return (void *)mPointer;
}

template <typename Type>
memcomp::Pointer<Type>::Pointer(const Pointer<Type> & rhs)
{
    mPointer = rhs.mPointer;
    mReleased = rhs.mReleased;

    increasePointer(mPointer);
}

template <typename Type>
void memcomp::Pointer<Type>::release()
{
    if (mReleased)
        return;

    if (!releasePointer(mPointer))
    {
        mPointer->~Type();
        mReleased = true;
        mPointer = nullptr;
    }
}

template <typename Type>
void memcomp::Pointer<Type>::increasePointer(void * p)
{
    if (!p)
        return;

    Allocator::getInst().registerPointer(p, this);
}

template <typename Type>
size_t memcomp::Pointer<Type>::releasePointer(void * p)
{
    if (!p)
        return 0;

    return Allocator::getInst().unregisterPointer(p, this);
}

memcomp::Allocator &memcomp::Allocator::getInst()
{
    static Allocator a;
    return a;
}

/* Create Pointer<Type> on previus allocated memory.
*/
template <typename Type, typename... TArgs>
memcomp::Pointer<Type> memcomp::Allocator::create(TArgs &&... args)
{
    if (mShift + sizeof(Type) > mBytes)
        throw std::bad_alloc();

    //placement new - call constructor
    Type *ptr = reinterpret_cast<Type*>(mPool + mShift);
    new (ptr)Type(std::forward<TArgs>(args)...);

    size_t oldPos = mShift;
    mShift += sizeof(*ptr);

    //create smart Pointer
    Pointer<Type> p(ptr);

    return p;
}

/* Compress memory: move used bytes to start of memory pool
*/
void memcomp::Allocator::compessMem()
{
    char *newPool = (char *)calloc(1, mBytes);
    size_t copyIndex = 0;

    for (auto a : mPointerMap)
    {
        void *p = a.second->getVoidPtr();
        size_t size = a.second->getObjSize();

        std::memcpy(newPool + copyIndex, p, size);
        //a.second->resetPointer(newPool + copyIndex);

        copyIndex += size;
    }

    free(mPool);
    mPool = newPool;
    mShift = copyIndex;
}

void memcomp::Allocator::registerPointer(void *ptr, IPointer *smartPtr)
{
    mPointerMap.insert(std::make_pair(ptr, smartPtr));
}

size_t memcomp::Allocator::unregisterPointer(void * ptr, IPointer * smartPtr)
{
    auto it = mPointerMap.find(ptr);
    do
    {
        bool find = it->second == smartPtr;
        if (find)
        {
            mPointerMap.erase(it);
            break;
        }
        ++it;

    } while (it->first == ptr);

    return mPointerMap.count(ptr);
}

memcomp::Allocator::~Allocator()
{
    free(mPool);
}

memcomp::Allocator::Allocator()
    : mBytes(100 * memcomp::constants::Kilobyte)
    , mPool((char *)calloc(1, mBytes))
    , mShift(0)
{
}
