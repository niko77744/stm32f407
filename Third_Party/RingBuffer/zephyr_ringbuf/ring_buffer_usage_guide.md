# Zephyr Ring Buffer 使用指南

## 目录
- [概述](#概述)
- [基本概念](#基本概念)
- [初始化方式](#初始化方式)
- [使用案例](#使用案例)
- [API 参考](#api-参考)
- [注意事项](#注意事项)

---

## 概述

Zephyr Ring Buffer 是一个高效的环形缓冲区实现，支持两种模式：
- **字节模式**：基本的数据读写，最小单位为字节
- **Item 模式**：结构化数据，每个数据项包含类型标识、整数值和数据

---

## 基本概念

### 环形缓冲区原理
环形缓冲区是一个先进先出（FIFO）的数据结构，当写指针追上读指针时表示缓冲区满，当读指针追上写指针时表示缓冲区空。

### 关键概念
- **容量（Capacity）**：缓冲区的总大小
- **已用空间**：当前存储的数据量
- **剩余空间**：可用的空闲空间

---

## 初始化方式

### 方式一：使用宏声明（推荐）

#### 字节模式
```c
RING_BUF_DECLARE(buffer_name, size8);
```
- `buffer_name`：缓冲区变量名
- `size8`：缓冲区大小（字节）

**示例：**
```c
RING_BUF_DECLARE(my_buffer, 128);  // 创建128字节的环形缓冲区
```

#### Item 模式
```c
RING_BUF_ITEM_DECLARE(buffer_name, size32);
```
- `buffer_name`：缓冲区变量名
- `size32`：缓冲区大小（32位字数）

**示例：**
```c
RING_BUF_ITEM_DECLARE(item_buffer, 64);  // 创建64个32位字的环形缓冲区（256字节）
```

### 方式二：手动初始化

#### 字节模式
```c
uint8_t storage_buffer[256];
struct ring_buf my_ring_buf;

ring_buf_init(&my_ring_buf, 256, storage_buffer);
```

#### Item 模式
```c
uint32_t storage_buffer[64];
struct ring_buf item_ring_buf;

ring_buf_item_init(&item_ring_buf, 64, storage_buffer);
```

---

## 使用案例

### 案例 1：字节模式 - 基本读写

```c
#include "ring_buffer.h"
#include <stdio.h>

RING_BUF_DECLARE(my_buffer, 128);

void example_basic_io(void) {
    // 写入数据
    uint8_t data[] = "Hello, World!";
    uint32_t written = ring_buf_put(&my_buffer, data, sizeof(data) - 1);
    printf("写入了 %u 字节\n", written);

    // 读取数据
    uint8_t read_buf[64] = {0};
    uint32_t read_count = ring_buf_get(&my_buffer, read_buf, sizeof(read_buf));
    printf("读取了 %u 字节: %s\n", read_count, read_buf);
}
```

### 案例 2：检查缓冲区状态

```c
void example_status_check(void) {
    // 检查是否为空
    if (ring_buf_is_empty(&my_buffer)) {
        printf("缓冲区为空\n");
    }

    // 获取已用空间
    uint32_t used = ring_buf_size_get(&my_buffer);
    printf("已使用: %u 字节\n", used);

    // 获取剩余空间
    uint32_t free = ring_buf_space_get(&my_buffer);
    printf("剩余空间: %u 字节\n", free);

    // 获取总容量
    uint32_t capacity = ring_buf_capacity_get(&my_buffer);
    printf("总容量: %u 字节\n", capacity);
}
```

### 案例 3：零拷贝模式 (Claim/Finish)

```c
void example_zero_copy(void) {
    // 零拷贝写入
    uint8_t *write_ptr;
    uint32_t size = ring_buf_put_claim(&my_buffer, &write_ptr, 64);
    if (size > 0) {
        // 直接写入到缓冲区，避免 memcpy
        for (uint32_t i = 0; i < size; i++) {
            write_ptr[i] = i * 2;
        }
        // 提交写入
        ring_buf_put_finish(&my_buffer, size);
    }

    // 零拷贝读取
    uint8_t *read_ptr;
    size = ring_buf_get_claim(&my_buffer, &read_ptr, 32);
    if (size > 0) {
        // 直接处理数据
        printf("数据: ");
        for (uint32_t i = 0; i < size; i++) {
            printf("%02X ", read_ptr[i]);
        }
        printf("\n");
        // 标记已读取
        ring_buf_get_finish(&my_buffer, size);
    }
}
```

### 案例 4：Item 模式 - 结构化数据

```c
void example_item_mode(void) {
    RING_BUF_ITEM_DECLARE(item_buffer, 64);

    // 写入数据项
    uint32_t sensor_data[] = {100, 200, 300, 400};
    int ret = ring_buf_item_put(&item_buffer, 1, 10, sensor_data, 4);
    // 参数: type=1, value=10, data=sensor_data, size=4个32位字

    if (ret == 0) {
        printf("数据项写入成功\n");
    } else if (ret == -EMSGSIZE) {
        printf("缓冲区空间不足\n");
    }

    // 读取数据项
    uint16_t type;
    uint8_t value;
    uint32_t received_data[10];
    uint8_t received_size = 10;  // 期望接收10个32位字

    ret = ring_buf_item_get(&item_buffer, &type, &value, received_data, &received_size);
    if (ret == 0) {
        printf("type=%u, value=%u, 数据项大小=%u\n", type, value, received_size);
        for (int i = 0; i < received_size; i++) {
            printf("%u ", received_data[i]);
        }
        printf("\n");
    } else if (ret == -EAGAIN) {
        printf("缓冲区为空\n");
    } else if (ret == -EMSGSIZE) {
        printf("接收缓冲区太小，需要 %u 个32位字\n", received_size);
    }
}
```

### 案例 5：Peek 操作（不删除数据）

```c
void example_peek(void) {
    // 写入数据
    uint8_t data[] = "Test Data";
    ring_buf_put(&my_buffer, data, sizeof(data) - 1);

    // Peek - 查看但不删除
    uint8_t peek_buf[64] = {0};
    uint32_t peeked = ring_buf_peek(&my_buffer, peek_buf, sizeof(peek_buf));
    printf("Peek 数据: %s\n", peek_buf);

    // 再次 Peek，数据仍然存在
    peeked = ring_buf_peek(&my_buffer, peek_buf, sizeof(peek_buf));
    printf("再次 Peek: %s\n", peek_buf);

    // 用 get 实际读取并删除
    uint32_t got = ring_buf_get(&my_buffer, peek_buf, sizeof(peek_buf));
    printf("Get 数据: %s\n", peek_buf);

    // 再 peek，缓冲区空了
    peeked = ring_buf_peek(&my_buffer, peek_buf, sizeof(peek_buf));
    printf("缓冲区空后 Peek: %u 字节\n", peeked);
}
```

### 案例 6：重置缓冲区

```c
void example_reset(void) {
    // 重置缓冲区到初始状态（清空所有数据）
    ring_buf_reset(&my_buffer);
    printf("缓冲区已重置\n");
}
```

### 案例 7：生产者-消费者模式

```c
#include <pthread.h>

RING_BUF_DECLARE(shared_buffer, 512);

// 生产者线程
void *producer(void *arg) {
    uint8_t data[64];
    for (int i = 0; i < 100; i++) {
        // 准备数据
        snprintf((char *)data, sizeof(data), "Message %d", i);

        // 等待空间
        while (ring_buf_space_get(&shared_buffer) < strlen((char *)data)) {
            // 可以加入延时或让出CPU
        }

        // 写入数据
        ring_buf_put(&shared_buffer, data, strlen((char *)data));
    }
    return NULL;
}

// 消费者线程
void *consumer(void *arg) {
    uint8_t data[64];
    while (1) {
        // 等待数据
        while (ring_buf_is_empty(&shared_buffer)) {
            // 可以加入延时或让出CPU
        }

        // 读取数据
        uint32_t count = ring_buf_get(&shared_buffer, data, sizeof(data) - 1);
        data[count] = '\0';
        printf("接收到: %s\n", data);
    }
    return NULL;
}
```

---

## API 参考

### 初始化和状态查询

| 函数 | 说明 | 返回值 |
|------|------|--------|
| `RING_BUF_DECLARE(name, size8)` | 声明并初始化字节缓冲区 | - |
| `RING_BUF_ITEM_DECLARE(name, size32)` | 声明并初始化 item 缓冲区 | - |
| `ring_buf_init(buf, size, data)` | 初始化字节缓冲区 | - |
| `ring_buf_item_init(buf, size, data)` | 初始化 item 缓冲区 | - |
| `ring_buf_is_empty(buf)` | 判断是否为空 | true/false |
| `ring_buf_size_get(buf)` | 获取已用空间（字节） | 字节数 |
| `ring_buf_space_get(buf)` | 获取剩余空间（字节） | 字节数 |
| `ring_buf_item_space_get(buf)` | 获取 item 模式剩余空间（32位字） | 32位字数 |
| `ring_buf_capacity_get(buf)` | 获取总容量 | 字节数 |
| `ring_buf_reset(buf)` | 重置缓冲区 | - |

### 字节数据操作

| 函数 | 说明 | 返回值 |
|------|------|--------|
| `ring_buf_put(buf, data, size)` | 写入字节数据 | 实际写入字节数 |
| `ring_buf_get(buf, data, size)` | 读取并删除字节数据 | 实际读取字节数 |
| `ring_buf_peek(buf, data, size)` | 查看（不删除）字节数据 | 实际读取字节数 |
| `ring_buf_put_claim(buf, &data, size)` | 预申请写入空间 | 可用空间大小 |
| `ring_buf_put_finish(buf, size)` | 提交写入 | 0/-EINVAL |
| `ring_buf_get_claim(buf, &data, size)` | 预申请读取空间 | 可读数据大小 |
| `ring_buf_get_finish(buf, size)` | 提交读取 | 0/-EINVAL |

### Item 数据操作

| 函数 | 说明 | 返回值 |
|------|------|--------|
| `ring_buf_item_put(buf, type, value, data, size32)` | 写入结构化数据项 | 0/-EMSGSIZE |
| `ring_buf_item_get(buf, &type, &value, data, &size32)` | 读取结构化数据项 | 0/-EAGAIN/-EMSGSIZE |

---

## 注意事项

### 1. 线程安全

**⚠️ 多线程使用时需要注意：**

- **多写入者**：必须防止并发写入操作
  - 使用互斥锁保护写入操作
  - 或禁用中断/调度
- **多读取者**：必须防止并发读取操作
  - 使用互斥锁保护读取操作
  - 或禁用中断/调度

```c
// 错误示例 - 两个线程同时写入
void thread1_writer(void) {
    ring_buf_put(&buf, data1, size1);  // 不安全！
}

void thread2_writer(void) {
    ring_buf_put(&buf, data2, size2);  // 不安全！
}

// 正确示例 - 使用互斥锁
pthread_mutex_t write_mutex;

void safe_thread1_writer(void) {
    pthread_mutex_lock(&write_mutex);
    ring_buf_put(&buf, data1, size1);
    pthread_mutex_unlock(&write_mutex);
}

void safe_thread2_writer(void) {
    pthread_mutex_lock(&write_mutex);
    ring_buf_put(&buf, data2, size2);
    pthread_mutex_unlock(&write_mutex);
}
```

### 2. 模式混用

**⚠️ 不要在同一个环形缓冲区实例上混合使用字节模式和 item 模式：**

```c
// 错误示例
RING_BUF_DECLARE(mixed_buffer, 128);
ring_buf_put(&mixed_buffer, data1, 10);          // 字节模式写入
ring_buf_item_put(&mixed_buffer, type, val, data, size);  // ❌ 不要这样做！
```

### 3. Claim/Finish 配对使用

**⚠️ `ring_buf_put_claim()` 必须与 `ring_buf_put_finish()` 配对使用：**

```c
// 错误示例
uint8_t *ptr;
ring_buf_put_claim(&buf, &ptr, 10);
// 忘记调用 put_finish - 数据不会被提交！

// 正确示例
uint8_t *ptr;
uint32_t size = ring_buf_put_claim(&buf, &ptr, 10);
if (size > 0) {
    memcpy(ptr, data, size);
    ring_buf_put_finish(&buf, size);  // 必须调用
}
```

**⚠️ 同样，`ring_buf_get_claim()` 必须与 `ring_buf_get_finish()` 配对使用。**

### 4. Peek 操作的特点

**⚠️ Peek 不会删除数据，多次调用会返回相同数据：**

```c
ring_buf_put(&buf, "Hello", 5);

ring_buf_peek(&buf, out1, 10);  // 返回 "Hello"
ring_buf_peek(&buf, out2, 10);  // 再次返回 "Hello" - 数据还在！

// 要删除数据，必须使用 get 或 get_finish
ring_buf_get(&buf, out3, 10);   // 读取并删除
```

### 5. 缓冲区大小限制

**⚠️ 缓冲区大小有最大限制：**

- 默认模式：最大 `UINT16_MAX / 2` 字节（约 32KB）
- 大缓冲区模式（`CONFIG_RING_BUFFER_LARGE`）：最大 `UINT32_MAX / 2` 字节（约 2GB）

### 6. 写入大小

**⚠️ `ring_buf_put()` 可能写入的数据少于请求的大小：**

```c
uint32_t to_write = 100;
uint32_t written = ring_buf_put(&buf, data, to_write);

if (written != to_write) {
    printf("只写入了 %u/%u 字节\n", written, to_write);
    // 需要处理部分写入的情况
}
```

### 7. Item 模式的数据大小

**⚠️ Item 模式中，数据项最大为 255 个 32 位字（1020 字节）：**

```c
// Item 模式使用 8 位长度字段（最大 255）
// 最大数据大小 = 255 * 4 = 1020 字节

uint32_t large_data[300];  // 1200 字节 - 太大！
ring_buf_item_put(&buf, type, val, large_data, 300);  // 可能失败或导致问题
```

### 8. 读取缓冲区大小检查

**⚠️ `ring_buf_item_get()` 需要检查返回值和大小：**

```c
uint32_t received_data[10];
uint8_t received_size = 10;

int ret = ring_buf_item_get(&buf, &type, &value, received_data, &received_size);

if (ret == 0) {
    // 成功，received_size 现在包含实际读取的 32 位字数
} else if (ret == -EMSGSIZE) {
    // 缓冲区太小，received_size 包含所需的大小
    printf("需要更大的缓冲区: %u 个 32 位字\n", received_size);
} else if (ret == -EAGAIN) {
    // 缓冲区为空
    printf("没有数据可读\n");
}
```

### 9. 数据为 NULL 的处理

**⚠️ 某些函数允许 data 参数为 NULL，但要注意：**

- `ring_buf_get(data, ...)`: data 可以为 NULL，用于丢弃数据
- `ring_buf_peek(data, ...)`: data **不能**为 NULL

```c
// 正确 - 丢弃数据
ring_buf_get(&buf, NULL, 10);

// 错误 - Peek 需要 data 参数
ring_buf_peek(&buf, NULL, 10);  // ❌ 会断言失败！
```

### 10. 内存对齐（Item 模式）

**⚠️ Item 模式要求缓冲区以 32 位字为单位对齐：**

```c
// 正确 - 使用 uint32_t 数组
uint32_t storage[64];
ring_buf_item_init(&buf, 64, storage);

// 错误 - 使用未对齐的指针
uint8_t *unaligned = malloc(256);
ring_buf_item_init(&buf, 64, (uint32_t *)unaligned);  // 可能有问题！
```

### 11. 断言和错误处理

**⚠️ 某些函数会触发断言，必须确保参数有效：**

```c
// 这些会触发断言：
ring_buf_init(&buf, RING_BUFFER_MAX_SIZE + 1, data);  // 大小超限
ring_buf_get_finish(&buf, size + 1);                   // size 超过 claim 的大小
```

### 12. 性能优化建议

**✅ 推荐使用零拷贝模式以提高性能：**

```c
// 不推荐 - 每次写入都需要 memcpy
uint8_t data[1000];
ring_buf_put(&buf, data, 1000);  // 发生一次 memcpy

// 推荐 - 零拷贝
uint8_t *ptr;
uint32_t size = ring_buf_put_claim(&buf, &ptr, 1000);
if (size == 1000) {
    // 直接写入到 ptr，避免 memcpy
    generate_data_directly(ptr, size);
    ring_buf_put_finish(&buf, size);
}
```

---

## 总结

Zephyr Ring Buffer 是一个高效、灵活的环形缓冲区实现，适用于嵌入式系统和实时应用。正确使用它需要注意线程安全、模式选择和 API 配对等细节。遵循本文档的注意事项和最佳实践，可以充分发挥其性能优势。
