#include "memutils.h"

namespace memutils
{
	void mem_write_arr(uint8_t *addr, uint8_t *arr, uint32_t size, bool protect)
	{
		for (int i = 0; i < size; i++)
			addr[i] = arr[i];
	}

	ptr mem_read_mips_jmp(uint8_t *addr)
	{
		return cast<ptr>((*cast<uint32_t *>(addr) & 0x03FFFFFF) << 2);
	}

	void mem_write_mips_jmp(uint8_t *addrFrom, uint8_t *addrTo, bool withNop)
	{
		uint64_t code = 0x08000000 | ((cast<uint32_t>(addrTo) >> 2) & 0x03FFFFFF);
		mem_write_arr(addrFrom, cast<uint8_t *>(&code), withNop ? 8 : 4);
	}

	void mem_write_mips_call(uint8_t *addrFrom, uint8_t *addrTo, bool withNop)
	{
		uint64_t code = 0x0C000000 | ((cast<uint32_t>(addrTo) >> 2) & 0x03FFFFFF);
		mem_write_arr(addrFrom, cast<uint8_t *>(&code), withNop ? 8 : 4);
	}
}