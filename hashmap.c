#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

// Fixed seed to deterministic test results.
const uint64_t SEED = 0x123456;
// Power of 2 for fast calculation of indices from hash.
const size_t INITIAL_SIZE = 256;
// A bit lower than the optimal 0.7 to be conservative.
const double LOAD_FACTOR = 0.6;

typedef struct hashmap_entry {
  void* key;
  size_t key_size;
  void* data;
  size_t data_size;
} hashmap_entry;

typedef struct hashmap {
  hashmap_entry* arr;
  size_t size;
  size_t capacity;
} hashmap;

// FNV hash
// `key` is the key to be hashed. `key_size` is the size of `key` in bytes. `h`
// is the seed.
// Credit: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
static uint32_t FNV(const void* key, size_t key_size, uint32_t h) {
  h ^= 2166136261UL;
  const uint8_t* data = (const uint8_t*)key;
  for (size_t i = 0; i < key_size; ++i) {
    h ^= data[i];
    h *= 16777619;
  }
  return h;
}

// Returns true if `lhs` and `rhs` have the same key. Otherwise returns false.
static bool compare_key(const hashmap_entry* lhs, const hashmap_entry* rhs) {
  return strncmp(lhs->key, rhs->key, rhs->key_size) == 0 &&
         lhs->key_size == rhs->key_size;
}

// Inserts `entry` into `map`, performing linear probing in case of hash
// collision. Returns true if insertion is successful. Otherwise (i.e.,
// `entry`'s key is already present in `map`) returns false. It is assumed that
// `map` has enough capacity and its load factor is below the rehash threshold.
static bool maybe_probe_and_insert(hashmap* map, const hashmap_entry* entry) {
  uint32_t hash = FNV(entry->key, entry->key_size, SEED);
  // Equivalent to hash % (map->capacity) if map->capacity is a power of 2.
  // Credit: https://stackoverflow.com/questions/70089037
  size_t index = hash & (map->capacity - 1);
  hashmap_entry* arr_entry = &map->arr[index];
  if (arr_entry->data) {
    if (compare_key(arr_entry, entry)) {
      // Key already inserted.
      return false;
    }
    size_t i = 1;
    // Hash collision. Do linear probing.
    for (; i < map->capacity; ++i) {
      // Equivalent to index_r = (index + i) % map->capacity
      size_t index_r = (index + i) & (map->capacity - 1);
      if (!map->arr[index_r].data) {
        arr_entry = &map->arr[index_r];
        break;
      }
    }
    if (i >= map->capacity) {
      // Not supposed to happen.
      error("FATAL: linear probing failed to find a slot!");
    }
  }
  *arr_entry = *entry;
  ++map->size;
  return true;
}

// Doubles `map`'s capacity and rehashes its elements based on its new capacity.
static void rehash(hashmap* map) {
  size_t old_capacity = map->capacity;
  map->capacity *= 2;
  hashmap_entry* arr = calloc(map->capacity, sizeof(hashmap_entry));
  {
    hashmap_entry* tmp = map->arr;
    map->arr = arr;
    arr = tmp;
  }
  for (size_t i = 0; i < old_capacity; ++i) {
    if (!arr[i].key) {
      continue;
    }
    maybe_probe_and_insert(map, &arr[i]);
  }
}

void hashmap_init(hashmap* map) {
  map->arr = calloc(INITIAL_SIZE, sizeof(hashmap_entry));
  map->size = 0;
  map->capacity = INITIAL_SIZE;
}

bool hashmap_insert(hashmap* map, const hashmap_entry* entry) {
  if ((double)map->size / map->capacity >= LOAD_FACTOR) {
    rehash(map);
  }
  return maybe_probe_and_insert(map, entry);
}

hashmap_entry* hashmap_get(hashmap* map, void* key, size_t key_size) {
  uint32_t hash = FNV(key, key_size, SEED);
  size_t index = hash & (map->capacity - 1);
  return &map->arr[index];
}

void hashmap_destroy(hashmap* map) {
  for (size_t i = 0; i < map->capacity; ++i) {
    free(map->arr[i].key);
    free(map->arr[i].data);
  }
  free(map);
}
