#pragma once

#include <map>
#include <vector>
#include <exception>
#include <array>

#include "Pointer.h"

#define KILOBYTE 1024 * sizeof(char)
#define MEGABYTE 1024 * 1024 * sizeof(char)

class Allocator
{
public:
    //returns single instance 
    static Allocator *getInst()
    {
        static Allocator a;
        return &a;
    }

    /* Create Pointer<Type> on previus allocated memory.
    */
    template <typename Type, typename... TArgs>
    Pointer<Type> create(TArgs &&... args)
    {
        if (mShift + sizeof(Type) > mBytes)
            throw std::bad_alloc();

        //placement new - call constructor
        Type *ptr = reinterpret_cast<Type*>(mPool + mShift);
        new (ptr)Type(std::forward<TArgs>(args)...);

        unsigned oldPos = mShift;
        mShift += sizeof(*ptr);

        //create smart Pointer
        Pointer<Type> p(ptr);

        return p;
    }	

    /* Compress memory: move used bytes to start of memory pool
    */
    void compessMem()
    {
        char *newPool = (char *)calloc(1, mBytes);
        unsigned copyIndex = 0;

        for (auto a : mPointerMap)
        {
            void *p = a.second->getVoidPtr();
            unsigned size = a.second->getObjSize();
            
            memcpy(newPool + copyIndex, p, size);
            //a.second->resetPointer(newPool + copyIndex);

            copyIndex += size;
        }

        free(mPool);
        mPool = newPool;
        mShift = copyIndex;
    }

    void registerPointer(void *ptr, IPointer *smartPtr)
    {
        mPointerMap.insert(std::make_pair(ptr, smartPtr));
    }

    unsigned unregisterPointer(void *ptr, IPointer *smartPtr)
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
    
    ~Allocator()
    {
        free(mPool);
    }

private:
    //private constructor need for single instance
    Allocator() 
        : mBytes(100 * KILOBYTE)
        , mPool((char *)calloc(1, mBytes))
        , mShift(0)
    {
    }

private:
    unsigned mBytes; 
    char *mPool;
    unsigned mShift;

    //usigned - index in pool
    //ptr - pointer to smart pointer to update 
    std::multimap<void *, IPointer *> mPointerMap;
};
