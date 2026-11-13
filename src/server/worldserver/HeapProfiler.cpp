/*
 * Sampling heap profiler for the memory-parity work (#924). Profiling branch only - never merge into mop.
 *
 * Replaces the global operator new/delete of the worldserver executable. Every block gets a 16-byte header; about one
 * allocated byte in SAMPLE_RATE is sampled (size-weighted Poisson sampling, as in tcmalloc), and a sampled block records
 * the call stack that allocated it. Live sampled bytes are kept per call-stack site, scaled back to estimated real bytes.
 * Every LOA_HEAPPROF_INTERVAL seconds (default 300) a background thread writes heapprof-<pid>-<seq>.txt next to the
 * executable: the module base, then one line per site sorted by estimated live bytes:
 *   <est_live_bytes> <live_samples> <est_total_bytes_ever> <frames...>   (frames = raw return addresses, hex)
 * Symbolize offline with the matching PDB (not shipped).
 * Only memory that goes through operator new is seen (malloc/HeapAlloc/VirtualAlloc users such as Detour are not).
 */
#include "HeapProfConfig.h"
#if defined(_WIN32) && LOA_HEAPPROF
#include <windows.h>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

// Live object counters, defined in Creature.cpp / GameObject.cpp / Player.cpp on the profiling branch.
extern std::atomic<int64_t> g_heapprofLiveCreatures;
extern std::atomic<int64_t> g_heapprofLiveGameObjects;
extern std::atomic<int64_t> g_heapprofLivePlayers;

// Per-thread allocation measurement around a construction (used by the startup probe in World.cpp).
void HeapProf_MeasureBegin();
void HeapProf_MeasureEnd(int64_t& bytes, int64_t& count);

namespace
{
    constexpr uint32_t MAGIC_PLAIN = 0xA110CA7Eu;
    constexpr uint32_t MAGIC_SAMPLED = 0x5A3D1E55u;
    constexpr double SAMPLE_RATE = 512.0 * 1024.0;   // mean bytes between samples
    constexpr uint32_t MAX_FRAMES = 28;
    constexpr uint32_t SITE_CAPACITY = 1u << 17;     // open addressing, power of two

    struct alignas(16) Header
    {
        uint64_t size;
        uint32_t magic;
        uint32_t site;   // site index + 1 for sampled blocks, 0 otherwise
    };
    static_assert(sizeof(Header) == 16, "header must keep 16-byte alignment");

    struct Site
    {
        uint32_t hash;
        uint32_t depth;
        void* frames[MAX_FRAMES];
        std::atomic<int64_t> liveBytes;
        std::atomic<int64_t> liveSamples;
        std::atomic<int64_t> totalBytes;
    };

    Site* g_sites = nullptr;          // HeapAlloc'd once, never freed
    SRWLOCK g_siteLock = SRWLOCK_INIT;
    std::atomic<uint32_t> g_siteCount{ 0 };
    std::atomic<bool> g_ready{ false };

    thread_local int64_t tl_untilSample = -1;
    thread_local uint64_t tl_rng = 0;
    thread_local bool tl_measuring = false;
    thread_local int64_t tl_measureBytes = 0;
    thread_local int64_t tl_measureCount = 0;

    inline uint64_t NextRandom()
    {
        if (!tl_rng)
            tl_rng = (uint64_t(GetCurrentThreadId()) << 32) ^ GetTickCount64() ^ 0x9E3779B97F4A7C15ull;
        tl_rng ^= tl_rng << 13; tl_rng ^= tl_rng >> 7; tl_rng ^= tl_rng << 17;
        return tl_rng;
    }

    inline int64_t NextGap()
    {
        double u = double((NextRandom() >> 11) + 1) / double(1ull << 53);
        return int64_t(-std::log(u) * SAMPLE_RATE) + 1;
    }

    // Estimated real bytes one sample of `size` bytes stands for (unbiased for Poisson sampling).
    inline int64_t Weight(uint64_t size)
    {
        double p = 1.0 - std::exp(-double(size) / SAMPLE_RATE);
        return p > 0.0 ? int64_t(double(size) / p) : int64_t(SAMPLE_RATE);
    }

