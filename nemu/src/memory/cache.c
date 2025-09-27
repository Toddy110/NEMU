#include "memory/cache.h"
#include <stdlib.h>

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

#define CACHE_SIZE        (64 * 1024)
#define CACHE_BLOCK_SIZE  64
#define CACHE_WAYS        8

#define CACHE_NR_BLOCKS   (CACHE_SIZE / CACHE_BLOCK_SIZE)
#define CACHE_NR_SETS     (CACHE_NR_BLOCKS / CACHE_WAYS)

typedef struct CacheBlock {
	uint8_t data[CACHE_BLOCK_SIZE];
	uint32_t tag;
	uint8_t valid;
} CacheBlock;

static CacheBlock cache[CACHE_NR_SETS][CACHE_WAYS];

void init_cache(void) {
	int s, w;
	for (s = 0; s < CACHE_NR_SETS; s++) {
		for (w = 0; w < CACHE_WAYS; w++) {
			cache[s][w].valid = 0;
			cache[s][w].tag = 0;
		}
	}
}

static inline unsigned rand_index(unsigned set) {
	static uint32_t seed = 1;
	seed = seed * 1103515245 + 12345 + set;
	return (seed >> 16) & (CACHE_WAYS - 1);
}

static inline CacheBlock * cache_fetch_block(hwaddr_t addr) {
	uint32_t block_addr = addr / CACHE_BLOCK_SIZE;
	uint32_t set = block_addr % CACHE_NR_SETS;
	uint32_t tag = block_addr / CACHE_NR_SETS;
	int w;
	for (w = 0; w < CACHE_WAYS; w++) {
		if (cache[set][w].valid && cache[set][w].tag == tag) return &cache[set][w];
	}
	int victim = rand_index(set);
	CacheBlock *blk = &cache[set][victim];
	blk->valid = 1;
	blk->tag = tag;
	uint32_t base = block_addr * CACHE_BLOCK_SIZE;
	int i;
	for (i = 0; i < CACHE_BLOCK_SIZE; i += 4) {
		uint32_t word = dram_read(base + i, 4);
		blk->data[i] = word & 0xff;
		blk->data[i + 1] = (word >> 8) & 0xff;
		blk->data[i + 2] = (word >> 16) & 0xff;
		blk->data[i + 3] = (word >> 24) & 0xff;
	}
	return blk;
}

static uint32_t cache_read_internal(hwaddr_t addr, size_t len) {
	uint32_t end = (addr & ~(CACHE_BLOCK_SIZE - 1)) + CACHE_BLOCK_SIZE;
	if (addr + len <= end) {
		CacheBlock *blk = cache_fetch_block(addr);
		uint32_t off = addr & (CACHE_BLOCK_SIZE - 1);
		uint32_t val = 0; int i;
		for (i = 0; i < (int)len; i++) val |= (uint32_t)blk->data[off + i] << (8 * i);
		return val & (~0u >> ((4 - len) << 3));
	}
	else {
		uint32_t first_len = end - addr;
		uint32_t low = cache_read_internal(addr, first_len);
		uint32_t high = cache_read_internal(addr + first_len, len - first_len);
		return low | (high << (first_len * 8));
	}
}

static void cache_write_internal(hwaddr_t addr, size_t len, uint32_t data) {
	uint32_t block_addr = addr / CACHE_BLOCK_SIZE;
	uint32_t set = block_addr % CACHE_NR_SETS;
	uint32_t tag = block_addr / CACHE_NR_SETS;
	int w; CacheBlock *hit = NULL;
	for (w = 0; w < CACHE_WAYS; w++) {
		if (cache[set][w].valid && cache[set][w].tag == tag) { hit = &cache[set][w]; break; }
	}
	uint32_t end = (addr & ~(CACHE_BLOCK_SIZE - 1)) + CACHE_BLOCK_SIZE;
	if (addr + len <= end) {
		if (hit) {
			uint32_t off = addr & (CACHE_BLOCK_SIZE - 1); int i;
			for (i = 0; i < (int)len; i++) hit->data[off + i] = (data >> (8 * i)) & 0xff;
		}
		dram_write(addr, len, data);
	}
	else {
		uint32_t first_len = end - addr;
		uint32_t low_mask = (~0u) >> ((4 - first_len) << 3);
		uint32_t low_part = data & low_mask;
		uint32_t high_part = data >> (first_len * 8);
		cache_write_internal(addr, first_len, low_part);
		cache_write_internal(addr + first_len, len - first_len, high_part);
	}
}

uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len) { return cache_read_internal(addr, len); }
void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) { cache_write_internal(addr, len, data); }
