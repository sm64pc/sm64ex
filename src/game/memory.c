#include <PR/ultratypes.h>
#include <string.h>

#include <stdlib.h>

#include "sm64.h"

#define INCLUDED_FROM_MEMORY_C

#include "buffers/buffers.h"
#include "decompress.h"
#include "game_init.h"
#include "main.h"
#include "memory.h"
#include "segment_symbols.h"
#include "segments.h"

// round up to the next multiple
#define ALIGN4(val) (((val) + 0x3) & ~0x3)
#define ALIGN8(val) (((val) + 0x7) & ~0x7)
#define ALIGN16(val) (((val) + 0xF) & ~0xF)

struct MainPoolState {
    void *prev;
};

struct MainPoolBlock {
    struct MainPoolBlock *prev;
    struct MainPoolBlock *next;
    void (*releaseHandler)(void *addr);
};

struct AllocOnlyPoolBlock {
    struct AllocOnlyPoolBlock *prev;
#if !IS_64_BIT
    void *pad; // require 8 bytes alignment
#endif
};

struct AllocOnlyPool {
    struct AllocOnlyPoolBlock *lastBlock;
    u32 lastBlockSize;
    u32 lastBlockNextPos;
};

struct FreeListNode {
    struct FreeListNode *next;
};

struct AllocatedNode {
    s32 bin;
    s32 pad;
};

struct MemoryPool {
    struct AllocOnlyPool *allocOnlyPool;
    struct FreeListNode *bins[27];
};

extern uintptr_t sSegmentTable[32];
extern u32 sPoolFreeSpace;
extern u8 *sPoolStart;
extern u8 *sPoolEnd;
extern struct MainPoolBlock *sPoolListHeadL;
extern struct MainPoolBlock *sPoolListHeadR;


/**
 * Memory pool for small graphical effects that aren't connected to Objects.
 * Used for colored text, paintings, and environmental snow and bubbles.
 */
struct MemoryPool *gEffectsMemoryPool;

uintptr_t sSegmentTable[32];
u32 sPoolFreeSpace;
u8 *sPoolStart;
u8 *sPoolEnd;
struct MainPoolBlock *sPoolListHeadL;
struct MainPoolBlock *sPoolListHeadR;


static struct MainPoolState *gMainPoolState = NULL;

uintptr_t set_segment_base_addr(s32 segment, void *addr) {
    sSegmentTable[segment] = (uintptr_t) addr & 0x1FFFFFFF;
    return sSegmentTable[segment];
}

void *get_segment_base_addr(s32 segment) {
    return (void *) (sSegmentTable[segment] | 0x80000000);
}

void *segmented_to_virtual(const void *addr) {
    return (void *) addr;
}

void *virtual_to_segmented(UNUSED u32 segment, const void *addr) {
    return (void *) addr;
}

void move_segment_table_to_dmem(void) {
}

static void main_pool_free_all(void) {
    while (sPoolListHeadL != NULL) {
        main_pool_free(sPoolListHeadL + 1);
    }
}

void main_pool_init(void) {
    atexit(main_pool_free_all);
}

/**
 * Allocate a block of memory from the pool of given size, and from the
 * specified side of the pool (MEMORY_POOL_LEFT or MEMORY_POOL_RIGHT).
 * If there is not enough space, return NULL.
 */
void *main_pool_alloc(u32 size, void (*releaseHandler)(void *addr)) {
    struct MainPoolBlock *newListHead = (struct MainPoolBlock *) malloc(sizeof(struct MainPoolBlock) + size);
    if (newListHead == NULL) {
        abort();
    }
    if (sPoolListHeadL != NULL) {
        sPoolListHeadL->next = newListHead;
    }
    newListHead->prev = sPoolListHeadL;
    newListHead->next = NULL;
    newListHead->releaseHandler = releaseHandler;
    sPoolListHeadL = newListHead;
    return newListHead + 1;
}

u32 main_pool_free(void *addr) {
    struct MainPoolBlock *block = ((struct MainPoolBlock *) addr) - 1;
    void *toFree;
    do {
        if (sPoolListHeadL == NULL) {
            abort();
        }
        if (sPoolListHeadL->releaseHandler != NULL) {
            sPoolListHeadL->releaseHandler(sPoolListHeadL + 1);
        }
        toFree = sPoolListHeadL;
        sPoolListHeadL = sPoolListHeadL->prev;
        if (sPoolListHeadL != NULL) {
            sPoolListHeadL->next = NULL;
        }
        free(toFree);
    } while (toFree != block);
    return 0;
}

u32 main_pool_push_state(void) {
    struct MainPoolState *prevState = gMainPoolState;
    gMainPoolState = main_pool_alloc(sizeof(*gMainPoolState), NULL);
    gMainPoolState->prev = prevState;
    return 0;
}

/**
 * Restore pool state from a previous call to main_pool_push_state. Return the
 * amount of free space left in the pool.
 */
u32 main_pool_pop_state(void) {
    struct MainPoolState *prevState = gMainPoolState->prev;
    main_pool_free(gMainPoolState);
    gMainPoolState = prevState;
}

/**
 * Perform a DMA read from ROM. The transfer is split into 4KB blocks, and this
 * function blocks until completion.
 */
static void dma_read(u8 *dest, u8 *srcStart, u8 *srcEnd) {
    u32 size = ALIGN16(srcEnd - srcStart);

    memcpy(dest, srcStart, srcEnd - srcStart);
}

/**
 * Perform a DMA read from ROM, allocating space in the memory pool to write to.
 * Return the destination address.
 */
