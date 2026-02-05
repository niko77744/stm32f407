# LwRB API 使用指南

## 概述

LwRB (Lightweight Ring Buffer) 是一个轻量级循环缓冲区库，提供通用的 FIFO（先进先出）缓冲区实现。它完全使用 C 语言编写，无动态内存分配，适合嵌入式系统和 DMA 传输场景。

---

## 核心结构

### `lwrb_t` - 缓冲区结构

```c
typedef struct lwrb {
    uint8_t* buff;           // 指向缓冲区数据
    lwrb_sz_t size;          // 缓冲区大小（实际可用为 size-1）
    lwrb_sz_atomic_t r_ptr;  // 读指针
    lwrb_sz_atomic_t w_ptr;  // 写指针
    lwrb_evt_fn evt_fn;      // 事件回调函数
    void* arg;               // 用户自定义参数
} lwrb_t;
```

### 重要类型

| 类型 | 描述 |
|------|------|
| `lwrb_sz_t` | 缓冲区大小类型（默认 `unsigned long`） |
| `lwrb_evt_type_t` | 事件类型枚举 |
| `lwrb_evt_fn` | 事件回调函数类型 |

---

## 工作原理

### 核心概念

LwRB 通过三个关键变量管理数据：

| 变量 | 作用 |
|------|------|
| **R** (Read) | 读指针 - 指向下一个要读取的位置 |
| **W** (Write) | 写指针 - 指向下一个要写入的位置 |
| **S** (Size) | 缓冲区总大小（固定值） |

### 指针的工作方式

```
缓冲区大小 S = 8

R 和 W 的取值范围：0, 1, 2, 3, 4, 5, 6, 7
当达到 7 后，下一个值是 0（循环）

例如：0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 0 → 1 → ...
```

**重要**：R 和 W 始终指向**下一个**操作的位置。

### 空满判断

| 状态 | 条件 | 说明 |
|------|------|------|
| **空** | `W == R` | 写追上读，没有数据 |
| **满** | `W == R - 1` | 写追上读的前一个，无空间 |

### 5 种典型状态

```
Case A: 空缓冲区
    W == R == 0
    [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
     ↑
    W,R
    
    已用: 0 字节


Case B: 简单情况（W > R）
    W = 4, R = 0
    [x] [x] [x] [x] [ ] [ ] [ ] [ ]
     ↑               ↑
    R(0)           W(4)
    
    已用: W - R = 4 字节


Case C: 满缓冲区（W > R）
    W = 7, R = 0
    [x] [x] [x] [x] [x] [x] [x] [ ]
     ↑                           ↑
    R(0)                        W(7)
    
    已用: 7 字节
    状态: W == R - 1（满）


Case D: 回绕情况（R > W）
    W = 3, R = 5
    [x] [x] [x] [ ] [ ] [x] [x] [x]
              ↑           ↑
             W(3)       R(5)
    
    已用: S - (R - W) = 8 - (5 - 3) = 6 字节


Case E: 满缓冲区（R > W）
    W = 4, R = 5
    [x] [x] [x] [x] [ ] [x] [x] [x]
              ↑       ↑
             W(4)   R(5)
    
    已用: 7 字节
    状态: W == R - 1（满）
```

### 容量计算公式

```
如果 W > R:
    已用字节数 = W - R

如果 R > W（回绕）:
    已用字节数 = S - (R - W)
```

### 最大容量

缓冲区实际能存储的最大字节数总是 **S - 1**（而不是 S）。

原因：如果用满 S 个字节，则 `W == R`，这会被误判为"空"。

### 模运算（处理指针溢出）

当进行指针加减运算时，需要使用模运算确保结果在 0 到 S-1 范围内：

```c
// 计算下一个写位置（加 1）
W = (W + 1) % S;

// 计算前一个位置（减 1）
W = (W - 1 + S) % S;  // +S 防止负数
```

### 总结要点

1. **R** 和 **W** 在 0 到 S-1 之间循环
2. **W == R** → 缓冲区空
3. **W == R - 1** → 缓冲区满
4. 最大容量为 **S - 1**
5. 数据可能分两段存储（回绕时）

---

## API 分类

### 1. 初始化与状态管理

