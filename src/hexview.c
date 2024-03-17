/*
 *   Hexview - Program for the PC-FX to view memory
 *
 *   Copyright (C) 2024 David Shadoff
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <eris/types.h>
#include <eris/std.h>
#include <eris/v810.h>
#include <eris/king.h>
#include <eris/low/7up.h>
#include <eris/tetsu.h>
#include <eris/romfont.h>
#include <eris/bkupmem.h>
#include <eris/timer.h>
#include <eris/pad.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define JOY_I            1
#define JOY_II           2
#define JOY_III          4
#define JOY_IV           8
#define JOY_V            16
#define JOY_VI           32
#define JOY_SELECT       64
#define JOY_RUN          128
#define JOY_UP           256
#define JOY_RIGHT        512
#define JOY_DOWN         1024
#define JOY_LEFT         2048
#define JOY_MODE1        4096
#define JOY_MODE2        16384


// Test positioning MACROS
//
#define LEFT_EDGE        1
#define TITLE_LINE       2
#define RANGE_LINE       4
#define HEXTITLE_LINE    8



// Palettes to use for text
//
#define PAL_TEXT      0
#define PAL_INVERSE   1
#define PAL_DIM       2
#define PAL_UNKNOWN   3
#define PAL_MULTITAP  4
#define PAL_JOYPAD    5
#define PAL_MOUSE     6


extern u8 read_addr(u32 address);

void printsjis(char *text, int x, int y);
void print_narrow(u32 sjis, u32 kram);
void print_wide(u32 sjis, u32 kram);

void print_at(int x, int y, int pal, char* str);
void putch_at(int x, int y, int pal, char c);
void putnumber_at(int x, int y, int pal, int digits, int value);

extern u8 font[];

// interrupt-handling variables
volatile int sda_frame_count = 0;
volatile int last_sda_frame_count = 0;

/* HuC6270-A's status register (RAM mapping). Used during VSYNC interrupt */
volatile uint16_t * const MEM_6270A_SR = (uint16_t *) 0x80000400;

int stepval = 0;

enum HexFormat {
	FMT_HEX,
	FMT_ASCII
};

char *hex_lookup = "0123456789ABCDEF";

uint8_t mem_buf[256];

char buffer[2048];


///////////////////////////////// Joypad routines
volatile u32 joypad;
volatile u32 joypad_last;
volatile u32 joytrg;


__attribute__ ((noinline)) void joyread(void)
{
u32 temp;

   joypad_last = joypad;

   temp = eris_pad_read(0);

   if ((temp >> 28) == PAD_TYPE_FXPAD) {  // PAD TYPE
      joytrg = (~joypad_last) & joypad;
   }
   else {
      joytrg = 0;
   }
   joypad = temp;
}




///////////////////////////////// Interrupt handler
__attribute__ ((interrupt_handler)) void my_vblank_irq (void)
{
   uint16_t vdc_status = *MEM_6270A_SR;

   if (vdc_status & 0x20) {
      sda_frame_count++;
   }
   joyread();
}

void vsync(int numframes)
{
   while (sda_frame_count < (last_sda_frame_count + numframes + 1));

   last_sda_frame_count = sda_frame_count;
}


///////////////////////////////// CODE

//
// for setting breakpoints - add a call to step() and breakpoint on it
// or watchpoint on stepval.
//
__attribute__ ((noinline)) void step(void)
{
   stepval++;;
}
//////////


