#include "core.h"
#include "core_asm.h"
#include "core_menu.h"
#include "armhook.h"
#include "utils.h"
#include "strutils.h"
#include "text.h"
#include "touch.h"
#include "ui.h"
#include "libres.h"
#include "pattern.h"
#include "mutex.h"
#include "memutils.h"
#include "psplang.h"

#include "../includes/pcsx2/memalloc.h"

uint8_t CLEOScripts[1000000] = { 1 };

namespace core
{
	e_game game;
	e_game GetGame() { return game; }

	enum e_game_version
	{
		VER_NONE,
		VER_GTA3_1_4,
		VER_GTAVC_1_03,
		VER_GTASA_1_00,
		VER_GTASA_1_01,
		VER_GTASA_1_02,
		VER_GTASA_1_03,
		VER_GTASA_1_05,
		VER_GTASA_1_06,
		VER_GTASA_1_05_GER,
		VER_GTASA_1_07,
		VER_GTA3_1_6,
		VER_GTAVC_1_06,
		VER_GTASA_1_08,
		VER_GTALCS_2_2,
		VER_GTA3_1_8_OR_HIGHER,
		VER_GTAVC_1_09_OR_HIGHER,
		VER_GTASA_2_00_OR_HIGHER,
		VER_GTALCS_2_4_OR_HIGHER,
		VER_GTALCS_PSP_1_05_OR_HIGHER,
		VER_GTAVCS_PSP_1_02_OR_HIGHER
	};

	const char *str_game_version[] = {
		"VER_NONE",
		"VER_GTA3_1_4",
		"VER_GTAVC_1_03",
		"VER_GTASA_1_00",
		"VER_GTASA_1_01",
		"VER_GTASA_1_02",
		"VER_GTASA_1_03",
		"VER_GTASA_1_05",
		"VER_GTASA_1_06",
		"VER_GTASA_1_05_GER",
		"VER_GTASA_1_07",
		"VER_GTA3_1_6",
		"VER_GTAVC_1_06",
		"VER_GTASA_1_08",
		"VER_GTALCS_2_2",
		"VER_GTA3_1_8_OR_HIGHER",
		"VER_GTAVC_1_09_OR_HIGHER",
		"VER_GTASA_2_00_OR_HIGHER",
		"VER_GTALCS_2_4_OR_HIGHER",
		"VER_GTALCS_PSP_1_05_OR_HIGHER",
		"VER_GTAVCS_PSP_1_02_OR_HIGHER"
	};

	int game_ver = VER_NONE;
	int GetGameVersion() { return game_ver; }

	std::string package_name;
	const char *GetPackageName() { return package_name.c_str(); }

	std::string package_version_name;
	const char *GetPackageVersionStr() { return package_version_name.c_str(); }

	int	package_version_code;
	int GetPackageVersionCode() { return package_version_code; }

	void preload_scripts();
	void launch_scripts();
	void save_scripts(uint32_t a = 0, uint32_t b = 0); // params not used for SA
	void process_opcodes(ptr handle);

	struct t_script
	{
		std::string name;
		int32_t invokable_id;
		uint8_t *handle; // returned by CTheScripts::StartNewScript
		uint8_t *code; // script code
		uint32_t code_size;
		uint32_t offset; // start ip for script vm, based on ScriptSpace overrun
		bool wait_passed;
		uint32_t context[32];

		t_script() : handle(NULL), code(NULL), offset(0) {}
	};

	std::vector<t_script *> scripts;

	t_script *get_script_using_handle(uint8_t *handle)
	{
		for (int32_t i = 0; i < scripts.size(); i++)
			if (scripts[i]->handle == handle)
				return scripts[i];
		return NULL;
	}

	int32_t image_base; // image base

	struct t_mutex_var
	{
		uint32_t id, value;
	};

	std::vector<t_mutex_var> mutex_vars;

	template <typename T> inline T getfield(ptr structure, uint32_t offset)
	{
		return *cast<T *>(structure + offset);
	}

	template <typename T> inline void setfield(ptr structure, uint32_t offset, T val)
	{
		*cast<T *>(structure + offset) = val;
	}

	template <typename T> inline T select(T val_3, T val_vc, T val_sa, T val_lcs, T val_vcs)
	{
		switch (game)
		{
		case GTA3: return val_3;
		case GTAVC: return val_vc;
		case GTASA: return val_sa;
		case GTALCS: return val_lcs;
		case GTAVCS: return val_vcs;
		}
	}

	// @windowSize
	typedef struct { uint32_t w, h; } _windowSize;
	_windowSize *windowSize;

	// @windowSize LCS
	typedef struct { uint32_t h, w; } _windowSizeLCS;
	_windowSizeLCS *windowSizeLCS;

	// @CTimer::m_snTimeInMilliseconds
	uint32_t *CTimer__m_snTimeInMilliseconds;

	// @ScriptParams
	uint32_t *ScriptParams;
	uint32_t *GetScriptParams() { return ScriptParams; }

	// @CTheScripts::ScriptSpace, @CTheScripts::pActiveScripts
	ptr CTheScripts__ScriptSpace, CTheScripts__pActiveScripts;
	ptr *CTheScripts__ScriptSpace_LCS;

	// @CTheScripts::StartNewScript
	typedef ptr (*fn_CTheScripts__StartNewScript)(uint32_t);
	fn_CTheScripts__StartNewScript CTheScripts__StartNewScript;

	// @CRunningScript::GetPointerToScriptVariable SA
	typedef ptr (*fn_SA_CRunningScript__GetPointerToScriptVariable)(ptr thiz, uint8_t);
	fn_SA_CRunningScript__GetPointerToScriptVariable SA_CRunningScript__GetPointerToScriptVariable;

	// @CRunningScript::GetPointerToScriptVariable VC3LCS
	typedef ptr (*fn_VC3LCS_CRunningScript__GetPointerToScriptVariable)(ptr thiz, uint32_t *ip, uint8_t);
	fn_VC3LCS_CRunningScript__GetPointerToScriptVariable VC3LCS_CRunningScript__GetPointerToScriptVariable;

	ptr CRunningScript__GetPointerToScriptVariable(ptr thiz, uint8_t param)
	{
		switch (game)
		{
		case GTA3:
		case GTAVC:
			return VC3LCS_CRunningScript__GetPointerToScriptVariable(thiz, cast<uint32_t *>(thiz + 0x10), param);
		case GTASA:
			return SA_CRunningScript__GetPointerToScriptVariable(thiz, param);
		case GTALCS:
			return VC3LCS_CRunningScript__GetPointerToScriptVariable(thiz, cast<uint32_t *>(thiz + 0x18), param);
		case GTAVCS:
			return VC3LCS_CRunningScript__GetPointerToScriptVariable(thiz, cast<uint32_t *>(thiz + 0x10), param);

		}
	}

	// @CRunningScript::CollectParameters SA
	typedef void (*fn_SA_CRunningScript__CollectParameters)(ptr thiz, uint32_t count);
	fn_SA_CRunningScript__CollectParameters SA_CRunningScript__CollectParameters;

	// @CRunningScript::CollectParameters VC3
	typedef void (*fn_VC3_CRunningScript__CollectParameters)(ptr thiz, uint32_t *ip, uint32_t count);
	fn_VC3_CRunningScript__CollectParameters VC3_CRunningScript__CollectParameters;

	// @CRunningScript::CollectParameters LCS
	typedef int (*fn_LCS_CRunningScript__CollectParameters)(ptr thiz, uint32_t *ip, uint32_t count, uint32_t *p);
	fn_LCS_CRunningScript__CollectParameters LCS_CRunningScript__CollectParameters;