static void *dynamic_dma_read(u8 *srcStart, u8 *srcEnd, u32 side) {
    void *dest;
    u32 size = ALIGN16(srcEnd - srcStart);

    dest = main_pool_alloc(size, NULL);
    if (dest != NULL) {
        dma_read(dest, srcStart, srcEnd);
    }
    return dest;
}

/**
 * Allocate an allocation-only pool from the main pool. This pool doesn't
 * support freeing allocated memory.
 * Return NULL if there is not enough space in the main pool.
 */
static void alloc_only_pool_release_handler(void *addr) {
    struct AllocOnlyPool *pool = (struct AllocOnlyPool *) addr;
    struct AllocOnlyPoolBlock *block = pool->lastBlock;
    while (block != NULL) {
        struct AllocOnlyPoolBlock *prev = block->prev;
        free(block);
        block = prev;
    }
}

struct AllocOnlyPool *alloc_only_pool_init(void) {
    struct AllocOnlyPool *pool;
    void *addr = main_pool_alloc(sizeof(struct AllocOnlyPool), alloc_only_pool_release_handler);

    pool = (struct AllocOnlyPool *) addr;
    pool->lastBlock = NULL;
    pool->lastBlockSize = 0;
    pool->lastBlockNextPos = 0;

    return pool;
}

void alloc_only_pool_clear(struct AllocOnlyPool *pool) {
    alloc_only_pool_release_handler(pool);
    pool->lastBlock = NULL;
    pool->lastBlockSize = 0;
    pool->lastBlockNextPos = 0;
}

void *alloc_only_pool_alloc(struct AllocOnlyPool *pool, s32 size) {
    u8 *addr;
    u32 s = size;
    if (pool->lastBlockSize - pool->lastBlockNextPos < s) {
        struct AllocOnlyPoolBlock *block;
        u32 nextSize = pool->lastBlockSize * 2;
        if (nextSize < 100) {
            nextSize = 100;
        }
        if (nextSize < s) {
            nextSize = s;
        }
        block = (struct AllocOnlyPoolBlock *) malloc(sizeof(struct AllocOnlyPoolBlock) + nextSize);
        if (block == NULL) {
            abort();
        }
        block->prev = pool->lastBlock;
        pool->lastBlock = block;
        pool->lastBlockSize = nextSize;
        pool->lastBlockNextPos = 0;
    }
    addr = (u8 *) (pool->lastBlock + 1) + pool->lastBlockNextPos;
    pool->lastBlockNextPos += s;
    return addr;
}

struct MemoryPool *mem_pool_init(UNUSED u32 size, UNUSED u32 side) {
    struct MemoryPool *pool;
    void *addr = main_pool_alloc(sizeof(struct MemoryPool), NULL);
    u32 i;

    pool = (struct MemoryPool *) addr;
    pool->allocOnlyPool = alloc_only_pool_init();
    for (i = 0; i < ARRAY_COUNT(pool->bins); i++) {
        pool->bins[i] = NULL;
    }

    return pool;
}

void *mem_pool_alloc(struct MemoryPool *pool, u32 size) {
    struct FreeListNode *node;
    struct AllocatedNode *an;
    s32 bin = -1;
    u32 itemSize;
    u32 i;

    for (i = 3; i < 30; i++) {
        if (size <= (1U << i)) {
            bin = i;
            break;
        }
    }
    if (bin == -1) {
        abort();
    }
    itemSize = 1 << bin;
    node = pool->bins[bin - 3];
    if (node == NULL) {
        node = alloc_only_pool_alloc(pool->allocOnlyPool, sizeof(struct AllocatedNode) + itemSize);
        node->next = NULL;
        pool->bins[bin - 3] = node;
    }
    an = (struct AllocatedNode *) node;
    pool->bins[bin - 3] = node->next;
    an->bin = bin;
    return an + 1;
}

void mem_pool_free(struct MemoryPool *pool, void *addr) {
    struct AllocatedNode *an = ((struct AllocatedNode *) addr) - 1;
    struct FreeListNode *node = (struct FreeListNode *) an;
    s32 bin = an->bin;
    node->next = pool->bins[bin - 3];
    pool->bins[bin - 3] = node;
}

void *alloc_display_list(u32 size) {
    size = ALIGN8(size);
    return alloc_only_pool_alloc(gGfxAllocOnlyPool, size);
}

static struct MarioAnimDmaRelatedThing *func_802789F0(u8 *srcAddr) {
    struct MarioAnimDmaRelatedThing *sp1C = dynamic_dma_read(srcAddr, srcAddr + sizeof(u32),
                                                             MEMORY_POOL_LEFT);
    u32 size = sizeof(u32) + (sizeof(u8 *) - sizeof(u32)) + sizeof(u8 *) +
               sp1C->count * sizeof(struct OffsetSizePair);
    main_pool_free(sp1C);

    sp1C = dynamic_dma_read(srcAddr, srcAddr + size, MEMORY_POOL_LEFT);
    sp1C->srcAddr = srcAddr;
    return sp1C;
}

void func_80278A78(struct MarioAnimation *a, void *b, struct Animation *target) {
    if (b != NULL) {
        a->animDmaTable = func_802789F0(b);
    }
    a->currentAnimAddr = NULL;
    a->targetAnim = target;
}

s32 load_patchable_table(struct MarioAnimation *a, u32 index) {
    s32 ret = FALSE;
    struct MarioAnimDmaRelatedThing *sp20 = a->animDmaTable;
    u8 *addr;
    u32 size;

    if (index < sp20->count) {
        do {
            addr = sp20->srcAddr + sp20->anim[index].offset;
            size = sp20->anim[index].size;
        } while (0);
        if (a->currentAnimAddr != addr) {
            dma_read((u8 *) a->targetAnim, addr, addr + size);
            a->currentAnimAddr = addr;
            ret = TRUE;
        }
    }
    return ret;
}
