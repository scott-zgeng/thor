// mem_pool.cpp by scott.zgeng@gmail.com 2014.08.15




//#include "buddy.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include <assert.h>
//#include <string.h>
//
//#define NODE_UNUSED 0
//#define NODE_USED 1	
//#define NODE_SPLIT 2
//#define NODE_FULL 3
//
//struct buddy {
//	int level;
//	uint8_t tree[1];
//};
//
//struct buddy * 
//buddy_new(int level) { 
//	int size = 1 << level;
//    struct buddy * self = (struct buddy *)malloc(sizeof(struct buddy) + sizeof(uint8_t)* (size * 2 - 2));
//	self->level = level;
//	memset(self->tree , NODE_UNUSED , size*2-1);
//	return self;
//}
//
//void
//buddy_delete(struct buddy * self) {
//	free(self);
//}
//
//static inline int
//is_pow_of_2(uint32_t x) {
//	return !(x & (x-1));
//}
//
//static inline uint32_t
//next_pow_of_2(uint32_t x) {
//	if ( is_pow_of_2(x) )
//		return x;
//	x |= x>>1;
//	x |= x>>2;
//	x |= x>>4;
//	x |= x>>8;
//	x |= x>>16;
//	return x+1;
//}
//
//static inline int
//_index_offset(int index, int level, int max_level) {
//	return ((index + 1) - (1 << level)) << (max_level - level);
//}
//
//static void 
//_mark_parent(struct buddy * self, int index) {
//	for (;;) {
//		int buddy = index - 1 + (index & 1) * 2;
//		if (buddy > 0 && (self->tree[buddy] == NODE_USED ||	self->tree[buddy] == NODE_FULL)) {
//			index = (index + 1) / 2 - 1;
//			self->tree[index] = NODE_FULL;
//		} else {
//			return;
//		}
//	}
//}
//
//int 
//buddy_alloc(struct buddy * self , int s) {
//	int size;
//	if (s==0) {
//		size = 1;
//	} else {
//		size = (int)next_pow_of_2(s);
//	}
//	int length = 1 << self->level;
//
//	if (size > length)
//		return -1;
//
//	int index = 0;
//	int level = 0;
//
//	while (index >= 0) {
//		if (size == length) {
//			if (self->tree[index] == NODE_UNUSED) {
//				self->tree[index] = NODE_USED;
//				_mark_parent(self, index);
//				return _index_offset(index, level, self->level);
//			}
//		} else {
//			// size < length
//			switch (self->tree[index]) {
//			case NODE_USED:
//			case NODE_FULL:
//				break;
//			case NODE_UNUSED:
//				// split first
//				self->tree[index] = NODE_SPLIT;
//				self->tree[index*2+1] = NODE_UNUSED;
//				self->tree[index*2+2] = NODE_UNUSED;
//			default:
//				index = index * 2 + 1;
//				length /= 2;
//				level++;
//				continue;
//			}
//		}
//		if (index & 1) {
//			++index;
//			continue;
//		}
//		for (;;) {
//			level--;
//			length *= 2;
//			index = (index+1)/2 -1;
//			if (index < 0)
//				return -1;
//			if (index & 1) {
//				++index;
//				break;
//			}
//		}
//	}
//
//	return -1;
//}
//
//static void 
//_combine(struct buddy * self, int index) {
//	for (;;) {
//		int buddy = index - 1 + (index & 1) * 2;
//		if (buddy < 0 || self->tree[buddy] != NODE_UNUSED) {
//			self->tree[index] = NODE_UNUSED;
//			while (((index = (index + 1) / 2 - 1) >= 0) &&  self->tree[index] == NODE_FULL){
//				self->tree[index] = NODE_SPLIT;
//			}
//			return;
//		}
//		index = (index + 1) / 2 - 1;
//	}
//}
//
//void
//buddy_free(struct buddy * self, int offset) {
//	assert( offset < (1<< self->level));
//	int left = 0;
//	int length = 1 << self->level;
//	int index = 0;
//
//	for (;;) {
//		switch (self->tree[index]) {
//		case NODE_USED:
//			assert(offset == left);
//			_combine(self, index);
//			return;
//		case NODE_UNUSED:
//			assert(0);
//			return;
//		default:
//			length /= 2;
//			if (offset < left + length) {
//				index = index * 2 + 1;
//			} else {
//				left += length;
//				index = index * 2 + 2;
//			}
//			break;
//		}
//	}
//}
//
//int
//buddy_size(struct buddy * self, int offset) {
//	assert( offset < (1<< self->level));
//	int left = 0;
//	int length = 1 << self->level;
//	int index = 0;
//
//	for (;;) {
//		switch (self->tree[index]) {
//		case NODE_USED:
//			assert(offset == left);
//			return length;
//		case NODE_UNUSED:
//			assert(0);
//			return length;
//		default:
//			length /= 2;
//			if (offset < left + length) {
//				index = index * 2 + 1;
//			} else {
//				left += length;
//				index = index * 2 + 2;
//			}
//			break;
//		}
//	}
//}
//
//static void
//_dump(struct buddy * self, int index , int level) {
//	switch (self->tree[index]) {
//	case NODE_UNUSED:
//		printf("(%d:%d)", _index_offset(index, level, self->level) , 1 << (self->level - level));
//		break;
//	case NODE_USED:
//		printf("[%d:%d]", _index_offset(index, level, self->level) , 1 << (self->level - level));
//		break;
//	case NODE_FULL:
//		printf("{");
//		_dump(self, index * 2 + 1 , level+1);
//		_dump(self, index * 2 + 2 , level+1);
//		printf("}");
//		break;
//	default:
//		printf("(");
//		_dump(self, index * 2 + 1 , level+1);
//		_dump(self, index * 2 + 2 , level+1);
//		printf(")");
//		break;
//	}
//}
//
//void
//buddy_dump(struct buddy * self) {
//	_dump(self, 0 , 0);
//	printf("\n");
//}
//
//
//
//









