#include <stdlib.h>
#include <inttypes.h>

typedef void *HashMapKey;
typedef void *HashMapVal;

typedef struct sHashMapEntry {
	HashMapKey key;
	HashMapVal val;
} HashMapEntry;

typedef struct sHashMap {
	size_t *table_size;
	size_t count;
	uint64_t (*hash_fn)(HashMapKey);
	int (*cmp_fn)(HashMapKey, HashMapKey);
	HashMapEntry *table;
} HashMap;

HashMap *insert_mutating(HashMap m, HashMapKey key, HashMapVal val);

HashMap *insert(HashMap m, HashMapKey key, HashMapVal val);

HashMap *get(HashMap m, HashMapKey key);

HashMap *has(HashMap m, HashMapKey key);

HashMap *remove_mutating(HashMap m, HashMapKey key);

HashMap *remove(HashMap m, HashMapKey key);

