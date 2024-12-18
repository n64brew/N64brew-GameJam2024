/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "memory.h"

namespace {
  heap_stats_t heap_stats{};
}

void Memory::dumpHeap(const char *name) {
  auto oldUsed = heap_stats.used;
  sys_get_heap_stats(&heap_stats);
  if(name) {
    debugf("[%s]: Heap: %d | diff: %d\n", name, heap_stats.used, heap_stats.used - oldUsed);
  } else {
    debugf("Heap: %d\n", heap_stats.used);
  }
}

