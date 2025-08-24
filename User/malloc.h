#ifndef __MALLOC_H
#define __MALLOC_H

#include "main.h"
#include "stm32f4xx.h"

#ifndef NULL
#define NULL 0
#endif

// 内存类型定义
#define INSRAM 0 // 内部内存
#define EXSRAM 1 // 外部内存

// 内部内存  内存分配用到的管理参数  用户可根据需要自行修改
#define INMEM_BLOCK_SIZE 32                                      // 内存块大小为32字节
#define INMEM_MAX_SIZE 80 * 1024                                 // 最大管理内部内存80K
#define INMEM_ALLOC_TABLE_SIZE INMEM_MAX_SIZE / INMEM_BLOCK_SIZE // 内存表大小

// 外部内存  内存分配用到的管理参数  用户可根据需要自行修改
#define EXMEM_BLOCK_SIZE 32                                      // 内存块大小为32字节
#define EXMEM_MAX_SIZE 800 * 1024                                // 最大管理外部内存800K
#define EXMEM_ALLOC_TABLE_SIZE EXMEM_MAX_SIZE / EXMEM_BLOCK_SIZE // 内存表大小

// 内存分配的有关函数声明
void Memory_Init(uint8_t memx);    // 内存管理初始化函数 memx:内外内存选择 0:内部  1:外部
uint8_t Mem_perused(uint8_t memx); // 获得内存使用率
void *Mem_malloc(uint8_t memx, uint32_t size);
void *Remem_malloc(uint8_t memx, void *old, uint32_t size);
void Mem_free(uint8_t memx, void *use);

#endif
