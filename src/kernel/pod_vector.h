// pod_vector.h by scott.zgeng@gmail.com 2014.08.12


#ifndef  __POD_VECTOR_H__
#define  __POD_VECTOR_H__



#include <stdlib.h>
#include <assert.h>

class heap_allocator
{
public:
    static void* allocate(size_t n) {
        return malloc(n);
    }

    static void deallocate(void* p) {
        free(p);
    }
};



// NOTE(scott.zgeng): 
//  和标准的VECTOR类似，但不会抛异常，仅支持POD类型
//  支持内置的数据类型，指针，以及POD结构（没有虚函数，构造函数，析构函数，虚函数）
//  数据拷贝是浅拷贝，不做深拷贝，不会调用构造和析构函数，使用时请谨慎
template<typename T, int INIT_SIZE=8, class allocator=heap_allocator>
class pod_vector
{
public:
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* iterator;
    typedef const T* const_iterator;

public:
    pod_vector() {
        init();
    }

    ~pod_vector() {
        clear();
    }

    iterator begin() {
        return m_begin;
    }

    const_iterator begin() const {
        return m_begin;
    }

    iterator end() {
        return m_end;
    }

    const_iterator end() const {
        return m_end;
    }

    size_t size() const {
        return m_size;
    }

    size_t max_size() const {
        return 1024*1024;
    }

    bool empty() const {
        return (m_size == 0);
    }

    size_t capacity() const {
        return m_capacity;
    }

    reference at(size_t idx) {
        assert(idx < m_size);
        return m_begin[idx];
    }

    const_reference at(size_t idx) const {
        assert(idx < m_size);
        return m_begin[idx];
    }

    reference operator[] (size_t idx) {
        assert(idx < m_size);
        return m_begin[idx];
    }

    const_reference operator[] (size_t idx) const {
        assert(idx < m_size);
        return m_begin[idx];
    }

    reference front() const {
        return *m_begin;
    }

    reference back() const {
        return *(m_end - 1);
    }

    bool push_back(const_reference val) {
        if (!ensure_capacity(m_size + 1))
            return false;

        memcpy(m_end, &val, sizeof(T));
        m_size++;
        m_end++;
        return true;
    }

    void pop_back() {
        assert(m_size > 0);
        m_size--;
        m_end--;
    }

    void clear() {
        if (m_begin != (T*)m_init_buff) {
            m_allocator.deallocate(m_begin);
        }
        init();
    }

    void reset() {
        m_size = 0;
        m_end = m_begin; 
    }

    iterator erase(iterator it) {
        return erase(it, it + 1);
    }

    iterator erase(iterator first, iterator last) {
        assert(m_begin <= first && first < last && last <= m_end);

        size_t n = last - first;
        memmove(first, last, sizeof(T) * n);
        m_size -= n;
        m_end -= n;
    }

    bool resize(size_t n) {
        if (n > m_size) {
            if (!ensure_capacity(n))
                return false;

            memset(m_begin + m_size, 0, sizeof(T) * (n - m_size));
        }

        m_size = n;
        m_end = m_begin + n;
        return true;
    }

    bool resize(size_t n, const_reference val) {
        if (n > m_size) {
            if (!ensure_capacity(n))
                return false;

            for (size_t i = m_size; i < n; i++) {
                memcpy(m_begin + i, &val, sizeof(T));
            }
        }

        m_size = n;
        m_end = m_begin + n;
        return true;
    }

    iterator insert(iterator pos, const_reference val) {
        return insert(pos, 1, val);
    }

    iterator insert(iterator pos, size_t n, const_reference val) {
        assert(m_begin <= pos && pos <= m_end);

        size_t idx = pos - m_begin;

        if (m_size + n <= m_capacity) {
            memmove(pos + n, pos, sizeof(T) * (m_size - idx));
            for (size_t i = 0; i < n; i++) {
                memcpy(pos + i, &val, sizeof(T));
            }
                
            m_size += n;
            m_end += n;
            return pos;
        }

        // need reallocate
        size_t new_capacity = m_capacity * 2;
        T* new_data = (T*)m_allocator.allocate(sizeof(T) * new_capacity);
        if (new_data == NULL)
            return NULL;

        memcpy(new_data, m_begin, sizeof(T) * idx);

        iterator new_pos = new_data + idx; 
        for (size_t i = 0; i < n; i++) {
            memcpy(new_pos + i, &val, sizeof(T));
        }

        memcpy(new_pos + n, pos, sizeof(T)* (m_size - idx));

        if (m_begin != (T*)m_init_buff)
            m_allocator.deallocate(m_begin);

        m_capacity = new_capacity;
        m_size += n;
        m_begin = new_data;
        m_end = m_begin + m_size;

        return new_pos;
    }

    bool assign(size_t n) {
        if (!ensure_capacity(n))
            return false;

        memset(m_begin, 0, sizeof(T) * n);
        m_size = n;
        m_end = m_begin + m_size;
        return true;
    }

    bool assign(size_t n, const_reference val) {
        if (!ensure_capacity(n))
            return false;

        for (size_t i = 0; i < n; i++) {
            memcpy(m_begin + i, val, sizeof(T));
        }

        m_size = n;
        m_end = m_begin + m_size;
        return true;
    }

    bool assign(iterator first, iterator last) {
        assign(first < last);

        size_t n = last - first;
        if (!ensure_capacity(n))
            return false;

        memcpy(m_begin, first, size_t(T) * n);
        m_size = n;
        m_end = m_begin + m_size;
        return true;
    }

private:
    pod_vector(const_reference val) { 
        assert(false); 
    }

    void operator=(const_reference val) { 
        assert(false);         
    }
    
    void init() {
        m_capacity = INIT_SIZE;
        m_size = 0;
        m_begin = (T*)m_init_buff;
        m_end = m_begin;            
    }

    bool ensure_capacity(size_t n) {
        if (n < m_capacity)
            return true;

        size_t new_capacity = m_capacity * 2;
        T* new_data = (T*)m_allocator.allocate(sizeof(T) * new_capacity);
        if (new_data == NULL)
            return false;
            
        memcpy(new_data, m_begin, sizeof(T) * m_size);
        if (m_begin != (T*)m_init_buff)
            m_allocator.deallocate(m_begin);

        m_capacity = new_capacity;
        m_begin = new_data;
        m_end = m_begin + m_size;

        return true;
    }
    
private:
    allocator m_allocator;
    char m_init_buff[INIT_SIZE * sizeof(T)]; // don't use it direct

    size_t m_capacity;
    size_t m_size;
    T* m_begin;
    T* m_end;
};


#endif //__POD_VECTOR_H__


