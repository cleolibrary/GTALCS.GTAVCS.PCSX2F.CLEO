#pragma once
#include "common.h"
#include "utils.h"
#include "../external/injector/include/ps2/hooks_guest.h"
#include "../external/injector/include/ps2/patches.h"
#include "../external/injector/include/ps2/memalloc.h"
#include "../external/injector/include/ps2/game_abi.h"
#include <type_traits>

extern "C" {
#include "../external/injector/include/ps2/log.h"
}

// Prepared once per module; code and handles live in this plugin's reservation.
extern "C" const void* PCSX2FContext;
namespace guest_hooks {
inline pcsx2_hook_guest storage;
inline pcsx2_hook_backend backend;
inline bool initialized;
inline uint32_t function_hooks, call_patches;
inline void require(bool success, const char* operation)
{
    if (!success) { logger.WriteF("CLEO hook failure: %s", operation); __builtin_trap(); }
}
inline void initialize()
{
    if (initialized) return;
    require(pcsx2_hook_guest_backend(&backend, &storage, PCSX2FContext, AllocMemBlock, FreeMemBlock) == PCSX2_HOOK_OK, "backend");
    initialized = true;
}
template<class Return,class... Args>
inline ptr bridge(Return (*function)(Args...),pcsx2_game_abi_direction direction)
{
    constexpr unsigned count=sizeof...(Args);
    constexpr bool floating[]={std::is_same<Args,float>::value...,false};
    uint32_t mask=0;unsigned floats=0;
    for (unsigned i=0;i<count;++i) if (floating[i]) { mask|=1u<<i;++floats; }
    if (direction==PCSX2_GAME_ABI_CALL && (!floats || floats==count)) return cast<ptr>(function);
    // One adapter per target and direction, shared by every patched call site.
    struct Adapter { uint32_t target, address; pcsx2_game_abi_direction direction; };
    static Adapter adapters[64];
    static unsigned adapterCount;
    const uint32_t target=(uint32_t)(uintptr_t)function;
    for (unsigned i=0;i<adapterCount;++i)
        if (adapters[i].target==target && adapters[i].direction==direction) return cast<ptr>(adapters[i].address);
    require(adapterCount<64,"game ABI adapter count");
    initialize();
    uint32_t words[128];size_t used=0;
    require(pcsx2_game_abi_emit(words,128,target,count,mask,direction,&used)==PCSX2_HOOK_OK,"game ABI");
    pcsx2_hook_buffer code={};
    require(backend.allocate(backend.user,used*4,target,&code)!=0,"game ABI allocation");
    require(code.words && code.capacity>=used*4,"game ABI storage");
    memcpy(code.words,words,used*4);
    backend.flush(backend.user,code.address,used*4);
    adapters[adapterCount++]={target,code.address,direction};
    return cast<ptr>(code.address); // Permanent, bounded module-owned storage.
}
inline void replace_call(ptr address, ptr target)
{
    initialize();
    pcsx2_patch patch = {};
    require(pcsx2_patch_create_call(&patch, &backend, (uint32_t)(uintptr_t)address,
        (uint32_t)(uintptr_t)target) == PCSX2_HOOK_OK, "prepare call");
    require(pcsx2_patch_enable(&patch) == PCSX2_HOOK_OK, "publish call");
    ++call_patches;
    // Permanent legacy call patch: no code allocation or destructor needed.
}
inline void hook_function(ptr address, uint32_t bytes, ptr target, ptr* original, ptr (*adapt)(ptr)=nullptr)
{
    initialize();
    require(original && bytes >= 8 && !(bytes & 3) && bytes <= PCSX2_HOOK_MAX_INSTRUCTIONS * 4, "prologue size");
    auto* hook = static_cast<pcsx2_hook*>(AllocMemBlock(sizeof(pcsx2_hook)));
    require(hook != nullptr, "handle allocation");
    memset(hook, 0, sizeof(*hook));
    auto status = pcsx2_hook_create_inline(hook, &backend, (uint32_t)(uintptr_t)address,
        (uint32_t)(uintptr_t)target, bytes / 4);
    if (status != PCSX2_HOOK_OK) { FreeMemBlock(hook); require(false, "relocate prologue"); }
    // The callback may run as soon as the entry patch is published.
    ptr entry=(ptr)(uintptr_t)hook->trampoline;
    *original = adapt ? adapt(entry) : entry;
    status = pcsx2_hook_enable(hook);
    if (status != PCSX2_HOOK_OK) {
        *original = nullptr;
        (void)pcsx2_hook_destroy(hook); FreeMemBlock(hook);
        require(false, "publish hook");
    }
    ++function_hooks;
    // Permanent legacy function hook: retain its handle and original trampoline.
}
inline void write_code(ptr address, const uint32_t* words, size_t bytes)
{
    initialize();
    require(backend.write(backend.user, (uint32_t)(uintptr_t)address, words, bytes) != 0, "write code");
    backend.flush(backend.user, (uint32_t)(uintptr_t)address, bytes);
}
}
