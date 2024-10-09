#include "ui.h"
#include "armhook.h"
#include "utils.h"
#include "pattern.h"
#include "touch.h"
#include "strutils.h"
#include "memutils.h"
#include "psplang.h"

namespace ui
{
	typedef void (*fn_Draw_Poly)(float, float, float, float, float, float, float, float, uint8_t *);
	fn_Draw_Poly draw_poly;

	enum eAlign
	{
		eAlignCenter,
		eAlignLeft,
		eAlignRight
	};

	enum eStyle
	{
		eStyleSimple,
		eStyleClassic,
		eStyleSpecial
	};

	typedef void (*fn_Print_String)(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style);
	fn_Print_String print_string;

	void on_draw();

	uint8_t menu_arrow_color[4];
	uint8_t menu_arrow_selected_alpha;
	uint8_t menu_active_item_font_color[4];
	uint8_t menu_selected_item_font_color[4];

	namespace lcs
	{
		// @CSprite2d::Draw2DPolygon
		typedef void (*fn_CSprite2d__Draw2DPolygon)(float, float, float, float, float, float, float, float, uint8_t *);
		fn_CSprite2d__Draw2DPolygon CSprite2d__Draw2DPolygon;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)();
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw()
		{
			CHud__Draw_();
			on_draw();
		}

		// @CFont::Details
		#pragma pack(push, 1)
		struct _CFont__Details
		{
			uint32_t	m_dwColor;				// 0x00
			float		m_fScaleX;	 			// 0x04
			float		m_fScaleY;	 			// 0x08
			float		m_fSlant;	 			// 0x0C
			float		m_fSlantRefPointX;		// 0x10
			float		m_fSlantRefPointY;		// 0x14
			bool		m_bLeftJustify;			// 0x18
			bool		m_bCentre;				// 0x19
			bool		m_bRightJustify;		// 0x1A
			bool		m_bBackground;			// 0x1B
			bool		m_bBackGroundOnlyText;	// 0x1C
			bool		m_bProp;				// 0x1D
			bool		m_bUnk0;				// 0x1E
			bool		m_bFlash;				// 0x1F
			uint32_t	m_dwUnk1;				// 0x20
			float		m_fAlphaFade; 			// 0x24
			uint32_t	m_dwBackgroundColor;	// 0x28
			float		m_fWrapx;	 			// 0x2C
			float		m_fCentreSize; 			// 0x30
			float		m_fRightJustifyWrap;	// 0x34
			uint16_t	m_wFontStyle;			// 0x38
			bool		m_bFontStyle_unk;		// 0x3A
			bool		m_bUnk2;				// 0x3B
			uint32_t	m_dwUnk3;				// 0x3C
			uint16_t	m_wDropShadowPosition;	// 0x40
			uint16_t	m_wUnkPad;				// 0x42
			uint32_t	m_dwDropColor;			// 0x44
			uint32_t	m_dwUnk4[4];			// 0x48
			uint32_t	m_dwOutlineColor;		// 0x58
			uint32_t	m_bOutlineOn;			// 0x5C
			uint32_t	m_dwNewLineAdd;			// 0x60
		} *CFont__Details;
		#pragma pack(pop)
		STRUCT_SIZE(_CFont__Details, 0x64);

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint16_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(float, float, uint16_t *, float *);
		fn_CFont__PrintString CFont__PrintString;

		float trans_x(float x) { return x * 480.0; }
		float trans_y(float y) { return y * 272.0; }

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			//CSprite2d__Draw2DPolygon(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
			//						 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty), rgba);

			int a0 = (int)rgba;
			float f12 = trans_x(topleftx);
			float f13 = trans_y(toplefty);
			float f14 = trans_x(toprightx);
			float f15 = trans_y(toprighty);
			float f16 = trans_x(bottomleftx);
			float f17 = trans_y(bottomlefty);
			float f18 = trans_x(bottomrightx);
			float f19 = trans_y(bottomrighty);