void init(void)
{
	int i, j;
//	u32 str[256];
	u16 microprog[16];
	u16 a, img;

	eris_low_sup_init(0);
	eris_low_sup_init(1);
	eris_king_init();
	eris_tetsu_init();
	
	eris_tetsu_set_priorities(0, 0, 1, 0, 0, 0, 0);
	eris_tetsu_set_7up_palette(0, 0);
	eris_tetsu_set_king_palette(0, 0, 0, 0);
	eris_tetsu_set_rainbow_palette(0);

	eris_king_set_bg_prio(KING_BGPRIO_3, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, 0);
	eris_king_set_bg_mode(KING_BGMODE_4_PAL, 0, 0, 0);
	eris_king_set_kram_pages(0, 0, 0, 0);

	for(i = 0; i < 16; i++) {
		microprog[i] = KING_CODE_NOP;
	}

	microprog[0] = KING_CODE_BG0_CG_0;
	eris_king_disable_microprogram();
	eris_king_write_microprogram(microprog, 0, 16);
	eris_king_enable_microprogram();

	//eris_tetsu_set_palette(3, 0x602C);
	//eris_tetsu_set_palette(4, 0x5080);
	//eris_tetsu_set_palette(5, 0xC422);
	//eris_tetsu_set_palette(6, 0x9999);
	//eris_tetsu_set_palette(7, 0x1234);

	/* Font uses sub-palette #1 for FG, #2 for BG */
//	/* palette #0 is default - light green background, bright white foreground */
//	eris_tetsu_set_palette(0x00, 0x2A66);
//	eris_tetsu_set_palette(0x01, 0xFC88);
//	eris_tetsu_set_palette(0x02, 0x2A66);

	/* palette #0 is default - black background, white foreground */
	eris_tetsu_set_palette(0x00, 0x0088);
	eris_tetsu_set_palette(0x01, 0xE088);
	eris_tetsu_set_palette(0x02, 0x0088);

	/* palette #1 is white background, black foreground */
	eris_tetsu_set_palette(0x10, 0xE088);
	eris_tetsu_set_palette(0x11, 0x0088);
	eris_tetsu_set_palette(0x12, 0xE088);

	/* palette #2 is black background, dimmed white foreground */
	eris_tetsu_set_palette(0x20, 0x0088);
	eris_tetsu_set_palette(0x21, 0x9088);
	eris_tetsu_set_palette(0x22, 0x0088);

	/* palette #3 is black background, bright red foreground */
	eris_tetsu_set_palette(0x30, 0x0088);
	eris_tetsu_set_palette(0x31, 0x2A2F);
	eris_tetsu_set_palette(0x32, 0x0088);

	/* palette #4 is black background, bright yellow foreground */
	eris_tetsu_set_palette(0x40, 0x0088);
	eris_tetsu_set_palette(0x41, 0xDF09);
	eris_tetsu_set_palette(0x42, 0x0088);

	/* palette #5 is black background, turquoise foreground */
	eris_tetsu_set_palette(0x50, 0x0088);
	eris_tetsu_set_palette(0x51, 0x9BB1);
	eris_tetsu_set_palette(0x52, 0x0088);

	/* palette #6 is black background, green foreground */
	eris_tetsu_set_palette(0x60, 0x0088);
	eris_tetsu_set_palette(0x61, 0x8006);
	eris_tetsu_set_palette(0x62, 0x0088);


	eris_tetsu_set_video_mode(TETSU_LINES_262, 0, TETSU_DOTCLOCK_7MHz, TETSU_COLORS_16,
				TETSU_COLORS_16, 1, 0, 1, 0, 0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0SUB, 0, 0);
	eris_king_set_scroll(KING_BG0, 0, 0);
	eris_king_set_bg_size(KING_BG0, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256);
	eris_low_sup_set_control(0, 0, 1, 0);
	eris_low_sup_set_access_width(0, 0, SUP_LOW_MAP_64X32, 0, 0);
	eris_low_sup_set_scroll(0, 0, 0);
	//eris_low_sup_set_video_mode(0, 2, 2, 4, 0x1F, 0x11, 2, 239, 2); // 5MHz numbers
	eris_low_sup_set_video_mode(0, 3, 3, 6, 0x2B, 0x11, 2, 239, 2);

	eris_king_set_kram_read(0, 1);
	eris_king_set_kram_write(0, 1);
	// Clear BG0's RAM
	for(i = 0; i < 0x1E00; i++) {
		eris_king_kram_write(0);
	}
	eris_king_set_kram_write(0, 1);

	eris_low_sup_set_vram_write(0, 0);
	for(i = 0; i < 0x800; i++) {
		eris_low_sup_vram_write(0, 0x120); // 0x80 is space
	}


	eris_low_sup_set_vram_write(0, 0x1200);
	// load font into video memory
	for(i = 0; i < 0x60; i++) {
		// first 2 planes of color
		for (j = 0; j < 8; j++) {
			img = font[(i*8)+j] & 0xff;
			a = (~img << 8) | img;
			eris_low_sup_vram_write(0, a);
		}
		// last 2 planes of color
		for (j = 0; j < 8; j++) {
			eris_low_sup_vram_write(0, 0);
		}
	}

	eris_pad_init(0); // initialize joypad


        // Disable all interrupts before changing handlers.
        irq_set_mask(0x7F);

        // Replace firmware IRQ handlers for the Timer and HuC6270-A.
        //
        // This liberis function uses the V810's hardware IRQ numbering,
        // see FXGA_GA and FXGABOAD documents for more info ...
        irq_set_raw_handler(0xC, my_vblank_irq);

        // Enable Timer and HuC6270-A interrupts.
        //
        // d6=Timer
        // d5=External
        // d4=KeyPad
        // d3=HuC6270-A
        // d2=HuC6272
        // d1=HuC6270-B
        // d0=HuC6273
        irq_set_mask(0x77);

        // Allow all IRQs.
        //
        // This liberis function uses the V810's hardware IRQ numbering,
        // see FXGA_GA and FXGABOAD documents for more info ...
        irq_set_level(8);

        // Enable V810 CPU's interrupt handling.
        irq_enable();

        eris_low_sup_setreg(0, 5, 0x88);  // Set Hu6270 BG to show, and VSYNC Interrupt

	eris_bkupmem_set_access(1,1);
}