	void CRunningScript__CollectParameters(ptr thiz, uint32_t count)
	{
		switch (game)
		{
		case GTA3:
		case GTAVC:
			VC3_CRunningScript__CollectParameters(thiz, cast<uint32_t *>(thiz + 0x10), count);
			break;
		case GTASA:
			SA_CRunningScript__CollectParameters(thiz, count);
			break;
		case GTALCS:
			LCS_CRunningScript__CollectParameters(thiz, cast<uint32_t *>(thiz + 0x18), count, ScriptParams);
			break;
		case GTAVCS:
			LCS_CRunningScript__CollectParameters(thiz, cast<uint32_t *>(thiz + 0x10), count, ScriptParams);
			break;
		}
	}

	// @CRunningScript::UpdateCompareFlag
	typedef void (*fn_CRunningScript__UpdateCompareFlag)(ptr thiz, bool flag);
	fn_CRunningScript__UpdateCompareFlag CRunningScript__UpdateCompareFlag;

	// inidcates when @CTheScripts::Load executing
	bool bCTheScripts__Load = false;

	// @CTheScripts::Load SA
	// starts custom scripts after game loads saved scripts, custom scripts start in CTheScripts::Init must be disabled
	typedef void (*fn_SA_CTheScripts__Load)();
	fn_SA_CTheScripts__Load _SA_CTheScripts__Load, SA_CTheScripts__Load_;
	void SA_CTheScripts__Load()
	{
		utils::log("CTheScripts::Load()");
		bCTheScripts__Load = true;
		SA_CTheScripts__Load_();
		bCTheScripts__Load = false;
		preload_scripts();
		launch_scripts();
	}

	// @CTheScripts::Load VC3LCS
	typedef void (*fn_VC3_CTheScripts__Load)(uint32_t, uint32_t);
	fn_VC3_CTheScripts__Load _VC3_CTheScripts__Load, VC3_CTheScripts__Load_;
	void VC3_CTheScripts__Load(uint32_t a, uint32_t b)
	{
		utils::log("CTheScripts::Load()");
		bCTheScripts__Load = true;
		VC3_CTheScripts__Load_(a, b);
		bCTheScripts__Load = false;
		preload_scripts();
		launch_scripts();
	}

	// @CTheScripts::Init
	// starts custom scripts after game loads scm
	typedef void (*fn_CTheScripts__Init)();
	fn_CTheScripts__Init _CTheScripts__Init, CTheScripts__Init_;
	void CTheScripts__Init()
	{
		utils::log("CTheScripts::Init()");
		CTheScripts__Init_();
		if (!bCTheScripts__Load)
		{
			preload_scripts();
			launch_scripts();
		}
	}

	// @CTheScripts::Init LCS
	typedef int32_t (*fn_LCS_CTheScripts__Init)(bool b);
	fn_LCS_CTheScripts__Init _LCS_CTheScripts__Init, LCS_CTheScripts__Init_;
	int32_t LCS_CTheScripts__Init(bool b)
	{
		/*typedef int32_t (*fn_LCS_CFileMgr__LoadFile)(const char *fname, uint8_t *buf, int32_t size, const char *mode);
		fn_LCS_CFileMgr__LoadFile LoadFile = getsym<fn_LCS_CFileMgr__LoadFile>("_ZN8CFileMgr8LoadFileEPKcPhiS1_");
		uint8_t *buf = (uint8_t *)malloc(1024 * 1024 * 4);
		int32_t size = LoadFile("data/main.scm", buf, 1024 * 1024 * 4, "rb");
		if (size > 0)
		{
			FILE *f = fopen("/sdcard/main.scm", "wb");
			fwrite(buf, 1, size, f);
			fclose(f);
		}*/

		utils::log("CTheScripts::Init()");
		int32_t res = LCS_CTheScripts__Init_(b);
		CTheScripts__ScriptSpace = *CTheScripts__ScriptSpace_LCS; // dynamic allocation
		preload_scripts();
		launch_scripts();
		return res;
	}

	// @CTheScripts::Save SA
	// removes custom scripts from active scripts list, calls save and adds custom scripts to active list
	typedef void (*fn_SA_CTheScripts__Save)();
	fn_SA_CTheScripts__Save _SA_CTheScripts__Save, SA_CTheScripts__Save_;
	void SA_CTheScripts__Save()
	{
		utils::log("CTheScripts::Save()");
		save_scripts();
	}

	// @CTheScripts::Save VC3LCS
	typedef void (*fn_VC3LCS_CTheScripts__Save)(uint32_t, uint32_t);
	fn_VC3LCS_CTheScripts__Save _VC3LCS_CTheScripts__Save, VC3LCS_CTheScripts__Save_;
	void VC3LCS_CTheScripts__Save(uint32_t a, uint32_t b)
	{
		utils::log("CTheScripts::Save()");
		save_scripts(a, b);
	}

	// @CRunningScript::ProcessOneCommand
	// executes one opcode, returns true when thread switch should occure
	typedef bool (*fn_CRunningScript__ProcessOneCommand)(ptr thiz);
	fn_CRunningScript__ProcessOneCommand _CRunningScript__ProcessOneCommand, CRunningScript__ProcessOneCommand_;
	bool CRunningScript__ProcessOneCommand(ptr thiz)
	{
		process_opcodes(thiz);
		return true;
	}

	// @CText::Get
	// gets gxt entry by name
	ptr CTextHandle = NULL;
	typedef uint16_t *(*fn_CText__Get)(ptr thiz, LPCSTR name);
	fn_CText__Get _CText__Get, CText__Get_;
	uint16_t *CText__Get(ptr thiz, LPCSTR name)
	{		
		//utils::log("CText::Get(): %s", name);
		CTextHandle = thiz;

		if (CTextHandle && !psplang::is_init())
			psplang::init();

		uint16_t *e = text::get_gxt_entry(name);
		return e ? e : CText__Get_(thiz, name);
	}

	uint16_t *GetGxtEntry(LPCSTR name, bool useCustom)
	{
		if (!CTextHandle || !name || !name[0] || (game == GTA3 && strlen(name) > 7))
			return NULL;
		uint16_t *e = useCustom ? text::get_gxt_entry(name) : NULL;
		return e ? e : CText__Get_(CTextHandle, name);
	}

	// sceCtrlReadBufferPositive
	typedef void (*fn_CPad__UpdatePads)();
	fn_CPad__UpdatePads _CPad__UpdatePads, CPad__UpdatePads_;
	void CPad__UpdatePads()
	{
		//utils::log("CPad::UpdatePads");
		CPad__UpdatePads_();

		touch::psp_input_event();
		ui::handle_psp_controls();
	}

	// legacy external storage path
	std::string get_storage_dir()
	{		
		char dir[64];
		sprintf(dir, "ms0:/PSP/PLUGINS/cleo/%s/", select("", "", "", "lcs", "vcs"));
		return dir;
	}

	uint32_t VC3LCS_CRunningScript__ProcessOneCommand_call_1,
			 VC3LCS_CRunningScript__ProcessOneCommand_call_2,
			 SA_CRunningScript__ProcessOneCommand_call,
			 SA_CRunningScript__ProcessOneCommand_call_end,
			 VC3SA_NvEventQueueActivity__init_ptr;

