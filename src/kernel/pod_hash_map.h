// pod_hash_map.h by scott.zgeng@gmail.com 2014.08.12


#ifndef  __POD_HASH_MAP_H__
#define  __POD_HASH_MAP_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fstring.h"


/*
    good hash table primes

    lwr    upr    %err      prime
    2 5    2 6    10.416667 53
    2 6    2 7    1.041667  97
    2 7    2 8    0.520833  193
    2 8    2 9    1.302083  389
    2 9    2 10   0.130208  769
    2 10   2 11   0.455729  1543
    2 11   2 12   0.227865  3079
    2 12   2 13   0.113932  6151
    2 13   2 14   0.008138  12289
    2 14   2 15   0.069173  24593
    2 15   2 16   0.010173  49157
    2 16   2 17   0.013224  98317
    2 17   2 18   0.002543  196613
    2 18   2 19   0.006358  393241
    2 19   2 20   0.000127  786433
    2 20   2 21   0.000318  1572869
    2 21   2 22   0.000350  3145739
    2 22   2 23   0.000207  6291469
    2 23   2 24   0.000040  12582917
    2 24   2 25   0.000075  25165843
    2 25   2 26   0.000010  50331653
    2 26   2 27   0.000023  100663319
    2 27   2 28   0.000009  201326611
    2 28   2 29   0.000001  402653189
    2 29   2 30   0.000011  805306457
    2 30   2 31   0.000000  1610612741
*/


template<typename K>
struct hash_function
{
    unsigned int operator() (const K& key) const {
        unsigned int seed = 131;
        unsigned int hash = 0;
        char* input = (char*)&key;

        for (size_t i = 0; i < sizeof(key); i++) {
            hash = hash * seed + input[i];
        }
        return hash;
    }
};


static inline unsigned int str_hash_function(char const* key)
{
    unsigned int seed = 131;
    unsigned int hash = 0;
    char const* input = key;

    while (*input != 0) {
        hash = hash * seed + *input;
        input++;
    }
    return hash;
}


template<>
struct hash_function<char const*> {
    unsigned int operator() (char const* key) const {
        return str_hash_function(key);
    }
};


template<>
struct hash_function<char*> {
    unsigned int operator() (char* key) const {
        return str_hash_function(key);
    }
};


template<typename T>
struct equal_function
{
    bool operator() (const T& a, const T& b) { 
        return (a == b); 
    }
};


template<>
struct equal_function<char const*> {
    bool operator() (char const* a, char const* b) {
        return (strcmp(a, b) == 0);
    }
};


template<>
struct equal_function<char*> {
    bool operator() (char* a, char * b) {
        return (strcmp(a, b) == 0);
    }
};


template<size_t SIZE>
struct equal_function<fstring<SIZE> > {
    bool operator() (const fstring<SIZE>& a, const fstring<SIZE>& b) {
        return (a == b);
    }
};


class map_allocator
{
public:
    static void* alloc_bucket(size_t sz) {
        return malloc(sz);
    }
    static void free_bucket(void* p) {
        free(p);
    }
    static void* alloc_node(size_t sz) {
        return malloc(sz);
    }
    static void free_node(void* p) {
        free(p);
    }
};


template<typename K, typename V, size_t INIT_SIZE = 53, class allocator = map_allocator>
class pod_hash_map
{
public:
    typedef pod_hash_map<K, V, INIT_SIZE, allocator> self;

public:
    class iterator;
    class node {
    public:
        friend self;
        friend iterator;

        unsigned int hash() const {
            return m_hash;
        }

        const K& key() const {
            return m_key;
        }

        const V& value() const {
            return m_value;
        }
    private:
        node* m_next;
        unsigned int m_hash;
        K m_key;
        V m_value;        
    };

    typedef node* node_pointer;

    class iterator
    {
    public:
        friend self;

        iterator() {
            m_base = NULL;
            m_curr = NULL;
            m_end = NULL;
        }

        iterator(self* map, node_pointer* base) {
            m_base = base;
            m_curr = base;
            m_end = map->m_buckets + map->m_bucket_num;
        }

        iterator(const iterator& it) {
            m_base = it.m_base;
            m_curr = it.m_curr;
            m_end = it.m_end;
        }

        void operator=(const iterator& it) {
            m_base = it.m_base;
            m_curr = it.m_curr;
            m_end = it.m_end;
        }

        const K& key() const {
            assert(m_curr != NULL && *m_curr != NULL);
            return (*m_curr)->m_key;
        }

        const V& value() const {
            assert(m_curr != NULL && *m_curr != NULL);
            return (*m_curr)->m_value;
        }

        void next() {
            assert(m_curr != NULL);

            node_pointer curr = *m_curr;
            if (curr != NULL && curr->m_next != NULL) {
                m_curr = &curr->m_next;
                return;
            }

            while (m_base != m_end) {
                m_base++;
                if (*m_base != NULL) {
                    m_curr = m_base;
                    return;
                }
            }
            m_curr = m_base;
            return;
        }
        void operator++() {
            next();
        }

        bool operator==(const iterator& it) const {
            return (m_base == it.m_base && m_curr == it.m_curr);
        }

        bool operator!=(const iterator& it) const {
            return (m_base != it.m_base || m_curr != it.m_curr);
        }

    private:
        node_pointer* m_base;
        node_pointer* m_curr;
        node_pointer* m_end;
    };
    
    pod_hash_map() {
        init(false);
    }

    pod_hash_map(bool fixed) {
        init(fixed);
    }

