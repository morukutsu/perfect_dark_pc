#include "offsetsmap.h"
#include "gfx/hashmap.h"
#include "print.h"

struct hashmap *offsets = NULL;

uint64_t offset_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct offset_pc *data = item;
    return hashmap_sip(item, sizeof(u32), seed0, seed1);
}

int offset_compare(const void *a, const void *b, void *udata) {
    const struct offset_pc *ua = a;
    const struct offset_pc *ub = b;
    return ua->old - ub->old;
}

void initOffsetsContext()
{
    if (offsets) clearOffsetsContext();
    offsets = hashmap_new(sizeof(struct offset_pc), 0, 0, 0, offset_hash, offset_compare, NULL, NULL);
}

void clearOffsetsContext()
{
    if (offsets) {
        hashmap_free(offsets);
        offsets = NULL;
    }
}

void addOffsetGlobal(u32 old, u32 new, u32 size)
{
    struct offset_pc moff = { old, new, size };
	hashmap_set(offsets, &moff);
}

void* replaceOffsetGlobal(u32 old)
{
    struct offset_pc moff = { old };
	struct offset_pc* item = hashmap_get(offsets, &moff);
	if (item) {
		return (void*)(uintptr_t)item->new;
	} 

    print("offsetsmap:replaceOffset() %x\n", old);
    print("offset not found\n");
    fatalExit();

    // unreached
	return (void*)(uintptr_t)old;
}