	bool init_pattern()
	{
		#define PATTERN_NOT_FOUND {	utils::log("required pattern %d not found!", __LINE__); return false; }
		#define FIND_PATTERN(...) if (!__FindPatternAddress(addr, __VA_ARGS__)) PATTERN_NOT_FOUND;
		
		uint32_t addr;

		// CRunningScript__ProcessOneCommand
		switch (game)
		{
		case GTALCS:
			FIND_PATTERN("2D 20 00 02 00 00 00 00 00 00 00 00 ?? ?? ?? ?? 00 00 00 00");
			VC3LCS_CRunningScript__ProcessOneCommand_call_1 = addr - 4;
			
			break;
		case GTAVCS:
			FIND_PATTERN("2B 18 64 00 ?? ?? ?? ?? 0B 02 02 92");
			VC3LCS_CRunningScript__ProcessOneCommand_call_1 = addr + 12;
			break;
		}

		return true;
	}

	void init_fixes()
	{

	}

	bool init_code()
	{
		if (!init_pattern())
			return false;

		if (game == GTALCS)
		{	
			uint32_t addr;

			#define READ_ADDR(addr1, addr2) ((static_cast<uint32_t>(*cast<uint16_t *>(addr1)) << 16) | *cast<uint16_t *>(addr2))
			#define READ_ADDR_INDIRECT(addr1, addr2) (READ_ADDR(addr1, addr2) - 0x10000)
			#define READ_ADDR_GP(addr) (libres::getGpValue() + *cast<int16_t *>(addr))
			#define READ_REL3_ADDR(offset) memutils::mem_read_mips_jmp(cast<ptr>(addr) + offset)

			// ScriptParams
			FIND_PATTERN("?? ?? 02 3C 2D 80 C0 00 ?? ?? 51 24");
			ScriptParams = cast<uint32_t*>(READ_ADDR_INDIRECT(addr, addr + 8));
			utils::log("ScriptParams: 0x%08X", ScriptParams);

			// _ZN6CTimer22m_snTimeInMillisecondsE
			FIND_PATTERN("FF 3F 43 30 ?? ?? ?? ?? 01 00 42 30 00 60 83 44");
			CTimer__m_snTimeInMilliseconds = cast<uint32_t*>(READ_ADDR_INDIRECT(addr - 8, addr - 4));
			utils::log("CTimer__m_snTimeInMilliseconds: 0x%08X", CTimer__m_snTimeInMilliseconds);

			// _ZN14CRunningScript17ProcessOneCommandEv
			FIND_PATTERN("18 00 C4 8C 21 10 44 00");
			_CRunningScript__ProcessOneCommand = cast<fn_CRunningScript__ProcessOneCommand>(addr - 16); //

			// _ZN11CTheScripts11ScriptSpaceE
			CTheScripts__ScriptSpace_LCS = cast<ptr*>(READ_ADDR_GP(addr - 4));
			utils::log("CTheScripts__ScriptSpace_LCS: 0x%08X", CTheScripts__ScriptSpace_LCS);

			// _ZN11CTheScripts14pActiveScriptsE
			FIND_PATTERN("14 00 B4 27 18 00 B6 27 1C 00 B7 27 21 88 02 02 00 00 00 00");
			CTheScripts__pActiveScripts = cast<ptr>(READ_ADDR_INDIRECT(addr - 8, addr - 4));
			utils::log("CTheScripts__pActiveScripts: 0x%08X", CTheScripts__pActiveScripts);

			// _ZN11CTheScripts14StartNewScriptEi
			FIND_PATTERN("08 00 B1 FF 2D 88 80 00 10 00 BF FF ?? ?? ?? ?? 2D 20 00 02");
			CTheScripts__StartNewScript = cast<fn_CTheScripts__StartNewScript>(addr - 24);
			utils::log("CTheScripts__StartNewScript: 0x%08X", CTheScripts__StartNewScript);

			// _ZN5CText3GetEPKc
			FIND_PATTERN("2D 30 A0 03 18 00 B1 FF 2D 80 80 00 2D 88 A0 00 20 00 BF FF ?? ?? ?? ?? 00 00 A0 A3 00 00 A3 93 ?? ?? ?? ?? 2D 20 40 00");
			_CText__Get = cast<fn_CText__Get>(addr - 8);
			utils::log("_CText__Get: 0x%08X", _CText__Get);

			// _ZN14CRunningScript26GetPointerToScriptVariableEPjh
			FIND_PATTERN("?? ?? ?? ?? 00 00 BF FF ?? ?? ?? ?? FF 00 C6 30");
			VC3LCS_CRunningScript__GetPointerToScriptVariable = cast<fn_VC3LCS_CRunningScript__GetPointerToScriptVariable>(addr);
			utils::log("VC3LCS_CRunningScript__GetPointerToScriptVariable: 0x%08X", VC3LCS_CRunningScript__GetPointerToScriptVariable);

			// _ZN11CTheScripts14SaveAllScriptsEPhPj
			FIND_PATTERN("28 00 B5 FF 2D 48 A0 00");
			_VC3LCS_CTheScripts__Save = cast<fn_VC3LCS_CTheScripts__Save>(addr - 12);
			utils::log("_VC3LCS_CTheScripts__Save: 0x%08X", _VC3LCS_CTheScripts__Save);

			// _ZN14CRunningScript17CollectParametersEPjiPi
			FIND_PATTERN("08 00 B1 FF 2D 80 A0 00 10 00 B2 FF 2D 88 E0 00");
			LCS_CRunningScript__CollectParameters = cast<fn_LCS_CRunningScript__CollectParameters>(addr - 8);
			utils::log("LCS_CRunningScript__CollectParameters: 0x%08X", LCS_CRunningScript__CollectParameters);

			// _ZN11CTheScripts4InitEb
			FIND_PATTERN("2D F0 80 00 30 00 B0 FF");
			_LCS_CTheScripts__Init = cast<fn_LCS_CTheScripts__Init>(addr - 8); //
			utils::log("_LCS_CTheScripts__Init: 0x%08X", _LCS_CTheScripts__Init);

			// sceCtrlReadBufferPositive
			FIND_PATTERN("?? ?? ?? ?? 2D 20 00 00 2D 28 00 00 ?? ?? ?? ?? 2D 20 40 00 ?? ?? ?? ?? 01 00 04 24");
			_CPad__UpdatePads = cast<fn_CPad__UpdatePads>(addr - 8);
			touch::CPad__GetPad = cast<touch::fn_CPad__GetPad>(READ_REL3_ADDR(0));
			utils::log("_CPad__UpdatePads: 0x%08X", _CPad__UpdatePads);
			
			armhook::hook_mips_func(_VC3LCS_CTheScripts__Save, 8, VC3LCS_CTheScripts__Save, &VC3LCS_CTheScripts__Save_);
			CRunningScript__ProcessOneCommand_ = _CRunningScript__ProcessOneCommand;
			armhook::replace_mips_call(VC3LCS_CRunningScript__ProcessOneCommand_call_1, CRunningScript__ProcessOneCommand);
			armhook::hook_mips_func(_LCS_CTheScripts__Init, 8, LCS_CTheScripts__Init, &LCS_CTheScripts__Init_);
			armhook::hook_mips_func(_CText__Get, 8, CText__Get, &CText__Get_);
			armhook::hook_mips_func(_CPad__UpdatePads, 8, CPad__UpdatePads, &CPad__UpdatePads_);
		}
		else if (game == GTAVCS)
		{
			uint32_t addr;

			// ScriptParams
			FIND_PATTERN("?? ?? 11 3C 10 00 12 26 18 00 BF FF ?? ?? ?? ?? ?? ?? ?? ?? 2D 28 40 02 ?? ?? ?? ?? 2D 20 00 02 2D 28 40 02");
			ScriptParams = cast<uint32_t*>(READ_ADDR_INDIRECT(addr, addr + 12));
			utils::log("ScriptParams: 0x%08X", ScriptParams);

			// _ZN6CTimer22m_snTimeInMillisecondsE
			FIND_PATTERN("4D 00 03 3C ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 23 10 43 00");
			CTimer__m_snTimeInMilliseconds = cast<uint32_t*>(READ_ADDR_INDIRECT(addr, addr + 4));
			utils::log("CTimer__m_snTimeInMilliseconds: 0x%08X", CTimer__m_snTimeInMilliseconds);

			// _ZN11CTheScripts11ScriptSpaceE
			FIND_PATTERN("48 00 03 3C 00 00 25 8E");
			CTheScripts__ScriptSpace_LCS = cast<ptr*>(READ_ADDR(addr, addr + 8));
			utils::log("CTheScripts__ScriptSpace_LCS: 0x%08X", CTheScripts__ScriptSpace_LCS);

			// _ZN11CTheScripts14pActiveScriptsE
			FIND_PATTERN("4D 00 05 3C 2D 20 00 02 ?? ?? ?? ?? ?? ?? ?? ?? 01 00 03 24 2D 10 00 02");
			CTheScripts__pActiveScripts = cast<ptr>(READ_ADDR_INDIRECT(addr, addr + 12));
			utils::log("CTheScripts__pActiveScripts: 0x%08X", CTheScripts__pActiveScripts);

			// _ZN11CTheScripts14StartNewScriptEi
			FIND_PATTERN("00 00 B0 FF ?? ?? ?? ?? 08 00 B1 FF ?? ?? ?? ?? 2D 88 80 00 10 00 BF FF ?? ?? ?? ?? 2D 20 00 02");
			CTheScripts__StartNewScript = cast<fn_CTheScripts__StartNewScript>(addr - 8);

			// _ZN14CRunningScript17ProcessOneCommandEv
			FIND_PATTERN("10 00 BF FF 48 00 02 3C");
			_CRunningScript__ProcessOneCommand = cast<fn_CRunningScript__ProcessOneCommand>(addr - 8); //

			// _ZN5CText3GetEPKc
			FIND_PATTERN("2D 30 A0 03 18 00 B1 FF 2D 80 80 00 2D 88 A0 00 20 00 BF FF ?? ?? ?? ?? 00 00 A0 A3 00 00 A3 93 ?? ?? ?? ?? 2D 20 40 00");
			_CText__Get = cast<fn_CText__Get>(addr - 8);
			utils::log("_CText__Get: 0x%08X", _CText__Get);

			// GetPointerToScriptVariable
			FIND_PATTERN("00 00 E6 8C 2D 60 40 00");
			VC3LCS_CRunningScript__GetPointerToScriptVariable = cast<fn_VC3LCS_CRunningScript__GetPointerToScriptVariable>(addr - 8);

			// _ZN11CTheScripts14SaveAllScriptsEPhPj
			FIND_PATTERN("4D 00 03 3C 28 00 B5 FF 2D 48 80 00");
			_VC3LCS_CTheScripts__Save = cast<fn_VC3LCS_CTheScripts__Save>(addr - 12);

			// _ZN14CRunningScript17CollectParametersEPjiPi
			FIND_PATTERN("10 00 B2 FF 2D 88 A0 00 18 00 B3 FF 2D 90 E0 00 20 00 B4 FF ?? ?? ?? ?? 00 00 B0 FF 2D A0 80 00");
			LCS_CRunningScript__CollectParameters = cast<fn_LCS_CRunningScript__CollectParameters>(addr - 8);

			// _ZN11CTheScripts4InitEb
			FIND_PATTERN("30 00 B0 FF 4D 00 05 3C");
			_LCS_CTheScripts__Init = cast<fn_LCS_CTheScripts__Init>(addr - 8); //

			// sceCtrlReadBufferPositive
			FIND_PATTERN("?? ?? ?? ?? 2D 20 00 00 2D 28 00 00 ?? ?? ?? ?? 2D 20 40 00 ?? ?? ?? ?? 01 00 04 24");
			touch::CPad__GetPad = cast<touch::fn_CPad__GetPad>(READ_REL3_ADDR(0));
			_CPad__UpdatePads = cast<fn_CPad__UpdatePads>(addr - 8);

			armhook::hook_mips_func(_VC3LCS_CTheScripts__Save, 8, VC3LCS_CTheScripts__Save, &VC3LCS_CTheScripts__Save_);
			CRunningScript__ProcessOneCommand_ = _CRunningScript__ProcessOneCommand;
			armhook::replace_mips_call(VC3LCS_CRunningScript__ProcessOneCommand_call_1, CRunningScript__ProcessOneCommand);
			armhook::hook_mips_func(_LCS_CTheScripts__Init, 8, LCS_CTheScripts__Init, &LCS_CTheScripts__Init_);
			armhook::hook_mips_func(_CText__Get, 8, CText__Get, &CText__Get_);
			armhook::hook_mips_func(_CPad__UpdatePads, 8, CPad__UpdatePads, &CPad__UpdatePads_);
		}
		
		return true;
	}