#### `lwrb_init()`
```c
uint8_t lwrb_init(lwrb_t* buff, void* buffdata, lwrb_sz_t size);
```
- **功能**：初始化缓冲区
- **参数**：
  - `buff`: 缓冲区结构指针
  - `buffdata`: 数据数组指针
  - `size`: 数组大小
- **返回值**：`1` 成功，`0` 失败
- **注意事项**：任何参数为 `NULL` 或 `size` 为 0 都会返回失败

#### `lwrb_is_ready()`
```c
uint8_t lwrb_is_ready(lwrb_t* buff);
```
- **功能**：检查缓冲区是否已初始化
- **返回值**：`1` 就绪，`0` 未就绪

#### `lwrb_reset()`
```c
void lwrb_reset(lwrb_t* buff);
```
- **功能**：重置缓冲区，清空所有数据
- **触发事件**：`LWRB_EVT_RESET`

#### `lwrb_free()`
```c
void lwrb_free(lwrb_t* buff);
```
- **功能**：释放缓冲区（将内部指针置 NULL）

---

### 2. 基本读写操作

#### `lwrb_write()`
```c
lwrb_sz_t lwrb_write(lwrb_t* buff, const void* data, lwrb_sz_t btw);
```
- **功能**：写入数据到缓冲区
- **参数**：
  - `btw`: bytes to write，要写入的字节数
- **返回值**：实际写入的字节数（可能小于请求值）
- **特点**：尽可能多地写入数据
- **触发事件**：`LWRB_EVT_WRITE`

#### `lwrb_read()`
```c
lwrb_sz_t lwrb_read(lwrb_t* buff, void* data, lwrb_sz_t btr);
```
- **功能**：从缓冲区读取数据
- **参数**：
  - `btr`: bytes to read，要读取的字节数
- **返回值**：实际读取的字节数
- **特点**：读取后数据从缓冲区移除
- **触发事件**：`LWRB_EVT_READ`

#### `lwrb_peek()`
```c
lwrb_sz_t lwrb_peek(const lwrb_t* buff, lwrb_sz_t skip_count, void* data, lwrb_sz_t btp);
```
- **功能**：查看数据但不移动读指针
- **参数**：
  - `skip_count`: 跳过的字节数
  - `btp`: bytes to peek，要查看的字节数
- **返回值**：实际查看的字节数
- **特点**：不会触发读事件，不影响缓冲区状态

---

### 3. 扩展读写操作（带标志）

#### `lwrb_write_ex()`
```c
uint8_t lwrb_write_ex(lwrb_t* buff, const void* data, lwrb_sz_t btw, 
                       lwrb_sz_t* bwritten, uint16_t flags);
```
- **功能**：扩展写入，支持原子操作
- **标志位**：
  - `LWRB_FLAG_WRITE_ALL`: 必须写入全部数据，否则返回失败
- **返回值**：`1` 成功，`0` 失败
- **用法示例**：
  ```c
  lwrb_sz_t written;
  uint8_t success = lwrb_write_ex(&buff, data, len, &written, LWRB_FLAG_WRITE_ALL);
  ```

#### `lwrb_read_ex()`
```c
uint8_t lwrb_read_ex(lwrb_t* buff, void* data, lwrb_sz_t btr, 
                     lwrb_sz_t* bread, uint16_t flags);
```
- **功能**：扩展读取，支持原子操作
- **标志位**：
  - `LWRB_FLAG_READ_ALL`: 必须读取全部数据，否则返回失败
- **返回值**：`1` 成功，`0` 失败

---

### 4. 缓冲区状态查询

#### `lwrb_get_free()`
```c
lwrb_sz_t lwrb_get_free(const lwrb_t* buff);
```
- **功能**：获取缓冲区可用空间
- **返回值**：可写入的字节数

#### `lwrb_get_full()`
```c
lwrb_sz_t lwrb_get_full(const lwrb_t* buff);
```
- **功能**：获取缓冲区已使用空间
- **返回值**：已存储的字节数

---

### 5. 线性块操作（适合 DMA）

#### `lwrb_get_linear_block_read_address()`
```c
void* lwrb_get_linear_block_read_address(const lwrb_t* buff);
```
- **功能**：获取线性可读块的起始地址