    void EnsureInit()
    {
        if (g_ready.load(std::memory_order_acquire))
            return;
        AcquireSRWLockExclusive(&g_siteLock);
        if (!g_sites)
            g_sites = static_cast<Site*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Site) * SITE_CAPACITY));
        ReleaseSRWLockExclusive(&g_siteLock);
        g_ready.store(g_sites != nullptr, std::memory_order_release);
    }

    uint32_t FindOrAddSite(void** frames, uint32_t depth, uint32_t hash)
    {
        if (!hash)
            hash = 1;
        uint32_t idx = hash & (SITE_CAPACITY - 1);
        AcquireSRWLockExclusive(&g_siteLock);
        for (uint32_t probe = 0; probe < SITE_CAPACITY; ++probe, idx = (idx + 1) & (SITE_CAPACITY - 1))
        {
            Site& s = g_sites[idx];
            if (s.hash == 0)
            {
                s.hash = hash;
                s.depth = depth;
                memcpy(s.frames, frames, depth * sizeof(void*));
                g_siteCount.fetch_add(1, std::memory_order_relaxed);
                ReleaseSRWLockExclusive(&g_siteLock);
                return idx;
            }
            if (s.hash == hash && s.depth == depth && !memcmp(s.frames, frames, depth * sizeof(void*)))
            {
                ReleaseSRWLockExclusive(&g_siteLock);
                return idx;
            }
        }
        ReleaseSRWLockExclusive(&g_siteLock);
        return UINT32_MAX;   // table full: drop the sample
    }

    void* Allocate(size_t size)
    {
        Header* h = static_cast<Header*>(malloc(size + sizeof(Header)));
        if (!h)
            return nullptr;
        h->size = size;
        h->magic = MAGIC_PLAIN;
        h->site = 0;
        if (tl_measuring)
        {
            tl_measureBytes += int64_t(size + sizeof(Header));
            ++tl_measureCount;
        }

        if (tl_untilSample < 0)
            tl_untilSample = NextGap();
        tl_untilSample -= int64_t(size);
        if (tl_untilSample <= 0)
        {
            tl_untilSample = NextGap();
            EnsureInit();
            if (g_ready.load(std::memory_order_acquire))
            {
                void* frames[MAX_FRAMES];
                ULONG hash = 0;
                USHORT depth = RtlCaptureStackBackTrace(2, MAX_FRAMES, frames, &hash);
                uint32_t idx = FindOrAddSite(frames, depth, hash);
                if (idx != UINT32_MAX)
                {
                    int64_t w = Weight(size);
                    g_sites[idx].liveBytes.fetch_add(w, std::memory_order_relaxed);
                    g_sites[idx].liveSamples.fetch_add(1, std::memory_order_relaxed);
                    g_sites[idx].totalBytes.fetch_add(w, std::memory_order_relaxed);
                    h->magic = MAGIC_SAMPLED;
                    h->site = idx + 1;
                }
            }
        }
        return h + 1;
    }

    void Release(void* p)
    {
        if (!p)
            return;
        Header* h = static_cast<Header*>(p) - 1;
        if (h->magic != MAGIC_PLAIN && h->magic != MAGIC_SAMPLED)
        {
            free(p);   // block from a runtime DLL's own operator new (no header): hand it back untouched
            return;
        }
        if (h->magic == MAGIC_SAMPLED && h->site)
        {
            Site& s = g_sites[h->site - 1];
            s.liveBytes.fetch_sub(Weight(h->size), std::memory_order_relaxed);
            s.liveSamples.fetch_sub(1, std::memory_order_relaxed);
        }
        h->magic = 0;
        free(h);
    }

    // Aligned variants: [base .. pad][Header][user block aligned to A]; the header stores the base offset in `site`'s
    // place is not possible (site is needed), so the base pointer sits in the 8 bytes before the header.
    void* AllocateAligned(size_t size, size_t align)
    {
        size_t a = align < 16 ? 16 : align;
        size_t total = size + a + sizeof(Header) + sizeof(void*);
        char* base = static_cast<char*>(malloc(total));
        if (!base)
            return nullptr;
        uintptr_t user = (uintptr_t(base) + sizeof(void*) + sizeof(Header) + a - 1) & ~(uintptr_t(a) - 1);
        Header* h = reinterpret_cast<Header*>(user) - 1;
        reinterpret_cast<void**>(h)[-1] = base;
        h->size = size;
        h->magic = MAGIC_PLAIN;   // aligned blocks are never sampled (rare, and keeps Release simple)
        h->site = 0;
        return reinterpret_cast<void*>(user);
    }

    void ReleaseAligned(void* p)
    {
        if (!p)
            return;
        Header* h = static_cast<Header*>(p) - 1;
        void* base = reinterpret_cast<void**>(h)[-1];
        h->magic = 0;
        free(base);
    }

    DWORD WINAPI DumpThread(LPVOID)
    {
        char const* env = getenv("LOA_HEAPPROF_INTERVAL");
        DWORD interval = env ? DWORD(atoi(env)) : 300;
        if (interval < 10)
            interval = 10;
        HMODULE exe = GetModuleHandleW(nullptr);
        uint32_t seq = 0;
        struct Row { int64_t live; uint32_t idx; };
        Row* rows = static_cast<Row*>(HeapAlloc(GetProcessHeap(), 0, sizeof(Row) * SITE_CAPACITY));
        for (;;)
        {
            Sleep(interval * 1000);
            if (!g_ready.load(std::memory_order_acquire) || !rows)
                continue;
            uint32_t n = 0;
            int64_t sum = 0;
            for (uint32_t i = 0; i < SITE_CAPACITY; ++i)
                if (g_sites[i].hash && g_sites[i].liveBytes.load(std::memory_order_relaxed) > 0)
                {
                    rows[n].live = g_sites[i].liveBytes.load(std::memory_order_relaxed);
                    rows[n].idx = i;
                    sum += rows[n].live;
                    ++n;
                }
            // partial selection: the top 4000 sites by live bytes are plenty for attribution
            uint32_t top = n < 4000 ? n : 4000;
            for (uint32_t i = 0; i < top; ++i)
            {
                uint32_t best = i;
                for (uint32_t j = i + 1; j < n; ++j)
                    if (rows[j].live > rows[best].live)
                        best = j;
                Row t = rows[i]; rows[i] = rows[best]; rows[best] = t;
            }
            char path[MAX_PATH];
            snprintf(path, sizeof(path), "heapprof-%lu-%03u.txt", GetCurrentProcessId(), ++seq);
            HANDLE f = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (f == INVALID_HANDLE_VALUE)
                continue;
            char line[1024];
            DWORD written;
            int len = snprintf(line, sizeof(line), "# heapprof pid=%lu seq=%u base=%p sites=%u est_live_total=%lld sample_rate=%.0f live_creatures=%lld live_gameobjects=%lld live_players=%lld\n",
                GetCurrentProcessId(), seq, (void*)exe, n, (long long)sum, SAMPLE_RATE,
                (long long)g_heapprofLiveCreatures.load(), (long long)g_heapprofLiveGameObjects.load(), (long long)g_heapprofLivePlayers.load());
            WriteFile(f, line, DWORD(len), &written, nullptr);
            for (uint32_t i = 0; i < top; ++i)
            {
                Site& s = g_sites[rows[i].idx];
                len = snprintf(line, sizeof(line), "%lld %lld %lld", (long long)rows[i].live,
                    (long long)s.liveSamples.load(std::memory_order_relaxed), (long long)s.totalBytes.load(std::memory_order_relaxed));
                for (uint32_t k = 0; k < s.depth && len < int(sizeof(line)) - 24; ++k)
                    len += snprintf(line + len, sizeof(line) - len, " %p", s.frames[k]);
                line[len++] = '\n';
                WriteFile(f, line, DWORD(len), &written, nullptr);
            }
            CloseHandle(f);
        }
    }

    struct Starter
    {
        Starter()
        {
            EnsureInit();
            CreateThread(nullptr, 0, DumpThread, nullptr, 0, nullptr);
        }
    } g_starter;
}

