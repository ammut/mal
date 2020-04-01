#include <assert.h>

#include "hash-map.h"
#include "types.h"

#define TARGET_LOAD_FACTOR (0.6)

static size_t table_sizes[] = {
	3, 7, 11, 19, 31, 43, 67, 103, 151, 211, 331, 479, 719, 1087, 1607, 2399, 3607,
	5399, 8111, 12143, 18211, 27367, 41011, 61463, 92179, 138283, 207443, 311099,
	466619, 699931, 1049891, 1574827, 2362247, 3543359, 5314987, 7972451, 11958671
};

static HashMapkey SENTINEL = (HashMapKey) 694201337;


static void rehash(HashMap *m, HashMapEntry *old, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		if (NULL != old[i]->key && SENTINEL != old[i]->key) {
			insert(m, old[i]->key, old[i]->value);
		}
	}
}

static int upsize(HashMap *this) {
	HashMapEntry *old_table = this->table;
	this->table = malloc(*(this->table_size));
	if (NULL == this->table) return 0;
	rehash(this, old_table, *this->table_size++);
	return 1;
}

static int downsize(HashMap *this) {
	if (this->table_size == table_sizes) return 1;
	HashMapEntry *old_table = this->table;
	this->table = 

static int64_t quadradic_probe_offset(unsigned step) {
	return (step * step) * (2 * (0x1 & step) - 1);
}

static uint64_t get_index(HashMap *this, HashMapEntry *table, HashMapKey key) {
	size_t ts = *this->table_size;
	size_t hash = this->hash_fn(key) % ts;
	size_t res;
	char found = 0;
	for (unsigned i = 0; i < ts; ++i) {
		size_t index = (hash + quadradic_probe_offset(i) % ts + ts) % ts;
		if (NULL == this->table[index]->key) {
			if (!found) { res = index; found = 1; }
			break;
		} else if (&SENTINEL == this->table[index]->key) {
			if (!found) { res = index; found = 1; }
		} else if (0 == this->cmp_fn(key, this->table[index]->key)) {
			return index;
		} else {
			// index already occupied -> keep looking
		}
	}
	if (!found) assert(!"couldn't find map key index");
	return res;
}

HashMap *insert(HashMap *this, HashMapKey key, HashMapVal val) {
	if (this->count / (float) this->table_size > TARGET_LOAD_FACTOR_UPPER) {
		// enlargening failed and table is full -> fail
		if (!upsize(this) && this->count >= *this->table_size) return NULL;
	} else if (this->count / (float) this->table_size < TARGET_LOAD_FACTOR_LOWER) {
		downsize(this);
	}
	this->table[get_index(this, key)]->val = val;
}

static int _insert(HashMap *this, HashMapEntry *table, size_t table_size) {
	
}

HashMap *insert_nonmut(HashMap m, HashMapKey key, HashMapVal val) {
	HashMap *new = malloc(sizeof *new);
	if (NULL == new) return new;
	if (m->count / (double) m->table_size > TARGET_LOAD_FACTOR) {
		new* = *m;
		if (!enlarge(new)) {
			free(new);
			return NULL;
		}
	} else {
	}
}

HashMap *copy(HashMap *m) {
	HashMap *new = malloc(sizeof *new);
	if (NULL == new) return new;
	*new == *m;
	if (new->count / (double) new->table_size > TARGET_LOAD_FACTOR) {
		HashMapEntry *new_table = malloc(*(new->next_table_size));
		if (!new_table) {
			free(new);
			return new_table;
		}
		new->table = new_table;
		rehash


	