	// calls all initializers
	void initialize()
	{
		mutex::init();

		utils::log(VERSION_STR);
		utils::log("initialize start");

		// resolve main game lib
		if (libres::init(game, image_base))
		{
			utils::log("game's (index %d) main library resolved (image base 0x%08X)", game, image_base);
		} else 
		{
			utils::log("initialize failed - can't resolve game's main library");
			return;
		}

		game_ver = game == GTALCS ? VER_GTALCS_PSP_1_05_OR_HIGHER : VER_GTAVCS_PSP_1_02_OR_HIGHER;

		// print game version
		utils::log("game ver: %s", str_game_version[game_ver]);

		// init armhook
		armhook::init();

		// init code
		if (init_code())
		{
			utils::log("code initialized");
		} else
		{
			utils::log("initialize failed - can't initialize code");
			return;
		}

		// init ui code
		ui::init();
		utils::log("ui code initialized");

		// init text
		text::init();

		utils::log("initialize success");
	}

	// script code buf
	// copies script to the buf and adds it to the list
	void preload_script(std::string fname, const uint8_t *code, uint32_t code_size, bool is_invokable, uint32_t invokable_id)
	{
		// copy script to the script code buf
		if (game == GTALCS || game == GTAVCS) // has to be fixed in SB
		{
			if (code_size >= 8)
			{
				uint64_t pad;
				memcpy(&pad, code, sizeof(pad));
				if (!pad)
				{
					code += 8;
					code_size -= 8;
				}
			}
		}
		uint8_t *script_code;
		script_code = new uint8_t[code_size + 8];		
		memcpy(script_code, code, code_size);

		// fill script desc and add it to the scripts arr
		t_script *script = new t_script();
		script->invokable_id = is_invokable ? invokable_id : -1;
		script->name = fname;
		script->code = script_code;
		script->code_size = code_size;
		script->offset = game == GTASA ? cast<uint32_t>(script->code) : cast<uint32_t>(script->code) - cast<uint32_t>(CTheScripts__ScriptSpace);
		scripts.push_back(script);

		// set invokable script name for menu
		if (is_invokable)
			text::set_gxt_invokable_script_name(invokable_id, fname);
	}