void HeapProf_MeasureBegin() { tl_measureBytes = 0; tl_measureCount = 0; tl_measuring = true; }
void HeapProf_MeasureEnd(int64_t& bytes, int64_t& count) { tl_measuring = false; bytes = tl_measureBytes; count = tl_measureCount; }

void* operator new(size_t size) { if (void* p = Allocate(size ? size : 1)) return p; throw std::bad_alloc(); }
void* operator new[](size_t size) { if (void* p = Allocate(size ? size : 1)) return p; throw std::bad_alloc(); }
void* operator new(size_t size, std::nothrow_t const&) noexcept { return Allocate(size ? size : 1); }
void* operator new[](size_t size, std::nothrow_t const&) noexcept { return Allocate(size ? size : 1); }
void operator delete(void* p) noexcept { Release(p); }
void operator delete[](void* p) noexcept { Release(p); }
void operator delete(void* p, size_t) noexcept { Release(p); }
void operator delete[](void* p, size_t) noexcept { Release(p); }
void operator delete(void* p, std::nothrow_t const&) noexcept { Release(p); }
void operator delete[](void* p, std::nothrow_t const&) noexcept { Release(p); }

void* operator new(size_t size, std::align_val_t al) { if (void* p = AllocateAligned(size ? size : 1, size_t(al))) return p; throw std::bad_alloc(); }
void* operator new[](size_t size, std::align_val_t al) { if (void* p = AllocateAligned(size ? size : 1, size_t(al))) return p; throw std::bad_alloc(); }
void* operator new(size_t size, std::align_val_t al, std::nothrow_t const&) noexcept { return AllocateAligned(size ? size : 1, size_t(al)); }
void* operator new[](size_t size, std::align_val_t al, std::nothrow_t const&) noexcept { return AllocateAligned(size ? size : 1, size_t(al)); }
void operator delete(void* p, std::align_val_t) noexcept { ReleaseAligned(p); }
void operator delete[](void* p, std::align_val_t) noexcept { ReleaseAligned(p); }
void operator delete(void* p, size_t, std::align_val_t) noexcept { ReleaseAligned(p); }
void operator delete[](void* p, size_t, std::align_val_t) noexcept { ReleaseAligned(p); }
#endif