			asm volatile ("lw $v0,  %[x]" ::[x] "m" (a0));
			asm volatile ("move $a0, $v0");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f12));
			asm volatile ("mtc1 $v0, $f12");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f13));
			asm volatile ("mtc1 $v0, $f13");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f14));
			asm volatile ("mtc1 $v0, $f14");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f15));
			asm volatile ("mtc1 $v0, $f15");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f16));
			asm volatile ("mtc1 $v0, $f16");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f17));
			asm volatile ("mtc1 $v0, $f17");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f18));
			asm volatile ("mtc1 $v0, $f18");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f19));
			asm volatile ("mtc1 $v0, $f19");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (CSprite2d__Draw2DPolygon));
			asm volatile ("jalr $v0");
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{
			// save current font settings
			_CFont__Details	*fd = CFont__Details;
			_CFont__Details fdbcp = *fd;

			// set font align
			fd->m_bCentre = false;
			fd->m_bLeftJustify = false;
			fd->m_bRightJustify = false;
			switch (align)
			{
			case eAlignCenter:
				fd->m_bCentre = true;
				break;
			case eAlignLeft:
				fd->m_bLeftJustify = true;
				break;
			case eAlignRight:
				fd->m_bRightJustify = true;
				break;
			}
			// style
			switch (style)
			{
			case eStyleClassic:
				CFont__SetFontStyle(2);
				scalex *= 0.8f;
				scaley *= 0.72f;
				break;
			case eStyleSimple:
			case eStyleSpecial:
				CFont__SetFontStyle(2);		
				scalex *= 0.75f;
				scaley *= 0.65f;
				break;
			}

			// scale
			fd->m_fScaleX = scalex;
			fd->m_fScaleY = scaley;
			// color
			fd->m_dwColor = *cast<uint32_t*>(rgba);
			// common
			fd->m_bBackground = false;
			fd->m_bFlash = false;
			fd->m_bProp = true;
			fd->m_wDropShadowPosition = 1;
			fd->m_dwDropColor = 0xFF000000;

			if (psplang::is_lcs_rus1())
			{
				CFont__SetFontStyle(1);
				scalex *= 0.92; 
				scaley *= 0.83;
				if (style == eStyleClassic)
					y -= 0.01;
				fd->m_fScaleX = scalex;
				fd->m_fScaleY = scaley;
				fd->m_wDropShadowPosition = 0;
				fd->m_dwDropColor = 0xFF000000;
			}

			// print string
			//CFont__PrintString(trans_x(x), trans_y(y), str, NULL);

			int a0 = (int)str;
			int a1 = (int)NULL;
			float f12 = trans_x(x);
			float f13 = trans_y(y);

			asm volatile ("lw $v0,  %[x]" ::[x] "m" (a0));
			asm volatile ("move $a0, $v0");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (a1));
			asm volatile ("move $a1, $v0");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f12));
			asm volatile ("mtc1 $v0, $f12");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f13));
			asm volatile ("mtc1 $v0, $f13");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (CFont__PrintString));
			asm volatile ("jalr $v0");

			// restore font settings
			*fd = fdbcp;
		}

		void init()
		{
			uint32_t addr;

			#define PATTERN_NOT_FOUND {	utils::log("required ui pattern %d not found!", __LINE__); exit(1); }
			#define FIND_PATTERN(...) if (!__FindPatternAddress(addr, __VA_ARGS__)) PATTERN_NOT_FOUND;			

			#define READ_ADDR(addr1, addr2) ((static_cast<uint32_t>(*cast<uint16_t *>(addr1)) << 16) | *cast<uint16_t *>(addr2))
			#define READ_ADDR_INDIRECT(addr1, addr2) (READ_ADDR(addr1, addr2) - 0x10000)

			// @CSprite2d::Draw2DPolygon
			FIND_PATTERN("82 94 01 46 C2 9C 00 46");
			CSprite2d__Draw2DPolygon = cast<fn_CSprite2d__Draw2DPolygon>(addr - 16);
			utils::log("CSprite2d__Draw2DPolygon: 0x%08X", CSprite2d__Draw2DPolygon);

			// @CHud::Draw
			FIND_PATTERN("01 00 06 24 D8 04 B1 FF");
			_CHud__Draw = cast<fn_CHud__Draw>(addr - 12);
			armhook::hook_mips_func(_CHud__Draw, 8, CHud__Draw, &CHud__Draw_);
			utils::log("_CHud__Draw: 0x%08X", _CHud__Draw);

			// @CFont::Details
			FIND_PATTERN("00 00 A5 93 01 00 A2 93");
			CFont__Details = cast<_CFont__Details*>(READ_ADDR_INDIRECT(addr - 24, addr - 16));
			utils::log("CFont__Details: 0x%08X", CFont__Details);

			// @CFont::SetFontStyle
			FIND_PATTERN("00 24 04 00 02 00 02 24");
			CFont__SetFontStyle = cast<fn_CFont__SetFontStyle>(addr);

			// @CFont::PrintString
			FIND_PATTERN("50 00 B6 FF 2D 88 80 00");
			CFont__PrintString = cast<fn_CFont__PrintString>(addr - 8);

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = {40, 235, 24, 255};
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t color_a[4] = {225, 216, 161, 255};
			memcpy(menu_active_item_font_color, color_a, sizeof(color_a));
			uint8_t arrow_color[4] = { 55, 127, 175, 110 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 220;
		}
	}

	namespace vcs
	{
		// @CSprite2d::Draw2DPolygon
		typedef void (*fn_CSprite2d__Draw2DPolygon)(float, float, float, float, float, float, float, float, uint8_t *);
		fn_CSprite2d__Draw2DPolygon CSprite2d__Draw2DPolygon;

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint32_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::Unknown_1
		typedef void (*fn_CFont__SetFontPreset1)();
		fn_CFont__SetFontPreset1 CFont__SetFontPreset1;

		// @CFont::SetUnkFloat
		typedef void (*fn_CFont__SetFontScale)(float);
		fn_CFont__SetFontScale CFont__SetFontScale;

		// @CFont::SetFontAlign
		enum eFontAlign
		{
			faNone = 0,
			faLeft = 1,
			faCenter = 2,
			faRight = 4
		};
		typedef void (*fn_CFont__SetFontAlign)(eFontAlign);
		fn_CFont__SetFontAlign CFont__SetFontAlign;

		// @CFont::SetColor
		typedef void (*fn_CFont__SetColor)(uint8_t *rgba);
		fn_CFont__SetColor CFont__SetColor;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(uint16_t *, int, int);
		fn_CFont__PrintString CFont__PrintString;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)(void *);
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw(void *thiz)
		{
			CHud__Draw_(thiz);
			on_draw();
		}

		float trans_x(float x) { return x * 480.0; }
		float trans_y(float y) { return y * 272.0; }

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			//CSprite2d__Draw2DPolygon(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
			//						 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty), rgba);

			int a0 = (int)rgba;
			float f12 = trans_x(topleftx);
			float f13 = trans_y(toplefty);
			float f14 = trans_x(toprightx);
			float f15 = trans_y(toprighty);
			float f16 = trans_x(bottomleftx);
			float f17 = trans_y(bottomlefty);
			float f18 = trans_x(bottomrightx);
			float f19 = trans_y(bottomrighty);

			asm volatile ("lw $v0,  %[x]" ::[x] "m" (a0));
			asm volatile ("move $a0, $v0");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f12));
			asm volatile ("mtc1 $v0, $f12");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f13));
			asm volatile ("mtc1 $v0, $f13");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f14));
			asm volatile ("mtc1 $v0, $f14");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f15));
			asm volatile ("mtc1 $v0, $f15");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f16));
			asm volatile ("mtc1 $v0, $f16");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f17));
			asm volatile ("mtc1 $v0, $f17");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f18));
			asm volatile ("mtc1 $v0, $f18");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (f19));
			asm volatile ("mtc1 $v0, $f19");
			asm volatile ("lw $v0,  %[x]" ::[x] "m" (CSprite2d__Draw2DPolygon));
			asm volatile ("jalr $v0");
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{

			eFontAlign font_align = faNone;
			switch (align)
			{
			case eAlignCenter:
				font_align = faCenter;
				break;
			case eAlignLeft:
				font_align = faLeft;
				break;
			case eAlignRight:
				font_align = faRight;
				break;
			}

			int font_style = 0;
			switch (style)
			{
			case eStyleClassic:
				font_style = 2;
				scaley *= 0.72f * 0.4f;
				break;
			case eStyleSimple:				
				if (scalex == 0.4f && scaley == 0.8f) // page index
				{
					font_style = 2;
					font_align = faNone;
					x -= 0.02;
					scaley *= 0.67f * 0.4f;
				} else
				{
					font_style = 2;
					scaley *= 0.67f * 0.3f;
				}				
				break;
			case eStyleSpecial:
				font_style = 0;
				break;
			}

			if (psplang::is_vcs_rus1())
			{
				font_style = 1;
				if (style == eStyleClassic)	scaley *= 1.8; else
				if (style == eStyleSimple)	scaley *= 1.6;
			}

			CFont__SetFontStyle(font_style);
			CFont__SetFontPreset1();
			CFont__SetFontScale(scaley);			
			CFont__SetFontAlign(font_align);
			CFont__SetColor(rgba);

			CFont__PrintString(str, (int)trans_x(x), (int)trans_y(y));
		}

		void init()
		{
			uint32_t addr;

			// @CSprite2d::Draw2DPolygon
			FIND_PATTERN("49 00 03 3C 49 00 02 3C ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 B0 FF C2 9C 00 46");
			CSprite2d__Draw2DPolygon = cast<fn_CSprite2d__Draw2DPolygon>(addr - 0);
			utils::log("CSprite2d__Draw2DPolygon: 0x%08X", CSprite2d__Draw2DPolygon);

			// @CHud::Draw
			FIND_PATTERN("28 00 B1 FF 2D 80 80 00 30 00 B2 FF 38 00 B3 FF 40 00 B4 FF 48 00 B5 FF"); // reads gp related addr
			_CHud__Draw = cast<fn_CHud__Draw>(addr - 8);
			armhook::hook_mips_func(_CHud__Draw, 8, CHud__Draw, &CHud__Draw_);

			// @CFont::PrintString
			FIND_PATTERN("18 00 B3 FF 2D 90 80 00 20 00 B4 FF 2D 98 A0 00 00 00 B0 FF");
			CFont__PrintString = cast<fn_CFont__PrintString>(addr - 8);

			#define READ_REL3_ADDR(offset) memutils::mem_read_mips_jmp(cast<ptr>(addr) + offset)

			FIND_PATTERN("2D 80 A0 00 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 02 00 04 24");

			// @CFont::SetFontStyle
			CFont__SetFontStyle = cast<fn_CFont__SetFontStyle>(READ_REL3_ADDR(12));

			// @CFont::SetFontPreset1
			CFont__SetFontPreset1 = cast<fn_CFont__SetFontPreset1>(READ_REL3_ADDR(20));

			// @CFont::SetFontAlign
			CFont__SetFontAlign = cast<fn_CFont__SetFontAlign>(READ_REL3_ADDR(28));

			FIND_PATTERN("02 00 04 24 0C 2C A8 92");

			// @CFont::SetColor
			CFont__SetColor = cast<fn_CFont__SetColor>(READ_REL3_ADDR(28));

			FIND_PATTERN("01 00 04 24 ?? ?? ?? ?? 00 00 00 00 19 3F 01 3C 9A 99 21 34 00 60 81 44 ?? ?? ?? ?? 00 00 00 00 50 00 A2 8E");

			// @CFont::SetFontScale
			CFont__SetFontScale = cast<fn_CFont__SetFontScale>(READ_REL3_ADDR(24));

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = { 40, 235, 24, 255 };
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t color_a[4] = { 225, 216, 161, 255 };
			memcpy(menu_active_item_font_color, color_a, sizeof(color_a));
			uint8_t arrow_color[4] = { 55, 127, 175, 110 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 220;
		}
	}

	// arrow

	static uint16_t unistr[512];
	uint16_t *ansi_to_unicode(LPCSTR str, bool localize = false)
	{		
		std::string localized;
		if (localize)
		{
			localized = psplang::localize(str);
			str = localized.c_str();
		}
		strutils::wstr_from_ansi(unistr, str);
		return unistr;
	}

	// draws arrow, length is 0..1
	void draw_arrow(float length)
	{
		uint8_t arrow_color[4] = { 255, 10, 10, 190 };
		float width = 0.08, top = 0.15, left = 0.5 - width / 2, h = length / 1.75;
		draw_poly(left, top, left + width, top, left, top + h, left + width, top + h, arrow_color);
		float aextent = 0.027, awidth = 0.12;
		draw_poly(left - aextent, top + h, left + width + aextent, top + h, 0.5, top + h + awidth, 0.5, top + h + awidth, arrow_color);
		uint8_t font_color[4] = { 252, 250, 250, 255 };

		print_string(ansi_to_unicode("OPEN CLEO MENU", true), 0.5, top - 0.05, eAlignCenter, 0.85, 2.1, font_color, eStyleClassic);
		print_string(ansi_to_unicode("PSP: PRESS START", true), 0.5, top + 0.1, eAlignCenter, 0.85, 2.1, font_color, eStyleClassic);
		print_string(ansi_to_unicode("TO OPEN GAME MENU HOLD START", true), 0.5, top + 0.19, eAlignCenter, 0.5, 1.3, font_color, eStyleClassic);
	}

	uint32_t arrow_draw_until_time;		// time in ticks til arrow must be drawn
	uint32_t arrow_frames;				// indicates arrow current draw stage

	// shows arrow, called from scripts
	void show_arrow()
	{
		static bool bShowArrow = true;	// arrow shows only at start
		if (bShowArrow)
		{
			bShowArrow = false;
			const uint32_t arrow_draw_time = 17000;
			arrow_draw_until_time = utils::get_tick_count() + arrow_draw_time;
			arrow_frames = 0;
		}
	}

	// hides arrow, called from scripts
	void hide_arrow()
	{
		arrow_draw_until_time = 0;
		arrow_frames = 0;
	}

	// menu

	std::string menu_name;								// title
	std::string menu_close_name;						// close button title
	std::vector<wide_string> menu_items;				// item names
	int32_t menu_active_page = -1;						// active page index, -1 when menu is not active
	int32_t menu_active_item = -1;						// active item index in menu_items
	int32_t menu_page_count;							// total page count, set on create
	int32_t menu_selected_item;							// selected item index in menu_items, updated on touch handle
	uint32_t menu_selected_item_time;					// time in ticks when menu_selected_item was selected
	const int32_t menu_items_per_page = 8;				// max items per page
	int32_t menu_arrow_left_highlight_frames_left;		// frames to draw left arrow highlighted
	int32_t menu_arrow_right_highlight_frames_left;		// frames to draw right arrow highlighted
	int32_t menu_item_highlight_frames_left;			// frames to draw selected item highlighted
	const int32_t menu_highlight_frames = 3;			// frame count for highlight to be active
	// updated on draw
	float menu_close_points[4];							// close button rect coords					[x1,y1,x2,y2]
	float menu_arrow_left_points[8];					// left arrow coords for checking touches 	[x1,y1,x2,y2] * 4
	float menu_arrow_right_points[8];					// right arrow coords for checking touches 	[x1,y1,x2,y2] * 4
	float menu_item_points[menu_items_per_page * 4];	// item coords for checking touches 		[x1,y1,x2,y2] * menu_items_per_page
	uint32_t menu_item_points_current;					// current page item count
	uint32_t hud_draw_time;

	// creates menu, called from scripts
	void create_menu(std::string name, std::string closename, std::vector<wide_string> &items)
	{
		menu_name = name;
		menu_close_name = closename;
		menu_items = items;
		menu_active_page = 0;
		menu_active_item = 0;
		menu_selected_item = -1;
		menu_selected_item_time = 0;
		menu_arrow_left_highlight_frames_left = 0;
		menu_arrow_right_highlight_frames_left = 0;
		menu_item_highlight_frames_left = 0;
		menu_page_count = menu_items.size() / menu_items_per_page;
		if (menu_items.size() % menu_items_per_page)
			menu_page_count++;
	}

	// get menu touched item index
	// resets touch state
	// returns -1 if no menu touch occured, -2 for close button, 0 and above for item indexes
	// maxtime - touch valid time
	int32_t get_menu_touched_item_index(uint32_t maxtime)
	{
		if (utils::get_tick_count() < menu_selected_item_time + maxtime)
		{
			// utils::log("menu_selected_item %d", menu_selected_item);
			menu_selected_item_time = 0; // reset touch state
			return menu_selected_item;
		}
		return -1;
	}

	// deletes menu, called from scripts or on restart
	void delete_menu()
	{
		menu_active_page = -1;
		menu_active_item = -1;
		menu_selected_item = -1;
		menu_selected_item_time = 0;
		menu_page_count = 0;
	}

	void set_menu_active_item_index(int32_t index)
	{
		if (index >= 0 && index < menu_items.size())
		{
			menu_active_item = index;
			menu_active_page = menu_active_item / menu_items_per_page;
		}
	}

	int32_t get_menu_active_item_index()
	{
		return menu_active_item;
	}

	void handle_psp_controls()
	{
		if (menu_active_page == -1 || menu_active_item == -1 || hud_draw_time + 150 < utils::get_tick_count()) return;

		bool up = menu_page_count > 0 ? touch::psp_control_pressed(CTRL_UP) || touch::psp_control_pressed(CTRL_STICK_UP) : false;
		bool down = menu_page_count > 0 ? touch::psp_control_pressed(CTRL_DOWN) || touch::psp_control_pressed(CTRL_STICK_DOWN) : false;
		bool cross = menu_page_count > 0 ? touch::psp_control_pressed(CTRL_CROSS) : false;
		bool right = menu_page_count > 1 ? touch::psp_control_pressed(CTRL_RIGHT) || touch::psp_control_pressed(CTRL_STICK_RIGHT) : false;
		bool left = menu_page_count > 1 ? touch::psp_control_pressed(CTRL_LEFT) || touch::psp_control_pressed(CTRL_STICK_LEFT) : false;
		bool circle = touch::psp_control_pressed(CTRL_CIRCLE) || touch::menu_button_pressed_timed(1900);

		int32_t start_item = menu_active_page * menu_items_per_page;
		int32_t menu_items_this_page = (start_item + menu_items_per_page > menu_items.size()) ? menu_items.size() - start_item : menu_items_per_page;

		if (up)
		{
			touch::psp_control_disable(CTRL_UP, 150);
			touch::psp_control_disable(CTRL_STICK_UP, 150);
			menu_active_item--;
			if (menu_active_item < start_item)
				menu_active_item = start_item + menu_items_this_page - 1;
		} else
		if (down)
		{
			touch::psp_control_disable(CTRL_DOWN, 150);
			touch::psp_control_disable(CTRL_STICK_DOWN, 150);
			menu_active_item++;
			if (menu_active_item == start_item + menu_items_this_page)
				menu_active_item = start_item;
		} else
		if (right || left)
		{
			if (right)
			{
				touch::psp_control_disable(CTRL_RIGHT, 200);
				touch::psp_control_disable(CTRL_STICK_RIGHT, 200);
				menu_arrow_right_highlight_frames_left = menu_highlight_frames;
				menu_active_page++;
				if (menu_active_page == menu_page_count)
					menu_active_page = 0;	
			} else
			{
				touch::psp_control_disable(CTRL_LEFT, 200);
				touch::psp_control_disable(CTRL_STICK_LEFT, 200);
				menu_arrow_left_highlight_frames_left = menu_highlight_frames;
				menu_active_page--;
				if (menu_active_page == -1)
					menu_active_page = menu_page_count - 1;
			}
			int32_t current_page_item = menu_active_item % menu_items_per_page;
			start_item = menu_active_page * menu_items_per_page;
			menu_active_item = start_item + current_page_item;
			if (menu_active_item >= menu_items.size())
				menu_active_item = menu_items.size() - 1;
		} else
		if (cross)
		{
			touch::psp_control_disable(CTRL_CROSS, 200);
			menu_selected_item = menu_active_item;
			menu_selected_item_time = utils::get_tick_count();
			menu_item_highlight_frames_left = menu_highlight_frames;
		} else
		if (circle)
		{
			touch::psp_control_disable(CTRL_CIRCLE, 200);
			menu_selected_item = -2;
			menu_selected_item_time = utils::get_tick_count();
		}
	}

	// menu must be active in order this to be called
	void draw_menu()
	{
		// draw rect
		float width = 0.3, height = 0.8, top = 0.12, left = 0.5 - width / 2;
		uint8_t rect_color[4] = { 55, 127, 175, 150 };
		draw_poly(left, top, left + width, top, left, top + height, left + width, top + height, rect_color);
		// draw title
		uint8_t font_color[4] = { 252, 250, 250, 255 };
		print_string(ansi_to_unicode(menu_name.c_str()), 0.5, top - 0.03, eAlignCenter, 0.8, 1.8, font_color, eStyleClassic);

		// draw page num if needed
		if (menu_page_count > 1)
		{
			char str[32];
			sprintf(str, "%d of %d", menu_active_page + 1, menu_page_count);
			print_string(ansi_to_unicode(str), 0.61, top + 0.04, eAlignCenter, 0.4f, 0.8f, font_color, eStyleSimple);
		}
		// draw items
		if (menu_active_page < 0 || menu_active_page > menu_page_count - 1)
			menu_active_page = 0;
		int32_t start_item = menu_active_page * menu_items_per_page;
		int32_t menu_items_this_page = (start_item + menu_items_per_page > menu_items.size()) ? menu_items.size() - start_item : menu_items_per_page;
		top = 0.2;
		menu_item_points_current = menu_items_this_page;
		for (int32_t i = 0; i < menu_items_this_page; i++)
		{
			uint8_t font_color[4] = { 252, 250, 250, 255 };
			if (menu_selected_item == start_item + i && menu_item_highlight_frames_left)
			{
				memcpy(font_color, menu_selected_item_font_color, sizeof(font_color));
				menu_item_highlight_frames_left--;
			}

			if (menu_active_item == start_item + i && !menu_item_highlight_frames_left) // menu_selected_item != menu_active_item
				memcpy(font_color, menu_active_item_font_color, sizeof(font_color));

			print_string((uint16_t *)menu_items[start_item + i].c_str(), 0.5, top, eAlignCenter, 0.5, 1.5, font_color, eStyleSimple);
			top += 0.01;
			menu_item_points[i * 4] = 0.5 - width / 2.5;
			menu_item_points[i * 4 + 1] = top;
			top += 0.07;
			menu_item_points[i * 4 + 2] = 0.5 + width / 2.5;
			menu_item_points[i * 4 + 3] = top;
			top += 0.01;
		}
		// draw arrows if needed
		if (menu_page_count > 1)
		{
			// left arrow
			float awidth = 0.08, aheight = 0.2, aextent = 0.015, right = left + width;
			uint8_t arrow_left_color[4];
			memcpy(arrow_left_color, menu_arrow_color, sizeof(menu_arrow_color));
			if (menu_arrow_left_highlight_frames_left)
			{
				arrow_left_color[3] = menu_arrow_selected_alpha;
				menu_arrow_left_highlight_frames_left--;
			}
			menu_arrow_left_points[0] = left - awidth - aextent;
			menu_arrow_left_points[1] = 0.5;
			menu_arrow_left_points[2] = left - aextent;
			menu_arrow_left_points[3] = 0.5 - aheight / 2;
			menu_arrow_left_points[4] = left - aextent;
			menu_arrow_left_points[5] = 0.5 + aheight / 2;
			menu_arrow_left_points[6] = left - awidth - aextent;
			menu_arrow_left_points[7] = 0.5;
			draw_poly(menu_arrow_left_points[0], menu_arrow_left_points[1], menu_arrow_left_points[2], menu_arrow_left_points[3],
					  menu_arrow_left_points[6], menu_arrow_left_points[7],
					  menu_arrow_left_points[4], menu_arrow_left_points[5],
					  arrow_left_color);
			// right arrow
			uint8_t arrow_right_color[4];
			memcpy(arrow_right_color, menu_arrow_color, sizeof(menu_arrow_color));
			if (menu_arrow_right_highlight_frames_left)
			{
				arrow_right_color[3] = menu_arrow_selected_alpha;
				menu_arrow_right_highlight_frames_left--;
			}
			menu_arrow_right_points[0] = right + awidth + aextent;
			menu_arrow_right_points[1] = 0.5;
			menu_arrow_right_points[2] = right + aextent;
			menu_arrow_right_points[3] = 0.5 - aheight / 2;
			menu_arrow_right_points[4] = right + aextent;
			menu_arrow_right_points[5] = 0.5 + aheight / 2;
			menu_arrow_right_points[6] = right + awidth + aextent;
			menu_arrow_right_points[7] = 0.5;
			draw_poly(menu_arrow_right_points[0], menu_arrow_right_points[1], menu_arrow_right_points[2], menu_arrow_right_points[3],
					  menu_arrow_right_points[6], menu_arrow_right_points[7],
					  menu_arrow_right_points[4], menu_arrow_right_points[5],
					  arrow_right_color);
		}
	}

	// common
	void on_draw()
	{
		// arrow
		uint32_t time = utils::get_tick_count();
		hud_draw_time = time;
		if (time < arrow_draw_until_time || arrow_frames > 0)
		{
			const int32_t arrow_frames_max = 40, arrow_freeze_frames_max = 20;

			draw_arrow(arrow_frames < arrow_frames_max ? (float)arrow_frames / (float)arrow_frames_max : 1.0);

			if (arrow_frames++ == arrow_frames_max + arrow_freeze_frames_max)
				arrow_frames = 0;
		}
		// menu
		if (menu_active_page != -1)
			draw_menu();
	}

	void init()
	{
		switch (core::GetGame())
		{
		case core::GTALCS:
			lcs::init();
			break;
		case core::GTAVCS:
			vcs::init();
			break;
		}
	}
}