	// preloads scripts at start
	void preload_scripts()
	{
		// clear scripts
		for (int i = 0; i < scripts.size(); i++)
		{
			delete[] scripts[i]->code;
			delete scripts[i];
		}
		scripts.clear();
		// init gxt entries
		text::init();
		// delete menu ui
		ui::delete_menu();
		// init script menu for sa
		switch (game)
		{
		case GTALCS:
			preload_script("menu", lcs_menu_script, sizeof(lcs_menu_script), false, 0);
			break;
		case GTAVCS:
			preload_script("menu", vcs_menu_script, sizeof(vcs_menu_script), false, 0);
			break;
		}
		// check all found files
		uint32_t invokable_count = 0;

		uintptr_t script_offset = (uintptr_t)(&CLEOScripts);
		do
		{
			std::string fname = (const char*)script_offset;
			size_t script_name_len = fname.size() + 1;
			script_offset += script_name_len;
			uint32_t script_size = *(uint32_t*)(script_offset);
			script_offset += sizeof(uint32_t);

			if (!script_size)
				break;

			if (script_offset + script_size > ((uintptr_t)(&CLEOScripts) + sizeof(CLEOScripts)))
				break;

			std::string root = strutils::get_parent_path(fname, true);
			fname = strutils::get_filename(fname);

			if ((game == GTALCS && root == "lcs") || (game == GTAVCS && root == "vcs") || root == "")
			{
				std::string ext = strutils::get_ext(fname);
				bool is_invokable = ext == "csi";
				if (ext == "csa" || is_invokable)
				{
					// preload script
					uint32_t code_size = script_size;
					uint8_t* code = (uint8_t*)script_offset;
					if (!code || !code_size)
					{
						utils::log("can't read script '%s'", fname.c_str());
						continue;
					}
					preload_script(fname, code, code_size, is_invokable, invokable_count);
					if (is_invokable)
						invokable_count++;
					utils::log("script '%s' with size %d preloaded", fname.c_str(), code_size);
				}
				else if (ext == "fxt")
				{
					// load fxt
					uint32_t size = script_size;
					LPSTR text_raw = cast<LPSTR>(script_offset);
					if (!text_raw || !size)
					{
						utils::log("can't read fxt '%s'", fname.c_str());
						continue;
					}
					LPSTR text = cast<LPSTR>(malloc(size + 1));
					memcpy(text, text_raw, size);
					text[size] = 0;
					text::load_gxt_entries_from_text(text, size);
					free(text);
					free(text_raw);
					utils::log("fxt '%s' loaded", fname.c_str());
				}
			}
			script_offset += script_size;

		} while (true);

		utils::log("total preloaded scripts %d", scripts.size());
	}

	// launches custom scripts
	void launch_scripts()
	{
		utils::log("launch_scripts start");
		mutex_vars.clear();
		for (int32_t i = 0; i < scripts.size(); i++)
		{
			t_script *script = scripts[i];
			// only normal custom scripts start here
			if (script->invokable_id == -1)
			{
				utils::log("starting script '%s'", script->name.c_str());
				script->handle = CTheScripts__StartNewScript(script->offset);
				script->wait_passed = false;
			} else // invokable scripts start by special opcode and don't require wait
			{
				script->handle = NULL;
				script->wait_passed = true;
			}
		}
		utils::log("launch_scripts success");
	}

	void save_scripts(uint32_t a, uint32_t b)
	{
		utils::log("save_scripts start");

		struct t_linked_script
		{
			t_linked_script *next;
			t_linked_script *prev;
			char *get_name()
			{
				return cast<char *>(cast<ptr>(this) + select(8, 8, 8, 16, 0x20F));
			}
		};

		ptr game_active_scripts = CTheScripts__pActiveScripts;

		t_linked_script *curActiveScript, *firstActiveScript = NULL;

		curActiveScript = *cast<t_linked_script **>(game_active_scripts);
		while (curActiveScript)
		{
			bool orig = get_script_using_handle(cast<ptr>(curActiveScript)) == NULL;
			if (orig)
			{
				if (!firstActiveScript)
				{
					firstActiveScript = curActiveScript;
					*cast<t_linked_script **>(game_active_scripts) = firstActiveScript;
				}
			} else
			{
				utils::log("skipping '%.8s'", curActiveScript->get_name());
				if (curActiveScript->prev)
					curActiveScript->prev->next = curActiveScript->next;
				if (curActiveScript->next)
					curActiveScript->next->prev = curActiveScript->prev;
			}
			curActiveScript = curActiveScript->next;
		}

		if (game == GTASA)
			SA_CTheScripts__Save_();
		else
			VC3LCS_CTheScripts__Save_(a, b);

		// find last active
		curActiveScript = *cast<t_linked_script **>(game_active_scripts);
		while (curActiveScript->next)
			curActiveScript = curActiveScript->next;

		for (int32_t i = 0; i < scripts.size(); i++)
		{
			if (!scripts[i]->handle)
				continue;
			curActiveScript->next = cast<t_linked_script *>(scripts[i]->handle);
			curActiveScript->next->prev = curActiveScript;
			curActiveScript->next->next = NULL;
			curActiveScript = curActiveScript->next;
		}

		utils::log("save_scripts success");
	}

	ptr get_real_code_ptr(uint32_t ip)
	{
		if (game != GTASA) ip += cast<uint32_t>(CTheScripts__ScriptSpace);
		return cast<ptr>(ip);
	}

	uint32_t get_ip_using_real_code_ptr(ptr realPtr)
	{
		if (game != GTASA) realPtr -= cast<uint32_t>(CTheScripts__ScriptSpace);
		return cast<uint32_t>(realPtr);
	}

	ptr get_real_label_ptr(ptr handle, uint32_t offset)
	{
		if (t_script *script = get_script_using_handle(handle))
		{
			int32_t offset_signed = *cast<int32_t *>(&offset);
			offset = (offset_signed >= 0) ? (cast<uint32_t>(script->code) + offset_signed) : (cast<uint32_t>(script->code) - offset_signed);
			return cast<ptr>(offset);
		}
		return NULL;
	}

	bool read_str_8byte(ptr handle, std::string &str)
	{
		uint32_t *p_ip = cast<uint32_t *>(handle + select(0x10, 0x10, 0x14, 0x18, 0x10));
		ptr code = get_real_code_ptr(*p_ip);
		str.clear();
		if (game != GTAVCS)
		{
			str.append(cast<const char *>(code));
			*p_ip += 8;
		} else
		{
			if (*code != 0x0A)
				return false;
			str.append(cast<const char *>(code + 1));
			*p_ip += str.size() + 2;
		}
		return true;
	}

	bool read_str_long(ptr handle, std::string &str)
	{
		uint32_t *p_ip = cast<uint32_t *>(handle + select(0x10, 0x10, 0x14, 0x18, 0x10));
		ptr code = get_real_code_ptr(*p_ip);
		if (*code != select(0x0E, 0x0E, 0x0E, 0x6B, 0x6B))
			return false;
		str.clear();
		str.append(cast<const char *>(code + 2), *(code + 1));
		*p_ip += *(code + 1) + 2;
		return true;
	}

