#pragma once

#include "Allocator.h"

class IPointer
{
public:
    virtual void *getVoidPtr() = 0;
    virtual void resetPointer(char *) = 0;
    virtual unsigned getObjSize() = 0;

    virtual ~IPointer() {}
};

template <typename Type>
class Pointer : public IPointer
{
public:
    friend class Allocator;

    ~Pointer() 
    {
        release();
    }

private:
    Pointer(Type *p) : mPointer(p), mReleased(false)
    {
        increasePointer(mPointer);
    }

public:

    Type *get() 
    {
        return mPointer; 
    }

    Type *operator ->() 
    {
        return mPointer; 
    }

    //from IPointer
    virtual void resetPointer(char *p) override 
    {
        mPointer = (Type *)p; 
    }

    virtual unsigned getObjSize() override
    {
        return sizeof(Type);
    }

    virtual void *getVoidPtr() override
    {
        return (void *)mPointer;
    }

    Pointer(const Pointer<Type> &rhs)
    {
        mPointer = rhs.mPointer;
        mReleased = rhs.mReleased;

        increasePointer(mPointer);
    }

private:
    
    void release()
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

    void increasePointer(void *p)
    {
        if (!p)
            return;

        Allocator::getInst()->registerPointer(p, this);
    }

    unsigned releasePointer(void *p)
    {
        if (!p)
            return 0;

        return Allocator::getInst()->unregisterPointer(p, this);
    }

private:
    Pointer<Type> operator=(Pointer<Type> &rhs) = delete;

private:
    Type *mPointer;
    bool mReleased;
};