#### `lwrb_get_linear_block_read_length()`
```c
lwrb_sz_t lwrb_get_linear_block_read_length(const lwrb_t* buff);
```
- **功能**：获取线性可读块的长度

#### `lwrb_skip()`
```c
lwrb_sz_t lwrb_skip(lwrb_t* buff, lwrb_sz_t len);
```
- **功能**：跳过指定字节数（相当于读取但不复制）

#### `lwrb_get_linear_block_write_address()`
```c
void* lwrb_get_linear_block_write_address(const lwrb_t* buff);
```
- **功能**：获取线性可写块的起始地址

#### `lwrb_get_linear_block_write_length()`
```c
lwrb_sz_t lwrb_get_linear_block_write_length(const lwrb_t* buff);
```
- **功能**：获取线性可写块的长度

#### `lwrb_advance()`
```c
lwrb_sz_t lwrb_advance(lwrb_t* buff, lwrb_sz_t len);
```
- **功能**：推进写指针（相当于写入但不复制）

---

### 6. 搜索与高级操作

#### `lwrb_find()`
```c
uint8_t lwrb_find(const lwrb_t* buff, const void* bts, lwrb_sz_t len, 
                  lwrb_sz_t start_offset, lwrb_sz_t* found_idx);
```
- **功能**：在缓冲区中搜索数据
- **参数**：
  - `bts`: bytes to search，要搜索的数据
  - `len`: 搜索数据长度
  - `start_offset`: 起始偏移量
  - `found_idx`: 找到的索引（输出参数）
- **返回值**：`1` 找到，`0` 未找到

#### `lwrb_overwrite()`
```c
lwrb_sz_t lwrb_overwrite(lwrb_t* buff, const void* data, lwrb_sz_t btw);
```
- **功能**：覆盖写入（空间不足时覆盖旧数据）
- **特点**：总是写入全部数据，可能丢弃最早的数据

#### `lwrb_move()`
```c
lwrb_sz_t lwrb_move(lwrb_t* dest, lwrb_t* src);
```
- **功能**：将数据从一个缓冲区移动到另一个
- **返回值**：实际移动的字节数

---

### 7. 事件管理

#### `lwrb_set_evt_fn()`
```c
void lwrb_set_evt_fn(lwrb_t* buff, lwrb_evt_fn fn);
```
- **功能**：设置事件回调函数

#### `lwrb_set_arg()` / `lwrb_get_arg()`
```c
void lwrb_set_arg(lwrb_t* buff, void* arg);
void* lwrb_get_arg(lwrb_t* buff);
```
- **功能**：设置/获取用户自定义参数

---

## 事件类型

| 事件 | 触发时机 |
|------|----------|
| `LWRB_EVT_READ` | 数据被读取时 |
| `LWRB_EVT_WRITE` | 数据被写入时 |
| `LWRB_EVT_RESET` | 缓冲区被重置时 |

---

## 重要注意事项

### 1. 缓冲区大小
- 实际可用空间为 `size - 1` 字节
- 例如：分配 10 字节，最多只能存储 9 字节
- 分配数组时建议多分配 1 字节

```c
// 存储 8 字节数据，需要分配 9 字节
uint8_t buffer_data[8 + 1];
lwrb_t buff;
lwrb_init(&buff, buffer_data, sizeof(buffer_data));
```

### 2. 线程与中断安全
- **单写单读场景**：在 ARM Cortex-M 等 CPU 上，当 `size_t` 读写是原子操作时，天然是线程/中断安全的
- **多写多读场景**：需要额外的互斥保护
- **小架构 CPU（如 AVR）**：需要原子保护来保证 `size_t` 读写安全

### 3. DMA 使用
- 使用 `get_linear_block_*` 函数获取连续内存块
- 配合 DMA 实现零拷贝传输
- 操作完成后调用 `skip()` 或 `advance()` 更新指针

### 4. 初始化检查
- 使用 `lwrb_is_ready()` 检查缓冲区是否已初始化
- 未初始化的缓冲区使用会返回错误

### 5. 空满判断
- **空**：`r_ptr == w_ptr`
- **满**：`w_ptr == r_ptr - 1`（模 size）

### 6. 返回值处理
- `lwrb_write()`/`lwrb_read()` 返回的是实际操作的字节数
- 需要检查返回值以确认是否满足需求
- 使用 `_ex` 版本可以确保全部/不写入