	// implements custom opcodes
	bool custom_opcode(t_script& script, uint16_t op);
	// processes opcodes until opcode after which CRunningScript switch occures
	void process_opcodes(ptr handle)
	{
		// check if this thread handle is from our script
		bool handle_found = false;
		bool wait_passed = false;
		uint8_t *code = NULL;
		std::string name;
		t_script *script = get_script_using_handle(handle);
		if (script)
		{
			code = script->code;
			name = script->name;
			wait_passed = script->wait_passed;
			handle_found = true;
		}

		//utils::log("%u handle_found %X %d", utils::get_tick_count(), handle, handle_found);

		// after the start custom scripts must be idle for a few seconds
		if (handle_found && !wait_passed)
		{
			uint32_t wait_time = (game == GTASA) ? 3000 : 2000;
			setfield<uint32_t>(handle, select(0x7C, 0x7C, 0xEC, 0x210, 0x200), *CTimer__m_snTimeInMilliseconds + wait_time);
			script->wait_passed = true;
			return;
		}

		// opcodes to replace
		uint16_t OP_J = 2,
				 OP_JT = 0x4C,
				 OP_JF = 0x4D,
				 OP_CALL = 0x50,
				 OP_RET = 0x51,
				 OP_ENDTHREAD = 0x4E,
				 OP_ENDCUSTOMTHREAD = 0x05DC;

		if (game == GTAVCS)
		{
			OP_JT = 0x21;
			OP_JF = 0x22;
			OP_CALL = 0x25;
			OP_RET = 0x26;
			OP_ENDTHREAD = 0x23;
		}

		bool result;
		do
		{
			ptr ip = getfield<ptr>(handle, select(0x10, 0x10, 0x14, 0x18, 0x10)); // ip, for SA is absolute ptr
			//utils::log("%s %08X %08X %08X", __FUNCTION__, handle, ip + cast<uint32_t>(CTheScripts__ScriptSpace), CTheScripts__ScriptSpace);
			if (game != GTASA) ip += cast<uint32_t>(CTheScripts__ScriptSpace);

			bool cond = getfield<uint8_t>(handle, select(0x78, 0x79, 0xE5, 0x20D, 0x209)) != 0; // thread if cond

			uint16_t op = *cast<uint16_t *>(ip);

			if (handle_found)
			{
				//utils::log("%s %X %X op %04X", cast<char *>(handle + select(8, 8, 8, 16, 0x20F)), handle, ip - code, op);

				if (op == OP_J || op == OP_JT || op == OP_JF || op == OP_CALL)
				{
					// move to opcode param
					ip += 2;
					// check param type
					if (!(*ip == 1 || ((game == GTALCS || game == GTAVCS) && *ip == 6)))
					{
						utils::log("wrong param type in '%s' at %d, terminating", name.c_str(), ip - code);
						exit(1);
					}
					ip++;
					// read offset as int
					int32_t offset_signed = *cast<int32_t *>(ip);
					ip += 4;
					// calc offset from ScriptSpace
					uint32_t offset = (offset_signed >= 0) ? (cast<uint32_t>(code) + offset_signed) : (cast<uint32_t>(code) - offset_signed);
					// OP_CALL saves thread ip on call stack
					if (op == OP_CALL)
					{
						//utils::log("OP_CALL");
						uint16_t si = getfield<uint16_t>(handle, select(0x2C, 0x2C, 0x38, 0x5C, 0x204));
						setfield<uint16_t>(handle, select(0x2C, 0x2C, 0x38, 0x5C, 0x204), si + 1);
						ptr vc3_ip = ip - cast<uint32_t>(CTheScripts__ScriptSpace);
						//utils::log("OP_CALL ret ip %X %X si %d", vc3_ip, ip, si);
						setfield<ptr>(handle, select(0x14, 0x14, 0x18, 0x1C, 0x14) + si * sizeof(uint32_t), select(vc3_ip, vc3_ip, ip, vc3_ip, vc3_ip));
					}
					// set new ip
					if ((op == OP_J) || (op == OP_CALL) || (op == OP_JT && cond) || (op == OP_JF && !cond))
						ip = cast<ptr>(offset);
					// set thread ip
					if (game != GTASA) ip -= cast<uint32_t>(CTheScripts__ScriptSpace);
					setfield<ptr>(handle, select(0x10, 0x10, 0x14, 0x18, 0x10), ip);
					result = false;
				} else
				if (op == OP_RET) // lcs uses it for ret from funcs as well, so let's replace it
				{					
					uint16_t si = getfield<uint16_t>(handle, select(0x2C, 0x2C, 0x38, 0x5C, 0x204)) - 1;
					setfield<ptr>(handle, select(0x10, 0x10, 0x14, 0x18, 0x10),
							getfield<ptr>(handle, select(0x14, 0x14, 0x18, 0x1C, 0x14) + si * sizeof(uint32_t)));
					setfield<uint16_t>(handle, select(0x2C, 0x2C, 0x38, 0x5C, 0x204), si);
					//utils::log("OP_RET ret ip %X si %d", getfield<ptr>(handle, select(0x10, 0x10, 0x14, 0x18)), si);
					result = false;
				} else
				if (op == OP_ENDTHREAD || op == OP_ENDCUSTOMTHREAD)
				{
					// replacement for thread end opcodes
					utils::log("terminating script '%s'", name.c_str());
					setfield<uint32_t>(handle, select(0x7C, 0x7C, 0xEC, 0x210, 0x200), 0xFFFFFFFF); // wait time
					result = true;
				} else
				{
					ptr *p_ip = cast<ptr*>(handle + select(0x10, 0x10, 0x14, 0x18, 0x10));
					*p_ip += 2;
					if (custom_opcode(*script, op) /*|| plugins::handle_opcode(script->handle, cast<uint32_t*>(p_ip), op)*/)
					{
						result = false;
					} else
					{
						*p_ip -= 2;
						result = CRunningScript__ProcessOneCommand_(handle);
					}
				}
			} else
				result = CRunningScript__ProcessOneCommand_(handle);

		} while (!result);
	}