#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mem_pool.h"


//db_uint32 next_power_of_2(db_uint32 x)
//{
//    --x;
//    x |= x>>1;
//    x |= x>>2;
//    x |= x>>4;
//    x |= x>>8;
//    x |= x>>16;
//    ++x;
//    return x;
//}


mem_pool::mem_pool()
{
    m_bitmap = NULL;    
    m_data = NULL;

    memset(m_free_entry, 0, sizeof(m_free_entry));
}

mem_pool::~mem_pool()
{
    uninit();
}


result_t mem_pool::init(db_size pool_size)
{
    IF_RETURN_FAILED(pool_size % MAX_PAGE_SIZE != 0);
    
    // 计算存放 MAX_PAGE 能存放多少个
    db_size page_num = pool_size / MAX_PAGE_SIZE;
    IF_RETURN_FAILED(page_num < 1);

    m_data = (db_byte*)malloc(pool_size);
    IF_RETURN_FAILED(m_data == NULL);

    // 一个 MAX_PAGE 对应一个BITMAP
    db_size bitmap_size = page_num * BITMAP_UNIT_SIZE;
    m_bitmap = (db_byte*)malloc(bitmap_size);

    // TODO(scott.zgeng@gmail.com): 先简单写，后面改为自动指针   
    if (m_bitmap == NULL) {
        free(m_data);
        m_data = NULL;
        return RT_FAILED;
    }
    
    // 清除BITMAP状态
    memset(m_bitmap, 0, bitmap_size);

    page_head_t* entry = m_free_entry + MAX_LEVEL;
    page_head_t* curr = (page_head_t*)m_data;

    curr->perv = entry;
    entry->next = curr;
    
    page_head_t* prev;
    
    for (db_byte* ptr = m_data + MAX_PAGE_SIZE; ptr < m_data + pool_size; ptr += MAX_PAGE_SIZE) {
        curr = (page_head_t*)ptr;
        prev = (page_head_t*)(ptr - MAX_PAGE_SIZE);

        curr->perv = prev;
        prev->next = curr;
    }

    curr = (page_head_t*)(m_data + pool_size - MAX_PAGE_SIZE);
    curr->next = NULL;

    return RT_SUCCEEDED;
}

void mem_pool::uninit()
{
    if (m_data != NULL) {
        free(m_data);
        m_data = NULL;
    }

    if (m_bitmap != NULL) {
        free(m_bitmap);
        m_bitmap = NULL;
    }

    memset(m_free_entry, 0, sizeof(m_free_entry));
}



db_uint32 calc_power_of_2(db_uint32 x)
{
    x--;
    db_uint32 count = 0;    
    while (x != 0) {
        x = x >> 1;
        count++;
    } 
    return count;
}


void* mem_pool::alloc(db_size size)
{
    assert(MIN_PAGE_SIZE <= size && size <= MAX_PAGE_SIZE);
    
    db_int32 level = calc_power_of_2(size) - MIN_PAGE_BITS;

    return alloc_inner(level);
}


mem_pool::page_head_t* mem_pool::alloc_inner(db_uint32 level)
{
    if (level > MAX_LEVEL) return NULL;

    // 如果空闲链表中有空闲PAGE，则直接将空闲的分配出去
    page_head_t* entry = m_free_entry + level;
    page_head_t* temp = entry->next;
    if (temp != NULL) {
        unlink_from_pool(temp); 
        set_state(temp, level, MEM_ALLOC);
        return temp;
    } 

    // 如果对应的空闲链表是空的，则需要从上一级申请一个大PAGE
    page_head_t* parent = alloc_inner(level + 1);
    if (parent == NULL) return NULL;

    // 将前面一个PAGE挂到空闲链表
    link_to_pool(parent, level);
    
    // 将后面一个page返回
    temp = (page_head_t*)((db_byte*)parent + (1 << level) * MIN_PAGE_SIZE);
    set_state(temp, level, MEM_ALLOC);
    return temp;
}



void mem_pool::free(void* ptr)
{
    // 计算最小PAGE所对应的序号
    db_uint32 page_no = get_page_no(ptr);

    // 找到PAGE对应BITMAP的入口地址
    db_byte* base = get_bitmap_base(page_no);
    db_uint32 page_idx = page_no & PAGE_MASK;    

    // 找出指针申请的长度
    db_uint32 level = 0;
    db_uint32 tree_idx = page_idx + (1 << MAX_LEVEL);

    while (true) {        
        state_t state = (state_t)bitmap<BITMAP_BIT>::get(base, tree_idx);
        if (state == MEM_ALLOC)
            break;

        tree_idx = tree_idx >> 1;
        level++;
    }

    free_inner((db_byte*)ptr, level);
    return;
}

void mem_pool::free_inner(db_byte* ptr, db_uint32 level)
{
    set_state(ptr, level, MEM_FREE);

    // 如果是已经最大页面了，直接放到链表即可
    if (level == MAX_LEVEL) {
        link_to_pool(ptr, level);        
        return;
    }

    db_byte* buddy_ptr = get_buddy(ptr, level);
    state_t state = get_state(buddy_ptr, level);

    if (state != MEM_FREE) {
        link_to_pool(ptr, level);        
        return;
    }

    // buddy is free
    unlink_from_pool(buddy_ptr);
    db_byte* parent_ptr = ptr < buddy_ptr ? ptr : buddy_ptr;
    free_inner(parent_ptr, level + 1);
    return;
}

