// smart_pointer.h by scott.zgeng@gmail.com 2014.08.13



#ifndef  __SMART_POINTER_H__
#define  __SMART_POINTER_H__


#include <assert.h>

template<typename T>
class default_allocator
{
public:
    T* allocate() {
        return new T();
    }

    void deallocate(T* p) {
        delete p;
    }
};


template<typename T, class AL = default_allocator<T> >
class smart_pointer
{
public:
    smart_pointer() {
        m_ptr = NULL;
    }

    smart_pointer(T* ptr) {
        m_ptr = ptr;
    }

    ~smart_pointer() {        
        if (m_ptr != NULL) {            
            AL allocator;
            allocator.deallocate(m_ptr);
        }
    }

    bool create_instance() {       
        assert(m_ptr == NULL);
        AL allocator;
        m_ptr = allocator.allocate();
        return (m_ptr != NULL);
    }

    void attach(T* t) {
        m_ptr = ptr;
    }

    T* detach() {
        T* ptr = m_ptr;
        m_ptr = NULL;
        return ptr;
    }

    T* ptr() const {
        return m_ptr;
    }

    T* operator-> () const {
        return m_ptr;
    }

private:
    smart_pointer(const smart_pointer& other) {
        assert(false);
    }
    void operator=(const smart_pointer& other) {
        assert(false);
    }

private:    
    T* m_ptr;
};



#endif //__SMART_POINTER_H__

