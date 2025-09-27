#include "memory/cache.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

void init_cache(void) {
  int i;
  srand(time(0));
  for (i = 0; i < CACHE_S * CACHE_E; i++) {
    cache_L1[i].valid = 0;
    cache_L1[i].tag = 0;
  }
}
static int read_cache_L1(hwaddr_t addr) {
  uint32_t set = (addr >> CACHE_b) & (CACHE_S - 1);
  uint32_t tag = addr >> (CACHE_b + CACHE_s);
  int set_begin = set * CACHE_E;
  int set_end = set_begin + CACHE_E;
  int i;
  for (i = set_begin; i < set_end; i++) {
    if (cache_L1[i].valid && cache_L1[i].tag == tag) return i;
  }
  for (i = set_begin; i < set_end; i++) {
    if (!cache_L1[i].valid) break;
  }
  if (i == set_end) {
    uint32_t r = (uint32_t)rand();
    i = set_begin + (r % CACHE_E);
  }
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
static void write_cache_L1(hwaddr_t addr, size_t len, uint32_t data) {
  uint32_t set = (addr >> CACHE_b) & (CACHE_S - 1);
  uint32_t tag = addr >> (CACHE_b + CACHE_s);
  uint32_t block_bias = addr & (CACHE_B - 1);
  int set_begin = set * CACHE_E;
  int set_end = set_begin + CACHE_E;
  int i;
  for (i = set_begin; i < set_end; i++) {
    if (cache_L1[i].valid && cache_L1[i].tag == tag) {
      if (block_bias + len > CACHE_B) {
        int first = CACHE_B - block_bias;
        int j;
        for (j = 0; j < first; j++) cache_L1[i].data[block_bias + j] = (data >> (8 * j)) & 0xff;
        dram_write(addr, first, data & (~0u >> ((4 - first) << 3)));
        write_cache_L1(addr + first, len - first, data >> (8 * first));
      } else {
        int j;
        for (j = 0; j < (int)len; j++) cache_L1[i].data[block_bias + j] = (data >> (8 * j)) & 0xff;
        dram_write(addr, len, data);
      }
      return;
    }
  }
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
static uint32_t read_L1(hwaddr_t addr, size_t len) {
  uint32_t block_bias = addr & (CACHE_B - 1);
  int idx = read_cache_L1(addr);
  if (block_bias + len > CACHE_B) {
    int first = CACHE_B - block_bias;
    uint32_t low = read_L1(addr, first);
    uint32_t high = read_L1(addr + first, len - first);
    return low | (high << (8 * first));
  }
  uint32_t val = 0;
  int i;
  for (i = 0; i < (int)len; i++) {
    val |= (uint32_t)cache_L1[idx].data[block_bias + i] << (8 * i);
  }
  return val & (~0u >> ((4 - len) << 3));
}

uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len){ 
    return read_L1(addr, len); 
}
void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data){ 
    write_cache_L1(addr, len, data); 
}

