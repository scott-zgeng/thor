// fstring.h by scott.zgeng@gmail.com 2014.08.5


#ifndef  __FSTRING_H__
#define  __FSTRING_H__


#include <assert.h>

template<size_t SIZE>
struct fstring
{
public:
    fstring() {
        m_data[0] = 0;
    }

    fstring(const char* str) {
        assert(str != NULL && strlen(str) < SIZE);
        strncpy_s(m_data, SIZE, str, SIZE);
    }

    fstring(const fstring& str) {
        strncpy_s(m_data, SIZE, str.m_data, SIZE);
    }

    void operator=(const char* str) {
        assert(str != NULL && strlen(str) < SIZE);
        strncpy_s(m_data, SIZE, str, SIZE);
    }

    bool operator==(const fstring& str) const {
        return (strcmp(m_data, str.m_data) == 0);
    }

    bool operator!=(const fstring& str) const {
        return (strcmp(m_data, str.m_data) != 0);
    }

    const char* c_str() const {
        return m_data;
    }

private:
    char m_data[SIZE];
};

#endif //__FSTRING_H__

