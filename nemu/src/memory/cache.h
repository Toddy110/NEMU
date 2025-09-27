#define CACHE_b 6
#define CACHE_e 3
#define CACHE_s 7
#define CACHE_CAP 65536
#define CACHE_B (1 << CACHE_b)
#define CACHE_E (1 << CACHE_e)
#define CACHE_S (1 << CACHE_s)

typedef struct{
    uint8_t data[CACHE_B];
    uint32_t tag;
    uint8_t valid;
} L1;

L1 cache[CACHE_S * CACHE_E];