    ~pod_hash_map() {
        clear();
    }

    bool empty() const {
        return (m_size == 0);
    }

    size_t size() const {
        return m_size;
    }

    iterator begin() {
        if (m_size == 0)
            return m_end;

        node_pointer* base = m_buckets;
        while (NULL == *base)
            base++;

        return iterator(this, base);
    }


    iterator find(const K& key) {
        hash_function<K> hash_func;
        unsigned int hash = hash_func(key);
        node_pointer* base = m_buckets + (hash % m_bucket_num);
        node_pointer* ptr = find_inner(hash, base, key);

        return (ptr != NULL) ? iterator(this, base, ptr) : m_end;
    }

    const iterator& find(const K& key) const {
        hash_function<K> hash_func;
        unsigned int hash = hash_func(key);
        node_pointer* base = m_buckets + (hash % m_bucket_num);
        node_pointer* ptr = find_inner(hash, base, key);

        return (ptr != NULL) ? iterator(this, base, ptr) : m_end;
    }

    node_pointer find_node(const K& key) const {
        hash_function<K> hash_func;
        unsigned int hash = hash_func(key);
        node_pointer* base = m_buckets + (hash % m_bucket_num);
        node_pointer* ptr = find_inner(hash, base, key);

        return (ptr != NULL) ? *ptr : NULL;
    }

    bool insert(const K& key, const V& value) {
        if (!ensure_capacity())
            return false;

        hash_function<K> hash_func;
        unsigned int hash = hash_func(key);
        node_pointer* base = m_buckets + (hash % m_bucket_num);

        if (find_inner(hash, base, key) != NULL)
            return false;

        return insert_inner(hash, base, key, value);
    }

    bool erase(const K& key) {
        hash_function<K> hash_func;
        unsigned int hash = hash_func(key);
        node_pointer* base = m_buckets + (hash % m_bucket_num);
        node_pointer* ptr = find_inner(hash, base, key);
        if (ptr == NULL)
            return false;

        erase_inner(ptr);
        return true;
    }

    bool erase(const iterator& it) {
        assert(it.m_curr != NULL && *(it.m_curr) != NULL);
        erase_inner(it.m_curr);
    }

    void clear() {
        for (node_pointer* base = m_buckets; base < m_end.m_base; base++) {
            while (*base != NULL) {
                erase_inner(base);
            }
        }

        assert(m_size == 0);
        if (m_buckets != m_init_buff) {
            m_allocator.free_bucket(m_buckets);
            m_buckets = m_init_buff;
            m_bucket_num = INIT_SIZE;
            memset(m_init_buff, 0, sizeof(m_init_buff));
        }
    }

private:
    pod_hash_map(const pod_hash_map& map) {
        assert(false);
    }

    void operator=(const pod_hash_map& map) {
        assert(false);
    }

protected:
    void init(bool fixed) {
        m_fixed = fixed;
        memset(m_init_buff, 0, sizeof(m_init_buff));
        m_buckets = m_init_buff;
        m_bucket_num = INIT_SIZE;
        m_size = 0;
        m_end = iterator(this, m_buckets + m_bucket_num);
    }

    node_pointer* find_inner(unsigned int hash, node_pointer* base, const K& key) const {
        node_pointer* prev = base;
        node_pointer curr = *base;
        equal_function<K> equal_func;

        while (curr != NULL) {
            if (curr->m_hash == hash && equal_func(curr->m_key, key))
                return prev;

            prev = &curr->m_next;
            curr = curr->m_next;
        }

        return NULL;
    }

    bool insert_inner(unsigned int hash, node_pointer* base, const K& key, const V& value) {
        node_pointer ptr = (node_pointer)m_allocator.alloc_node(sizeof(node));
        if (ptr == NULL)
            return false;

        ptr->m_hash = hash;
        memcpy(&ptr->m_key, &key, sizeof(K));
        memcpy(&ptr->m_value, &value, sizeof(V));
        ptr->m_next = *base;
        *base = ptr;
        m_size++;
        return true;
    }

    void erase_inner(node_pointer* ptr) {
        assert(ptr != NULL && *ptr != NULL);

        node_pointer curr = *ptr;
        *ptr = curr->m_next;
        m_allocator.free_node(curr);
        m_size--;
    }

    bool ensure_capacity() {
        if (m_fixed || m_size * 2 < m_bucket_num)
            return true;

        size_t new_size = m_bucket_num * 2 + 1;
        size_t alloc_size = (new_size + 1) * sizeof(node_pointer);
        node_pointer* new_buckets = (node_pointer*)m_allocator.alloc_bucket(alloc_size);
        if (new_buckets == NULL)
            return false;

        memset(new_buckets, 0, alloc_size);

        iterator it = begin();
        while (it != m_end) {
            node_pointer curr = *(it.m_curr);
            it.next();

            node_pointer* base = new_buckets + (curr->m_hash & new_size);
            curr->m_next = *base;
            *base = curr;
        }

        if (m_buckets != m_init_buff)
            m_allocator.free_bucket(m_buckets);

        m_buckets = new_buckets;
        m_bucket_num = new_size;
        m_end = iterator(this, m_buckets + m_bucket_num);
        return true;
    }

private:
    bool m_fixed;
    node_pointer m_init_buff[INIT_SIZE + 1]; // don't use it direct
    node_pointer* m_buckets;
    size_t m_bucket_num;
    size_t m_size;
    iterator m_end;

    allocator m_allocator;
};


#endif //__POD_HASH_MAP_H__