void read_array(u32 base_addr)
{
int i;

   for (i = 0; i < 256; i++) {
      mem_buf[i] = read_addr(base_addr + i);
   }
}

/* TODO */

void hex_format_line(enum HexFormat fmt_type, uint8_t *bin_buf, char *output_string)
{
int i;
int out_ind = 0;
int temp;

   for (i = 0; i < 16; i++)
   {
      if (fmt_type == FMT_HEX) {
         temp = ((*(bin_buf + i)) >> 4) & 0x0F;
         *(output_string + out_ind) = hex_lookup[temp];
         out_ind++;

         temp = (*(bin_buf + i)) & 0x0F;
         *(output_string + out_ind) = hex_lookup[temp];
         out_ind++;
      }
      else {
         *(output_string + out_ind) = ' ';
         out_ind++;

         temp = *(bin_buf + i);
	 if ((temp >= 0x20) && (temp <= 0x7F))
            *(output_string + out_ind) = (char)temp;
         else
            *(output_string + out_ind) = ' ';
         out_ind++;
      }
      
      if ((i == 15)) {
         *(output_string + out_ind) = 0x00;
      }
      else if ((i & 1) == 1) {
         *(output_string + out_ind) = ' ';
         out_ind++;
      }
   }
}

int main(int argc, char *argv[])
{
static u32 joy1 = 0;
static u32 prevjoy1;
u32 baseaddr;
u32 offset;
int i;

char buf[10];
char range_buf[24];
char hexdata[64];
char * pointer;
enum HexFormat FmtType = FMT_HEX;

   init();

   print_at(16, TITLE_LINE, PAL_TEXT, "FX Hexview");
   print_at(35, TITLE_LINE, PAL_TEXT, "v0.1");


   print_at(LEFT_EDGE+ 1, RANGE_LINE   , PAL_DIM, "Range (Max 00200000):");
   print_at(LEFT_EDGE+ 1, RANGE_LINE+1 , PAL_JOYPAD, "00XXXXX0 - 00XXXXXF");

   print_at(LEFT_EDGE, HEXTITLE_LINE   , PAL_UNKNOWN, "    0 1  2 3  4 5  6 7  8 9  A B  C D  E F");
   for (i = 0; i < 16; i++)
   {
      sprintf(buf, "%cx:", hex_lookup[i]);
      print_at(LEFT_EDGE, HEXTITLE_LINE+i+1, PAL_UNKNOWN, buf);
   }

   baseaddr = 0x8000;

   // start out ignoring current keypresses
   joy1 = joypad;
   prevjoy1 = joy1;

   while (1)
   {
      prevjoy1 = joy1;
      joy1 = joypad;

      FmtType = FMT_HEX;

      offset = 256;
      pointer = "   ^";

      if (((joy1 >> 28) & 0x0F) == PAD_TYPE_FXPAD)
      {

         if (joy1 & JOY_SELECT) {
            FmtType = FMT_ASCII;
	 }
         if (joy1 & JOY_VI) {
            offset = 4096;
            pointer = "  ^ ";
	 }
         if (joy1 & JOY_V) {
            offset = 65536;
            pointer = " ^  ";
	 }
         if (joy1 & JOY_IV) {
            offset = 1048576;
            pointer = "^   ";
	 }

         if (prevjoy1 != joy1) {
            if (((prevjoy1 & JOY_UP) == 0) && ((joy1 & JOY_UP) != 0)) {
               baseaddr = baseaddr + offset;
	    }

            if (((prevjoy1 & JOY_DOWN) == 0) && ((joy1 & JOY_DOWN) != 0)) {
               baseaddr = baseaddr - offset;
	    }
         }
      }

      sprintf(range_buf, "%8.8X - %8.8X", baseaddr, (baseaddr + 0xff));
      print_at(LEFT_EDGE+ 1, RANGE_LINE+1 , PAL_JOYPAD, range_buf);
      print_at(LEFT_EDGE+ 3, RANGE_LINE+2 , PAL_JOYPAD, pointer);

      read_array(baseaddr);

      for (i = 0; i < 16; i++)
      {
         hex_format_line( FmtType, &mem_buf[(i*16)], hexdata );

         print_at(LEFT_EDGE+3, HEXTITLE_LINE+i+1, PAL_TEXT, hexdata);
      }

//      vsync(0);
   }

   return 0;
}