	bool custom_opcode(t_script &script, uint16_t op)
	{
		const uint16_t OP_GET_LABEL_ADDR = 0x0DD0,
					   OP_GET_FUNC_ADDR_CSTR = 0x0DD1,
					   OP_CONTEXT_CALL = 0x0DD2,
					   OP_CONTEXT_SET_REG = 0x0DD3,
					   OP_CONTEXT_GET_REG = 0x0DD4,
					   OP_GET_PLATFORM = 0x0DD5,
					   OP_GET_GAME_VER = 0x0DD6,
					   OP_GET_IMAGE_BASE = 0x0DD7,
					   OP_READ_MEM = 0x0DD8,
					   OP_WRITE_MEM = 0x0DD9,
					   OP_SEARCH_MEM = 0x0DDA,
					   OP_GET_GAME_VER_EX = 0x0DDB,
					   OP_SET_MUTEX_VAR = 0x0DDC,
					   OP_GET_MUTEX_VAR = 0x0DDD,
					   OP_CALL_FUNCTION = 0x0DDE,
					   OP_GET_TOUCH_POINT_STATE = 0x0DE0,
					   OP_GET_TOUCH_SLIDE_STATE = 0x0DE1,
					   OP_GET_MENU_BUTTON_STATE = 0x0DE2,
					   OP_GET_MENU_BUTTON_PRESSED = 0x0DE3,
					   OP_GET_PSP_CONTROL_STATE = 0x0DE4,
					   OP_GET_PSP_CONTROL_PRESSED = 0x0DE5,
					   OP_INVOKABLE_SCRIPT_STATS = 0x0DEE,
					   OP_START_INVOKABLE_SCRIPT = 0x0DEF,
					   OP_SHOW_MENU_OPEN_ARROW = 0x0DF0,
					   OP_HIDE_MENU_OPEN_ARROW = 0x0DF1,
					   OP_CREATE_MENU = 0x0DF2,
					   OP_DELETE_MENU = 0x0DF3,
					   OP_GET_MENU_TOUCHED_ITEM_INDEX = 0x0DF4,
					   OP_SET_MENU_ACTIVE_ITEM_INDEX = 0x0DF5,
					   OP_GET_MENU_ACTIVE_ITEM_INDEX = 0x0DF6;

		switch (op)
		{
		case OP_GET_LABEL_ADDR:
		{
			//utils::log("OP_GET_LABEL_ADDR");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t offset = ScriptParams[0];
			int32_t  offset_signed = *cast<int32_t *>(&offset);
			offset = (offset_signed >= 0) ? (cast<uint32_t>(script.code) + offset_signed) : (cast<uint32_t>(script.code) - offset_signed);
			*v = offset;
			return true;
		}
		case OP_GET_FUNC_ADDR_CSTR:
		{
			//utils::log("OP_GET_FUNC_ADDR_CSTR");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);			
			*v = 0;
			//utils::log("func '%s' addr is 0x%X", func_name, *v);
			return true;
		}
		case OP_CONTEXT_CALL:
		{
			//utils::log("OP_CONTEXT_CALL");
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t func = ScriptParams[0];
			//utils::log("call func 0x%x", func);
			asm_call(script.context, func);
			return true;
		}
		case OP_CONTEXT_SET_REG:
		{
			//utils::log("OP_CONTEXT_SET_REG");
			CRunningScript__CollectParameters(script.handle, 2);
			uint32_t reg = ScriptParams[0];
			uint32_t val = ScriptParams[1];
			script.context[reg] = val;
			return true;
		}
		case OP_CONTEXT_GET_REG:
		{
			//utils::log("OP_CONTEXT_GET_REG");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t reg = ScriptParams[0];
			*v = script.context[reg];
			return true;
		}
		case OP_GET_PLATFORM:
		{
			//utils::log("OP_GET_PLATFORM");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*v = 2;
			return true;
		}
		case OP_GET_GAME_VER:
		{
			//utils::log("OP_GET_GAME_VER");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*v = game_ver;
			return true;
		}
		case OP_GET_IMAGE_BASE:
		{
			//utils::log("OP_GET_IMAGE_BASE");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*v = image_base;
			return true;
		}
		case OP_READ_MEM:
		{
			//utils::log("OP_READ_MEM");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 3);
			uint32_t addr = ScriptParams[0];
			uint32_t size = ScriptParams[1];
			uint32_t correct_ib = ScriptParams[2];
			if (correct_ib)
				addr += image_base;
			*v = 0;
			memcpy(v, cast<const void *>(addr), size);
			return true;
		}
		case OP_WRITE_MEM:
		{
			//utils::log("OP_WRITE_MEM");
			CRunningScript__CollectParameters(script.handle, 5);
			uint32_t addr = ScriptParams[0];
			uint32_t val = ScriptParams[1];
			uint32_t size = ScriptParams[2];
			uint32_t correct_ib = ScriptParams[3];
			uint32_t protect = ScriptParams[4];
			if (correct_ib)
				addr += image_base;
			memutils::mem_write_arr(cast<uint8_t *>(addr), cast<uint8_t *>(&val), size, protect);
			return true;
		}
		case OP_SEARCH_MEM:
		{
			//utils::log("OP_SEARCH_MEM");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 2);
			LPSTR pattern = cast<LPSTR>(ScriptParams[0]);
			uint32_t index = ScriptParams[1];			
			if (!__FindPatternAddressCompact(*v, pattern, index))
				*v = 0;
			//utils::log("pattern '%s' index %d addr is 0x%X", pattern, index, *v);
			return true;
		}
		case OP_GET_GAME_VER_EX:
		{
			//utils::log("OP_GET_GAME_VER_EX");
			uint32_t *id = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			uint32_t *ver = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			uint32_t *ver_code = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*id = 0;
			sscanf(package_name.c_str() + 4, "%d", id);			
			*ver = strutils::str_hash(package_version_name);
			*ver_code = package_version_code;
			return true;
		}
		case OP_SET_MUTEX_VAR:
		{
			//utils::log("OP_SET_MUTEX_VAR");
			CRunningScript__CollectParameters(script.handle, 2);
			uint32_t id = ScriptParams[0];
			uint32_t v = ScriptParams[1];
			if (mutex_vars.size())
				for (int32_t i = 0; i < mutex_vars.size(); i++)
					if (mutex_vars.at(i).id == id)
					{
						mutex_vars.at(i).value = v;
						return true;
					}
			t_mutex_var mv;
			mv.id = id;
			mv.value = v;
			mutex_vars.push_back(mv);
			return true;
		}
		case OP_GET_MUTEX_VAR:
		{
			//utils::log("OP_GET_MUTEX_VAR");
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t id = ScriptParams[0];
			*v = 0;
			if (mutex_vars.size())
				for (int32_t i = 0; i < mutex_vars.size(); i++)
					if (mutex_vars.at(i).id == id)
					{
						*v = mutex_vars.at(i).value;
						break;
					}
			return true;
		}
		case OP_CALL_FUNCTION:
		{
			//utils::log("OP_CALL_FUNCTION");
			CRunningScript__CollectParameters(script.handle, 2);
			uint32_t addr = ScriptParams[0];
			uint32_t add_ib = ScriptParams[1];
			if (add_ib) addr += image_base;

			int params_i[8];
			memset(params_i, 0, sizeof(params_i));

			float params_f[8];
			memset(params_f, 0, sizeof(params_f));

			bool is_res_float = false;	

			uint32_t params_a[32]; // any
			memset(params_a, 0, sizeof(params_a));

			int params_i_count = 0;
			int params_f_count = 0;
			int params_a_count = 0;

			void *res_ptr = NULL;			

			while (true)
			{				
				// see if this is opcode param list end
				uint32_t *p_ip = cast<uint32_t *>(script.handle + select(0x10, 0x10, 0x14, 0x18, 0x10));
				ptr code = get_real_code_ptr(*p_ip);
				if (*code == 0)
				{
					*p_ip += 1;
					break;
				}

				// read string type
				std::string str;
				if (!read_str_8byte(script.handle, str))
				{
					utils::log("func call param type has to be 8byte string");
					exit(1);
				}
				str = strutils::str_to_lower(str);

				// param type enum
				enum eParamType
				{
					ptNone,
					ptInt,
					ptFloat
				};

				// parse param
				eParamType pt = ptNone;
				uint32_t int_val = 0;
				float float_val = 0.0f;
				if (str == "i")
				{
					CRunningScript__CollectParameters(script.handle, 1);
					int_val = ScriptParams[0];
					pt = ptInt;
				} else
				if (str == "f")
				{
					CRunningScript__CollectParameters(script.handle, 1);
					float_val = *cast<float *>(&ScriptParams[0]);
					pt = ptFloat;
				} else
				if (str == "ref")
				{
					int_val = cast<uint32_t>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
					pt = ptInt;
				} else
				if (str == "resi")
				{
					res_ptr = CRunningScript__GetPointerToScriptVariable(script.handle, 0);
				} else
				if (str == "resf")
				{
					res_ptr = CRunningScript__GetPointerToScriptVariable(script.handle, 0);
					is_res_float = true;
				} else
				{
					utils::log("func call has unknown param type '%s'", str.c_str());
					exit(1);
				}

				// if this is a func param then put it into one of the func param arrays
				if (pt != ptNone)
				{
					// verify func param limit
					if (params_i_count + params_f_count + params_a_count == 32)
					{
						utils::log("func call has more than 32 params");
						exit(1);
					}
					if (pt == ptInt)
					{
						if (params_i_count < 8)
							params_i[params_i_count++] = int_val;
						else
							params_a[params_a_count++] = int_val;							
					} else
					{
						if (params_f_count < 8)
							params_f[params_f_count++] = float_val;
						else {
							void *float_ptr = &float_val;
							params_a[params_a_count++] = *cast<uint32_t *>(float_ptr);
						}
					}
				}
			}

			#define PARAM_TYPES \
				uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, \
				float,    float,    float,    float,    float,    float,    float,    float,    \
				uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, \
				uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, \
				uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t

			#define PARAM_VALS \
				params_i[0],  params_i[1],  params_i[2],  params_i[3],  params_i[4],  params_i[5],  params_i[6],  params_i[7],  \
				params_f[0],  params_f[1],  params_f[2],  params_f[3],  params_f[4],  params_f[5],  params_f[6],  params_f[7],  \
				params_a[0],  params_a[1],  params_a[2],  params_a[3],  params_a[4],  params_a[5],  params_a[6],  params_a[7],  \
				params_a[8],  params_a[9],  params_a[10], params_a[11], params_a[12], params_a[13], params_a[14], params_a[15], \
				params_a[16], params_a[17],	params_a[18], params_a[19], params_a[20], params_a[21], params_a[22], params_a[23]

			if (!is_res_float)
			{
				typedef uint32_t (*func_i_t)(PARAM_TYPES);
				uint32_t res = cast<func_i_t>(addr)(PARAM_VALS);
				if (res_ptr)
					*cast<uint32_t *>(res_ptr) = res;
			} else
			{
				typedef float (*func_f_t)(PARAM_TYPES);
				float res = cast<func_f_t>(addr)(PARAM_VALS);
				if (res_ptr)
					*cast<float *>(res_ptr) = res;
			}

			#undef PARAM_VALS
			#undef PARAM_TYPES

			return true;
		}
		case OP_GET_TOUCH_POINT_STATE:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 2);
			*v = 0;
			return true;
		}
		case OP_GET_TOUCH_SLIDE_STATE:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 4);
			*v = 0;
			return true;
		}
		case OP_GET_MENU_BUTTON_STATE:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*v = (touch::menu_button_pressed()) ? 1 : 0;
			return true;
		}
		case OP_GET_MENU_BUTTON_PRESSED:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t mintime = ScriptParams[0];
			*v = (touch::menu_button_pressed_timed(mintime)) ? 1 : 0;
			return true;
		}
		case OP_GET_PSP_CONTROL_STATE:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t index = ScriptParams[0];
			*v = (touch::psp_control_pressed((ePspControl)index)) ? 1 : 0;
			return true;
		}
		case OP_GET_PSP_CONTROL_PRESSED:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 2);
			uint32_t index = ScriptParams[0];
			uint32_t mintime = ScriptParams[1];
			*v = (touch::psp_control_pressed_timed((ePspControl)index, mintime)) ? 1 : 0;
			return true;
		}
		case OP_INVOKABLE_SCRIPT_STATS:
		{
			uint32_t inv_count = 0;
			if (scripts.size())
				for (int32_t i = 0; i < scripts.size(); i++)
					if (scripts[i]->invokable_id != -1)
						inv_count++;
			uint32_t *p_count = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			uint32_t *p_page_count = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*p_count = inv_count;
			*p_page_count = inv_count / 12 + 1;
			return true;
		}
		case OP_START_INVOKABLE_SCRIPT:
		{
			uint32_t *v = cast<uint32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t id = ScriptParams[0];
			if (scripts.size())
			{
				for (int32_t i = 0; i < scripts.size(); i++)
				{
					t_script *script = scripts[i];
					if (script->invokable_id == id)
					{
						// if script isn't started before
						if (script->handle == NULL)
						{
							utils::log("starting invokable script '%s'", script->name.c_str());
							script->handle = CTheScripts__StartNewScript(script->offset);
							*v = 0;
							return true;
						}
						// if script started before and already finished
						uint32_t wait_time = getfield<uint32_t>(script->handle, select(0x7C, 0x7C, 0xEC, 0x210, 0x200));
						if (wait_time == 0xFFFFFFFF)
						{
							utils::log("restarting invokable script '%s'", script->name.c_str());
							// set start ip
							uint32_t *p_ip = cast<uint32_t *>(script->handle + select(0x10, 0x10, 0x14, 0x18, 0x10));
							*p_ip = script->offset;
							// remove wait
							setfield<uint32_t>(script->handle, select(0x7C, 0x7C, 0xEC, 0x210, 0x200), 0);
							*v = 1;
							return true;
						}
						// script is running
						utils::log("invokable script '%s' already running", script->name.c_str());
						*v = 2;
						return true;
					}
				}
			}
			utils::log("invokable script %d not found", id);
			*v = -1;
			return true;
		}
		case OP_SHOW_MENU_OPEN_ARROW:
		{
			//ui::show_arrow();
			return true;
		}
		case OP_HIDE_MENU_OPEN_ARROW:
		{
			//ui::hide_arrow();
			return true;
		}
		case OP_CREATE_MENU:
		{
			CRunningScript__CollectParameters(script.handle, 2);
			uint32_t addr = ScriptParams[0];
			uint32_t item_count = ScriptParams[1];
			// read flags
			uint8_t flags = *cast<uint8_t *>(addr);
			bool bItemsUseGxt = flags & 1;
			bool bGxtTrimToFit = (flags & 2) != 0;
			uint32_t maxItemStrSize = bGxtTrimToFit ? select(20, 20, 20, 18, 19) : 0;
			addr += 4;
			// read title
			std::string title;
			while (*cast<char *>(addr))
			{
				title += *cast<char *>(addr);
				addr++;
			}
			addr++;
			title = psplang::localize(title);
			// read close button title
			std::string titleclose;
			while (*cast<char *>(addr))
			{
				titleclose += *cast<char *>(addr);
				addr++;
			}
			addr++;
			titleclose = psplang::localize(titleclose);
			// read items
			std::vector<wide_string> items;
			while (*cast<char *>(addr) && item_count)
			{
				// read item
				std::string a_item;
				wide_string w_item;				
				while (*cast<char *>(addr))
				{
					a_item += *cast<char *>(addr);
					w_item += *cast<char *>(addr);
					addr++;
				}
				addr++;
				bool bGxtEntryExists = false;
				if (bItemsUseGxt)
				{
					if (uint16_t *wstr = GetGxtEntry(a_item.c_str(), true))
					{
						std::string str = strutils::ansi_from_wstr(wstr);
						if (str.size())
						{
							if (game == GTA3)
							{
								if (!strstr(str.c_str(), "missing"))
								{
									bGxtEntryExists = true;
									w_item = wstr;									
								}
							} else
							{
								bGxtEntryExists = true;
								w_item = wstr;
							}
						}
					}
				}
				if (!bGxtEntryExists)
					w_item = strutils::wstr_from_ansi(psplang::localize(a_item).c_str());	
				if (bGxtEntryExists && maxItemStrSize && w_item.size() > maxItemStrSize)
					w_item.erase(maxItemStrSize);
				items.push_back(w_item);
				item_count--;
			}
			// create menu
			ui::create_menu(title, titleclose, items);
			return true;
		}
		case OP_DELETE_MENU:
		{
			ui::delete_menu();
			return true;
		}
		case OP_GET_MENU_TOUCHED_ITEM_INDEX:
		{
			int32_t *v = cast<int32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			CRunningScript__CollectParameters(script.handle, 1);
			uint32_t maxtime = ScriptParams[0];	// touch valid time
			*v = ui::get_menu_touched_item_index(maxtime);
			return true;
		}
		case OP_SET_MENU_ACTIVE_ITEM_INDEX:
		{
			CRunningScript__CollectParameters(script.handle, 1);
			int32_t page_index = ScriptParams[0];
			ui::set_menu_active_item_index(page_index);
			return true;
		}
		case OP_GET_MENU_ACTIVE_ITEM_INDEX:
		{
			int32_t *v = cast<int32_t *>(CRunningScript__GetPointerToScriptVariable(script.handle, 0));
			*v = ui::get_menu_active_item_index();
			return true;
		}
		default:
			return false;
		}
	}
}


