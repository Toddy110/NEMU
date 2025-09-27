/* 与新的 cache.h 同步使用宏/结构 */
#include "memory/cache.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 外部 DRAM 接口 */
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* 直接使用 cache.h 中的宏，不再重新定义辅助宏 */

/* 头文件已定义：
 * CACHE_b, CACHE_s, CACHE_e
 * CACHE_B = 1<<b, CACHE_E = 1<<e, CACHE_S = 1<<s
 * CacheBlock, cache_L1[]
 */

/* 使用 srand(time(0)) + rand() 生成随机替换序号 */

void init_cache(void) {
  int i;
  /* 初始化随机种子 */
  srand(time(0));
  for (i = 0; i < CACHE_S * CACHE_E; i++) {
    cache_L1[i].valid = 0;
    cache_L1[i].tag = 0;
  }
}

/* 查找/装填：返回块在一维数组中的索引 */
static int read_cache_L1(hwaddr_t addr) {
  uint32_t set = (addr >> CACHE_b) & (CACHE_S - 1);
  uint32_t tag = addr >> (CACHE_b + CACHE_s);
  int set_begin = set * CACHE_E;
  int set_end = set_begin + CACHE_E;
  int i;
  for (i = set_begin; i < set_end; i++) {
    if (cache_L1[i].valid && cache_L1[i].tag == tag) return i; /* Hit */
  }
  /* Miss: 找空位 */
  for (i = set_begin; i < set_end; i++) {
    if (!cache_L1[i].valid) break;
  }
  if (i == set_end) {
    /* 随机替换 */
    uint32_t r = (uint32_t)rand();
    i = set_begin + (r % CACHE_E);
  }
  /* 填充：对齐到块起始地址，从 DRAM 读取 */
  uint32_t block_start = (addr >> CACHE_b) << CACHE_b;
  int off;
  for (off = 0; off < CACHE_B; off += 4) {
    uint32_t word = dram_read(block_start + off, 4);
    cache_L1[i].data[off]     = word & 0xff;
    cache_L1[i].data[off + 1] = (word >> 8) & 0xff;
    cache_L1[i].data[off + 2] = (word >> 16) & 0xff;
    cache_L1[i].data[off + 3] = (word >> 24) & 0xff;
  }
  cache_L1[i].valid = 1;
  cache_L1[i].tag = tag;
  return i;
}

/* 写一级缓存(命中才更新)，不写分配，直写到 DRAM */
static void write_cache_L1(hwaddr_t addr, size_t len, uint32_t data) {
  uint32_t set = (addr >> CACHE_b) & (CACHE_S - 1);
  uint32_t tag = addr >> (CACHE_b + CACHE_s);
  uint32_t block_bias = addr & (CACHE_B - 1);
  int set_begin = set * CACHE_E;
  int set_end = set_begin + CACHE_E;
  int i;
  for (i = set_begin; i < set_end; i++) {
    if (cache_L1[i].valid && cache_L1[i].tag == tag) {
      /* 命中：块内偏移写，考虑跨块 */
      if (block_bias + len > CACHE_B) {
        /* 第一块部分 */
        int first = CACHE_B - block_bias;
        int j;
        for (j = 0; j < first; j++) cache_L1[i].data[block_bias + j] = (data >> (8 * j)) & 0xff;
        dram_write(addr, first, data & (~0u >> ((4 - first) << 3)));
        /* 递归写余下部分 */
        write_cache_L1(addr + first, len - first, data >> (8 * first));
      } else {
        int j;
        for (j = 0; j < (int)len; j++) cache_L1[i].data[block_bias + j] = (data >> (8 * j)) & 0xff;
        dram_write(addr, len, data);
      }
      return;
    }
  }
  /* 未命中：不写分配，直接写 DRAM (可能跨块) */
  uint32_t block_end = (addr & ~(CACHE_B - 1)) + CACHE_B;
  if (addr + len <= block_end) {
    dram_write(addr, len, data);
    return;
  }
  uint32_t first = block_end - addr;
  uint32_t low_mask = (~0u) >> ((4 - first) << 3);
  uint32_t low_part = data & low_mask;
  uint32_t high_part = data >> (8 * first);
  dram_write(addr, first, low_part);
  write_cache_L1(addr + first, len - first, high_part);
}

/* 对外读取接口 (适配 cache.h -> cache_hwaddr_read) */
static uint32_t read_L1(hwaddr_t addr, size_t len) {
  uint32_t block_bias = addr & (CACHE_B - 1);
  int idx = read_cache_L1(addr);
  if (block_bias + len > CACHE_B) {
    int first = CACHE_B - block_bias;
    uint32_t low = read_L1(addr, first);
    uint32_t high = read_L1(addr + first, len - first);
    return low | (high << (8 * first));
  }
  /* 逐字节拼装，避免别名警告 */
  uint32_t val = 0;
  int i;
  for (i = 0; i < (int)len; i++) {
    val |= (uint32_t)cache_L1[idx].data[block_bias + i] << (8 * i);
  }
  return val & (~0u >> ((4 - len) << 3));
}

uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len) { return read_L1(addr, len); }
void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) { write_cache_L1(addr, len, data); }