### 7. Peek 操作
- `lwrb_peek()` 不会触发读事件，也不移动读指针
- 适合预览数据但不消费的场景

---

## 代码示例

### 1. 最小示例

```c
#include "lwrb/lwrb.h"

/* 声明缓冲区实例和数据数组 */
lwrb_t buff;
uint8_t buff_data[8];

/* 初始化缓冲区 */
lwrb_init(&buff, buff_data, sizeof(buff_data));

/* 写入 4 字节数据 */
lwrb_write(&buff, "0123", 4);

/* 打印缓冲区中的字节数 */
printf("Bytes in buffer: %d\r\n", (int)lwrb_get_full(&buff));  // 输出: 4

/* 读取数据 */
uint8_t data[8];
size_t len = lwrb_read(&buff, data, sizeof(data));
printf("Number of bytes read: %d\r\n", (int)len);  // 输出: 4
```

### 2. 循环读取直到缓冲区为空

```c
uint8_t data[2];
lwrb_sz_t len;

/* 写入 4 字节数据 */
lwrb_write(&buff, "0123", 4);

/* 循环读取直到缓冲区为空 */
while ((len = lwrb_read(&buff, data, sizeof(data))) > 0) {
    printf("Successfully read %d bytes\r\n", (int)len);
}
```

### 3. DMA 写入（使用 advance）

处理缓冲区回绕情况，分两次写入：

```c
lwrb_t buff;
uint8_t buff_data[8];
uint8_t* data;
lwrb_sz_t len;

lwrb_init(&buff, buff_data, sizeof(buff_data));

/* 场景：R=4, W=4，缓冲区为空 */

/* 第一次写入：获取线性可写块 */
if ((len = lwrb_get_linear_block_write_length(&buff)) > 0) {
    data = lwrb_get_linear_block_write_address(&buff);  // 返回 &buff_data[4]

    /* 通过 DMA 或 memcpy 写入数据 */
    receive_data(data, len);

    /* 推进写指针 */
    lwrb_advance(&buff, len);
}

/* 第二次写入：可能回绕到缓冲区开头 */
if ((len = lwrb_get_linear_block_write_length(&buff)) > 0) {
    data = lwrb_get_linear_block_write_address(&buff);  // 返回 &buff_data[0]

    /* 写入数据 */
    receive_data(data, len);

    /* 推进写指针 */
    lwrb_advance(&buff, len);
}
```

### 4. DMA 读取（使用 skip）

处理缓冲区回绕情况，分两次读取：

```c
lwrb_t buff;
uint8_t buff_data[8];
uint8_t* data;
lwrb_sz_t len;

lwrb_init(&buff, buff_data, sizeof(buff_data));

/* 场景：R=5, W=4，缓冲区满 */

/* 第一次读取 */
if ((len = lwrb_get_linear_block_read_length(&buff)) > 0) {
    data = lwrb_get_linear_block_read_address(&buff);  // 返回 &buff_data[5]

    /* 通过 DMA 发送数据 */
    send_data(data, len);

    /* 跳过已发送的数据 */
    lwrb_skip(&buff, len);
}

/* 第二次读取：可能回绕到缓冲区开头 */
if ((len = lwrb_get_linear_block_read_length(&buff)) > 0) {
    data = lwrb_get_linear_block_read_address(&buff);  // 返回 &buff_data[0]

    /* 发送数据 */
    send_data(data, len);

    /* 跳过已发送的数据 */
    lwrb_skip(&buff, len);
}
```

### 5. DMA 读取完整示例（含中断处理）

```c
lwrb_t buff;
uint8_t buff_data[8];
volatile size_t len;  /* DMA 传输长度 */

void send_data(void);

int main(void) {
    lwrb_init(&buff, buff_data, sizeof(buff_data));
    lwrb_write(&buff, "0123", 4);

    /* 启动 DMA 传输 */
    send_data();

    while (1);
}

/* 发送数据函数 */
void send_data(void) {
    /* 如果 len > 0，说明 DMA 正在传输 */
    if (len > 0) {
        return;
    }

    /* 获取线性可读块长度 */
    len = lwrb_get_linear_block_read_length(&buff);
    if (len > 0) {
        /* 获取可读数据地址 */
        uint8_t* data = lwrb_get_linear_block_read_address(&buff);

        /* 启动 DMA 传输 */
        start_dma_transfer(data, len);
    }
}

/* DMA 传输完成中断 */
void DMA_Interrupt_handler(void) {
    if (len > 0) {
        /* 跳过已传输的数据 */
        lwrb_skip(&buff, len);

        /* 重置传输标志 */
        len = 0;

        /* 尝试发送更多数据 */
        send_data();
    }
}
```

