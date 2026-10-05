#include "armhook.h"
#include "libres.h"
#include "memutils.h"
#include "utils.h"

#include "guest-hooks.h"

namespace armhook
{
	void init() { guest_hooks::initialize(); }

	void replace_mips_call(ptr addr, ptr func_to)
	{ guest_hooks::replace_call(addr, func_to); }

	void hook_mips_func(ptr func, uint32_t startSize, ptr func_to, ptr* func_orig)
	{ guest_hooks::hook_function(func, startSize, func_to, func_orig); }

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
