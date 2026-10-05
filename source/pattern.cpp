#include "pattern.h"
#include "libres.h"
extern "C" {
#include "../external/injector/include/ps2/patterns.h"
}
bool __FindPatternAddressCompact(void*& result, const char* signature, int index)
{
    if (index < 0) return false;
    uintptr_t address = range_pattern.get((size_t)index, 0x100000, 0xf00000, signature, 0);
    if (!address) return false;
    result = (void*)address;
    return true;
}