### 6. 限制最大长度的 DMA 读取

```c
size_t len;
uint8_t* data;
const size_t max_len = 4;  /* DMA 最大传输长度 */

/* 循环读取所有数据 */
while ((len = lwrb_get_linear_block_read_length(&buff)) > 0) {
    /* 限制最大传输长度 */
    if (len > max_len) {
        len = max_len;
    }

    /* 获取读取地址 */
    data = lwrb_get_linear_block_read_address(&buff);

    /* 启动 DMA 传输 */
    send_data(data, len);

    /* 跳过已传输的数据 */
    lwrb_skip(&buff, len);
}
```

### 7. 事件回调

```c
void my_buff_evt_fn(lwrb_t* buff, lwrb_evt_type_t type, size_t len) {
    switch (type) {
        case LWRB_EVT_RESET:
            printf("[EVT] Buffer reset event!\r\n");
            break;
        case LWRB_EVT_READ:
            printf("[EVT] Buffer read event: %d byte(s)!\r\n", (int)len);
            break;
        case LWRB_EVT_WRITE:
            printf("[EVT] Buffer write event: %d byte(s)!\r\n", (int)len);
            break;
        default:
            break;
    }
}

/* 初始化并设置事件回调 */
lwrb_t buff;
uint8_t buff_data[8];
lwrb_init(&buff, buff_data, sizeof(buff_data));
lwrb_set_evt_fn(&buff, my_buff_evt_fn);
```

### 8. 线程安全（多写多读场景）

```c
lwrb_t rb;
mutex_t m_w, m_r;  /* 写和读互斥锁 */

/* 写线程 1 */
void thread_write_1(void* arg) {
    while (1) {
        mutex_get(&m_w);
        lwrb_write(&rb, data1, len1);
        mutex_give(&m_w);
    }
}

/* 写线程 2 */
void thread_write_2(void* arg) {
    while (1) {
        mutex_get(&m_w);
        lwrb_write(&rb, data2, len2);
        mutex_give(&m_w);
    }
}

/* 读线程 1 */
void thread_read_1(void* arg) {
    while (1) {
        mutex_get(&m_r);
        lwrb_read(&rb, buf, sizeof(buf));
        mutex_give(&m_r);
    }
}

/* 读线程 2 */
void thread_read_2(void* arg) {
    while (1) {
        mutex_get(&m_r);
        lwrb_read(&rb, buf, sizeof(buf));
        mutex_give(&m_r);
    }
}
```

### 9. 缓冲区大小示例（演示 +1 的重要性）

```c
#define N  3  /* 数据块数量 */

/* 自定义数据结构：8 字节 */
typedef struct {
    uint32_t a;
    uint32_t b;
} data_t;

/* 缓冲区 1：正确分配（+1） */
lwrb_t buff_1;
uint8_t buff_data_1[sizeof(data_t) * N + 1];  /* 25 字节 */

/* 缓冲区 2：未分配 +1 */
lwrb_t buff_2;
uint8_t buff_data_2[sizeof(data_t) * N];  /* 24 字节 */

/* 初始化 */
lwrb_init(&buff_1, buff_data_1, sizeof(buff_data_1));
lwrb_init(&buff_2, buff_data_2, sizeof(buff_data_2));

/* 写入 3 个数据块 */
for (size_t i = 0; i < N; ++i) {
    data_t d;
    d.a = i;
    d.b = i * 2;

    lwrb_sz_t len_1 = lwrb_write(&buff_1, &d, sizeof(d));
    lwrb_sz_t len_2 = lwrb_write(&buff_2, &d, sizeof(d));

    printf("Buff1: %d/%d written; Buff2: %d/%d written\r\n",
           (int)len_1, (int)sizeof(d),
           (int)len_2, (int)sizeof(d));
}

/*
输出：
Buff1: 8/8 written; Buff2: 8/8 written
Buff1: 8/8 written; Buff2: 8/8 written
Buff1: 8/8 written; Buff2: 0/8 written  ← 第三个写入失败
*/
```

