/*
 * #924 profiling branch: 1 = build the sampling heap profiler (worldserver/HeapProfiler.cpp), the live object counters
 * (Creature/GameObject/Player) and the startup probe (World.cpp); 0 = a normal build for performance measurements.
 * Only the files that include this header rebuild when it changes.
 */
#ifndef LOA_HEAPPROF_CONFIG_H
#define LOA_HEAPPROF_CONFIG_H

#define LOA_HEAPPROF 0

#endif
