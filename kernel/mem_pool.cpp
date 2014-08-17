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
#include "bitmap.h"



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
    
    m_data = (db_byte*)malloc(pool_size);
    IF_RETURN_FAILED(m_data == NULL);

        
    // 计算能存放最小PAGE的上限
    db_size page_num_limit = pool_size % MIN_PAGE_SIZE;

    // 一个最小页面占用一个BIT，存放完全二叉树的
    // 计算用于存放标记空闲状态的完全二叉树的BITMAP的内存大小，
    db_size bitmap_size = (page_num_limit + 7) / 8 * 2;
    m_bitmap = (db_byte*)malloc(bitmap_size);

    // TODO(scott.zgeng@gmail.com): 先简单写，后面改为自动指针   
    if (m_bitmap == NULL) {
        free(m_data);
        return RT_FAILED;
    }
    
    return RT_SUCCEEDED;
}

void mem_pool::uninit()
{

}


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


void* mem_pool::alloc_inner(db_uint32 level)
{
    if (level > MAX_LEVEL) return NULL;

    page_head_t* first = m_free_entry + level;
    
    page_head_t* temp = first->next;
    if (temp != NULL) {
        unlink_from_pool(temp); 
        set_state(temp, level, MEM_ALLOC);
        return temp;
    } 

    // 如果对应的空闲链表是空的，则需要从上一级申请一个大PAGE
    db_byte* parent = (db_byte*)alloc_inner(level + 1);
    if (parent == NULL) return NULL;

    set_state(parent, level + 1, MEM_SPLIT);
    // 将前面一个PAGE挂到空闲链表
    link_to_pool(parent, first);    
    
    // 将后面一个page返回
    temp = (page_head_t*)parent + (1 << level) * MIN_PAGE_SIZE;
    set_state(temp, level, MEM_ALLOC);
    return temp;
}



void mem_pool::set_state(void* ptr, db_uint32 level, state_t state)
{
    // 计算最小PAGE所对应的id
    db_uint32 page_no = get_page_no(ptr);

    // 找到PAGE对应BITMAP的入口地址
    db_byte* base = get_bitmap_base(page_no);

    // 计算在二叉树的实际位置
    db_uint32 offset = page_no & PAGE_MASK;
    db_uint32 index = (offset >> level) + (1 << (MAX_LEVEL - level));
     
    bitmap<2>::set(base, index, state);
}


void mem_pool::free(void* ptr)
{
    // 计算最小PAGE所对应的序号
    db_uint32 page_no = get_page_no(ptr);

    // 找到PAGE对应BITMAP的入口地址
    db_byte* base = get_bitmap_base(page_no);
    db_uint32 offset = page_no & PAGE_MASK;    

    // 找出指针申请的长度
    db_uint32 level = 0;
    db_uint32 index = 0;
    while (true) {
        db_uint32 index = (offset >> level) + (1 << (MAX_LEVEL - level));
        state_t state = (state_t)bitmap<2>::get(base, index);

        if (state == MEM_ALLOC)
            break;

        level++;
    }


    free_inner(ptr, level);







    // 如果是未使用状态，则将兄弟节点从链表中释放，调用上一级的
    

    //page_head_t* temp = (page_head_t*)ptr;

    //temp



}

void mem_pool::free_inner(db_byte* base, void* ptr, db_uint32 level, db_uint32 offset)
{
    page_head_t* first = m_free_entry + level;

    // 检查对应的兄弟节点是否处于未使用状态    
    db_uint32 index = (offset >> level) + (1 << (MAX_LEVEL - level));

    db_uint32 buddy = ((index >> 1) << 2) + 1 - index;    
    state_t state = (state_t)bitmap<2>::get(base, buddy);

    if (state != MEM_FREE)
        return;
      
    void* buddy = (index & 0x01) ? (db_byte*)ptr - (1 << level) : (db_byte*)ptr + (1 << level);

    unlink_from_pool(buddy);


    free_inner();

}