### 10. Peek 操作示例

```c
uint8_t peek_buff[8];
lwrb_sz_t peek_len;

/* 写入数据 */
lwrb_write(&buff, "ABCDEFGH", 8);

/* 查看前 8 字节 */
peek_len = lwrb_peek(&buff, 0, peek_buff, 8);
printf("Peeked: %.*s\r\n", (int)peek_len, peek_buff);  // "ABCDEFGH"

/* 跳过 3 字节后查看 5 字节 */
peek_len = lwrb_peek(&buff, 3, peek_buff, 5);
printf("Peeked: %.*s\r\n", (int)peek_len, peek_buff);  // "DEFGH"

/* Peek 不会移动读指针，缓冲区仍有 8 字节 */
printf("Buffer full: %d\r\n", (int)lwrb_get_full(&buff));  // 8
```

### 11. 搜索数据示例

```c
lwrb_sz_t found_idx;
uint8_t found;

/* 准备数据 */
lwrb_write(&buff, "12345678", 8);

/* 搜索 "123" */
found = lwrb_find(&buff, "123", 3, 0, &found_idx);
if (found) {
    printf("Found '123' at index %d\r\n", (int)found_idx);  // 0
}

/* 从偏移 1 开始搜索 */
found = lwrb_find(&buff, "123", 3, 1, &found_idx);
if (!found) {
    printf("Not found (expected)\r\n");
}
```

### 12. Overwrite（覆盖写入）示例

```c
uint8_t tmp[8];

/* 初始化并写入 6 字节 */
lwrb_reset(&buff);
lwrb_write(&buff, "abcdef", 6);

/* 覆盖写入，空间不足时丢弃最早的数据 */
lwrb_overwrite(&buff, "0", 1);
lwrb_peek(&buff, 0, tmp, 8);
printf("Buffer: %.*s\r\n", 8, tmp);  // "abcdef0"

lwrb_overwrite(&buff, "1", 1);
lwrb_peek(&buff, 0, tmp, 8);
printf("Buffer: %.*s\r\n", 8, tmp);  // "abcdef01"

lwrb_overwrite(&buff, "2", 1);
lwrb_peek(&buff, 0, tmp, 8);
printf("Buffer: %.*s\r\n", 8, tmp);  // "bcdef012" （最老的 'a' 被覆盖）
```

### 13. 数据移动（Move）示例

```c
lwrb_t src, dst;
uint8_t src_data[16], dst_data[8];
lwrb_sz_t moved;

/* 初始化 */
lwrb_init(&src, src_data, sizeof(src_data));
lwrb_init(&dst, dst_data, sizeof(dst_data));

/* 源写入 6 字节 */
lwrb_reset(&src);
lwrb_reset(&dst);
lwrb_write(&src, "012345", 6);

/* 移动数据 */
moved = lwrb_move(&dst, &src);
printf("Moved %d bytes\r\n", (int)moved);  // 6

/* src 为空，dst 有数据 */
printf("Src full: %d, Dst full: %d\r\n",
       (int)lwrb_get_full(&src),
       (int)lwrb_get_full(&dst));  // 0, 6
```

### 14. 基本读写（完整）

```c
uint8_t buffer_data[32];
lwrb_t buff;

/* 初始化 */
lwrb_init(&buff, buffer_data, sizeof(buffer_data));

/* 写入数据 */
lwrb_sz_t written = lwrb_write(&buff, "Hello", 5);

/* 读取数据 */
uint8_t read_data[10];
lwrb_sz_t read = lwrb_read(&buff, read_data, sizeof(read_data));

/* 查询状态 */
lwrb_sz_t free_space = lwrb_get_free(&buff);
lwrb_sz_t used_space = lwrb_get_full(&buff);
```

---

## 总结

LwRB 是一个高效、轻量的循环缓冲区库，特别适合嵌入式系统和 DMA 场景。正确理解其 API 和注意事项能够有效避免常见的使用错误，提高系统的可靠性和性能。