// print with first 7up (HuC6270 #0)
//
void print_at(int x, int y, int pal, char* str)
{
	int i;
	u16 a;

	i = (y * 64) + x;

	eris_low_sup_set_vram_write(0, i);
	for (i = 0; i < strlen8(str); i++) {
		a = (pal * 0x1000) + str[i] + 0x100;
		eris_low_sup_vram_write(0, a);
	}
}

void putch_at(int x, int y, int pal, char c)
{
        int i;
        u16 a;

        i = (y * 64) + x;

        eris_low_sup_set_vram_write(0, i);

        a = (pal * 0x1000) + c + 0x100;
        eris_low_sup_vram_write(0, a);
}

void putnumber_at(int x, int y, int pal, int len, int value)
{
        int i;
        u16 a;
	char str[64];

        i = (y * 64) + x;

	if (len == 2) {
	   sprintf(str, "%2d", value);
	}
	else if (len == 4) {
	   sprintf(str, "%4d", value);
	}
	else if (len == 5) {
	   sprintf(str, "%5d", value);
	}

        eris_low_sup_set_vram_write(0, i);

	for (i = 0; i < strlen8(str); i++) {
                a = (pal * 0x1000) + str[i] + 0x100;
                eris_low_sup_vram_write(0, a);
        }
}

// functions related to printing with KING processor
//

void printsjis(char *text, int x, int y)
{

int offset;
u8 ch, ch2;
u32 sjis;
u32 kram;

   offset = 0;
   kram = x + (y <<5);

   ch = *(text+offset);

   while (ch != 0)
   {
      if ((ch < 0x81) || ((ch >= 0xA1) && (ch <= 0xDF)))
      {
         sjis = ch;
         print_narrow(sjis, kram);
         kram++;
      }
      else
      {
         offset++;
         ch2 = *(text+offset);
         sjis = (ch << 8) + ch2;
         print_wide(sjis, kram);
         kram += 2;
      }

      offset++;
      ch = *(text+offset);
   }

}

void print_narrow(u32 sjis, u32 kram)
{
        u16 px;
        int x, y;
        u8* glyph;

        glyph = eris_romfont_get(sjis, ROMFONT_ANK_8x16);

        for(y = 0; y < 16; y++) {
                eris_king_set_kram_write(kram + (y << 5), 1);
                px = 0;
                for(x = 0; x < 8; x++) {
                        if((glyph[y] >> x) & 1) {
                                px |= 1 << (x << 1);
                        }
                }
                eris_king_kram_write(px);
        }
}

void print_wide(u32 sjis, u32 kram)
{
        u16 px;
        int x, y;
        u16* glyph;

        glyph = (u16*) eris_romfont_get(sjis, ROMFONT_KANJI_16x16);

        for(y = 0; y < 16; y++) {
                eris_king_set_kram_write(kram + (y << 5), 1);
                px = 0;
                for(x = 0; x < 8; x++) {
                        if((glyph[y] >> x) & 1) {
                                px |= 1 << (x << 1);
                        }
                }
                eris_king_kram_write(px);

                eris_king_set_kram_write(kram + (y << 5) + 1, 1);
                px = 0;
                for(x = 0; x < 8; x++) {
                        if((glyph[y] >> (x+8)) & 1) {
                                px |= 1 << (x << 1);
                        }
                }
                eris_king_kram_write(px);
        }
}

