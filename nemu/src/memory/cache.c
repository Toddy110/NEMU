#include "memory/cache.h"
#include "macro.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

uint64_t cache_cycle = 0;
uint64_t cache_hit   = 0;
uint64_t cache_miss  = 0;

/* L1: 64KB total, 64B block, 8-way => 128 sets, 1024 lines */
L1 cache_L1[CACHE_L1_S * CACHE_L1_E];

static void seed_random_once(void) {
  static int inited = 0;
  if (!inited) {
    inited = 1;
    srand(0xC0FFEE);
  }
}

static uint32_t get_block_offset(hwaddr_t addr) {
  /* block_offset = addr & 0x3F */
  return addr & 0x3fu;
}

static uint32_t get_set_idx(hwaddr_t addr) {
  /* set_idx = (addr >> 6) & 0x7F */
  return (addr >> 6) & 0x7fu;
}

static uint32_t get_tag(hwaddr_t addr) {
  /* tag = addr >> 13 */
  return addr >> 13;
}

static hwaddr_t get_block_base(hwaddr_t addr) {
  return addr & ~0x3fu;
}

void init_cache(void) {
  int i;
  for (i = 0; i < (int)(CACHE_L1_S * CACHE_L1_E); i++) {
    cache_L1[i].validVal = false;
    cache_L1[i].tag = 0;
    memset(cache_L1[i].data, 0, CACHE_B);
  }

  cache_cycle = 0;
  cache_hit = 0;
  cache_miss = 0;
  seed_random_once();
}

static int l1_probe(hwaddr_t addr) {
  uint32_t set_idx = get_set_idx(addr);
  uint32_t tag = get_tag(addr);
  int base = (int)(set_idx * 8);
  int way;

  for (way = 0; way < 8; way++) {
    int idx = base + way;
    if (cache_L1[idx].validVal && cache_L1[idx].tag == tag) {
      return idx;
    }
  }
  return -1;
}

static int l1_fill(hwaddr_t addr) {
  uint32_t set_idx = get_set_idx(addr);
  uint32_t tag = get_tag(addr);
  int base = (int)(set_idx * 8);
  int victim;
  hwaddr_t base_addr;
  int off;

  seed_random_once();
  victim = base + (rand() % 8);

  base_addr = get_block_base(addr);
  for (off = 0; off < 64; off += 4) {
    uint32_t w = dram_read(base_addr + (hwaddr_t)off, 4);
    memcpy(cache_L1[victim].data + off, &w, 4);
  }

  cache_L1[victim].tag = tag;
  cache_L1[victim].validVal = true;
  return victim;
}

static uint8_t l1_read_u8(hwaddr_t addr) {
  int idx = l1_probe(addr);

  if (idx >= 0) {
    cache_hit++;
    cache_cycle += 2;
  } else {
    cache_miss++;
    cache_cycle += 200;
    idx = l1_fill(addr);
  }

  return cache_L1[idx].data[get_block_offset(addr)];
}

static void l1_write_u8(hwaddr_t addr, uint8_t val) {
  int idx = l1_probe(addr);

  if (idx >= 0) {
    /* hit: write-through + update cache line */
    cache_hit++;
    cache_cycle += 2;
    cache_L1[idx].data[get_block_offset(addr)] = val;
  } else {
    /* miss: not write allocate */
    cache_miss++;
    cache_cycle += 200;
  }

  /* write-through: always update DRAM */
  dram_write(addr, 1, val);
}

uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len) {
  uint8_t buf[4] = {0, 0, 0, 0};
  size_t i;
  uint32_t raw;
  unalign u;

  assert(len == 1 || len == 2 || len == 4);

  /* Cross-block is handled by reading byte-by-byte, then assembling via unalign_rw(). */
  for (i = 0; i < len; i++) {
    buf[i] = l1_read_u8(addr + (hwaddr_t)i);
  }

  memset(&u, 0, sizeof(u));
  memcpy(&u, buf, 4);
  raw = unalign_rw(&u, 4);
  return raw & (~0u >> ((4 - len) << 3));
}

void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
  size_t i;

  assert(len == 1 || len == 2 || len == 4);

  /* write-through + not write allocate; handle cross-block byte-by-byte */
  for (i = 0; i < len; i++) {
    uint8_t b = (data >> (8 * i)) & 0xff;
    l1_write_u8(addr + (hwaddr_t)i, b);
  }
}
