#include "core.h"
#include "utils.h"

#include "../includes/pcsx2/pcsx2f_api.h"
#include "../includes/pcsx2/nanoprintf.h"
#include "../includes/pcsx2/log.h"
#include "../includes/pcsx2/memalloc.h"

int CompatibleCRCList[] = {
    (int)0x4F32A11F, (int)0xB3AD1EA4, // vcs
    (int)0x7EA439F5, (int)0xD693D4CF, // lcs
};

char OSDText[OSDStringNum][OSDStringSize] = { {1} };

#define STR(x) STR2(x)
#define STR2(x) #x

extern "C" const char VERSION[] = "[|VERSION]" VERSION_DATE ";" STR(VERSION_CODE) ";" VERSION_CODE_STR "[/VERSION|]";

#ifdef __cplusplus
void* operator new(size_t size)
{
    void* p = AllocMemBlock(size);
    return p;
}

void operator delete(void* p)
{
    FreeMemBlock(p);
}
#endif

extern "C" void init()
{
    logger.SetBuffer(OSDText, sizeof(OSDText) / sizeof(OSDText[0]), sizeof(OSDText[0]));

    core::initialize();
}

int main()
{
    return 0;
}
