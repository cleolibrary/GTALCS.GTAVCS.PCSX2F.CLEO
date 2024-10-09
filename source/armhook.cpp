#include "armhook.h"
#include "libres.h"
#include "memutils.h"
#include "utils.h"

#include "../includes/pcsx2/memalloc.h"

namespace armhook
{
	void init()
	{
	}

	void replace_mips_call(ptr addr, ptr func_to)
	{
		memutils::mem_write_mips_call(addr, func_to, false);
	}

	void hook_mips_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig)
	{
		uint8_t *space = cast<uint8_t *>(AllocMemBlock(startSize + 32));
		while ((cast<uint32_t>(space) & 0x0F) != (cast<uint32_t>(func) & 0x0F))
			space++;
		memutils::mem_write_arr(space, func, startSize);
		memutils::mem_write_mips_jmp(space + startSize, func + startSize, true);
		*func_orig = space;
		memutils::mem_write_mips_jmp(func, func_to, true);
	}

	std::vector<ptr> find_mips_func_calls(ptr func)
	{
		uint32_t code = 0x0C000000 | ((cast<uint32_t>(func) >> 2) & 0x03FFFFFF);
		std::vector<ptr> res;

		void *imageBase = (void*)0x100000;

		uint8_t* addrStart = (uint8_t*)imageBase;
		uint8_t* addrMax = (uint8_t*)0xFFFFFF;
		for (uint8_t* addr = addrStart; addr < addrMax; addr += 4)
			if (*cast<uint32_t*>(addr) == code)
				res.push_back(addr);

		return res;
	}

	std::vector<ptr> find_mips_func_calls_in_func(ptr func, ptr func_in)
	{
		uint32_t code = 0x0C000000 | ((cast<uint32_t>(func) >> 2) & 0x03FFFFFF);
		std::vector<ptr> res;

		// let's assume first jr $ra is func end
		for (uint8_t *addr = func_in; *cast<uint32_t *>(addr) != 0x03E00008; addr += 4)
			if (*cast<uint32_t *>(addr) == code)
				res.push_back(addr);

		return res;
	}
}
