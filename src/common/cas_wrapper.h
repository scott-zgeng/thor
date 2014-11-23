// cas_wrapper.h by scott.zgeng@gmail.com 2014.11.23

#ifndef  __CAS_WRAPPER_H__
#define  __CAS_WRAPPER_H__




#ifdef  __linux


inline bool cas_int8(volatile char* ptr, char old_val, char new_val)
{
    return __sync_bool_compare_and_swap(ptr, old_val, new_val);
}

inline bool cas_int16(volatile short* ptr, short old_val, short new_val)
{
    return __sync_bool_compare_and_swap(ptr, old_val, new_val);
}

inline bool cas_int32(volatile int* ptr, int old_val, int new_val)
{
    return __sync_bool_compare_and_swap(ptr, old_val, new_val);
}

inline bool cas_int64(volatile long long* ptr, long long old_val, long long new_val)
{
    return __sync_bool_compare_and_swap(ptr, old_val, new_val);
}

inline bool cas_ptr(volatile void** ptr, void* old_val, void* new_val)
{
    return __sync_bool_compare_and_swap(ptr, old_val, new_val);
}

#else 

inline bool cas_int8(volatile char* ptr, char old_val, char new_val)
{
    return false;
}

inline bool cas_int16(volatile short* ptr, short old_val, short new_val)
{
    return false;
}

inline bool cas_int32(volatile int* ptr, int old_val, int new_val)
{
    return false;
}

inline bool cas_int64(volatile long long* ptr, long long old_val, long long new_val)
{
    return false;
}

inline bool cas_ptr(volatile void** ptr, void* old_val, void* new_val)
{
    return false;
}


#endif



#endif //__CAS_WRAPPER_H__


