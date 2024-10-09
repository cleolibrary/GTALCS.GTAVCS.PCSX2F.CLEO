#include "libres.h"
#include "utils.h"
#include "pattern.h"

using namespace core;

namespace libres
{
    uint32_t getGpValue()
    {
        unsigned int gp;
        asm(
            "move %0, $gp\n"
            : "=r"(gp)
        );
        return gp;
    }

    bool init(e_game& game, int32_t& image_base)
    {
        uint32_t addr;
        if (__FindPatternAddress(addr, "52 00 02 3C ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 00 00 ?? ?? ?? ?? 00 00 00 00 ?? ?? ?? ?? 48 00 10 3C"))
        {
            game = GTAVCS;
            image_base = 0;
            return true;
        }
        else if (__FindPatternAddress(addr, "2D 20 40 00 63 00 04 3C"))
        {
            game = GTALCS;
            image_base = 0;
            return true;
        }

        return false;
    }
}
