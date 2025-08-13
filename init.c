/*
 * Copyright wgchnln. All rights reserved.
 * function:BIT boot init
 * log:8.1.2019 first create this file
*/

#include "./early_print.h"
#include "./early_console.h"
#include "boot_info.h"
#include "tsc.h"

#define  DEBUG_KERNEL_ADDR	0

#ifndef NULL
#define NULL			((void *)0)
#endif

#define MENU_INDEX_END		0xFF
#define MENU_INDEX_MAX		10


#define INPUT_TIME_OUT		3

typedef void (*__func)(void);

struct menu_list {
	uint8_t	index;
	char *pdesc;
	__func func;
};

extern void reboot(void);
static void log_show(void);
static void menu_show(void);
static void menu_run(void);
static uint64_t get_tsc_to_sec(void);
static uint8_t ascii_to_dec(uint8_t ascii);
static uint8_t dec_to_ascii(uint8_t ascii);
static void menu_clear(void);
static void time_display(void);

struct menu_list menu[MENU_INDEX_MAX] =
{
	{1, "reboot", reboot},
	{2, "start kernel", start_kernel},
	{MENU_INDEX_END, NULL, NULL}
};

/*
 * fuction:physics primary CPU init 
 * parameter：void
 *      in:
 *      out:
 * return:void
 * note:BP init
*/
void main(void)
{
	log_show();

#if DEBUG_KERNEL_ADDR
	printf("\nEnter %s(file:%s,line:%d), function address @ 0x%lx. %s", __FUNCTION__, __FILE__, __LINE__, phyPrimaryCpuInit, __TIME__);
	dump_mem(0x100000, 0x100);
#endif

	do {
		menu_show();
		menu_run();
		menu_clear();
	}while (1);
}

static void menu_clear(void)
{
	printf("\033[2J");
	printf("\033[0;0H");
}

static void menu_show(void)
{
	struct menu_list *p_menu = (struct menu_list *)(&menu[0]);

	do {
		printf("\n        %d | %s", p_menu->index, p_menu->pdesc);
		p_menu++;
	} while(MENU_INDEX_END != p_menu->index);
}

static void menu_run(void)
{
	struct menu_list *p_menu = (struct menu_list *)(&menu[0]);
	static uint8_t input_status = 0;
	uint64_t sec, sec_now;
	uint8_t times;
	uint8_t input;
	
	sec_now = 0;
	times = 0;

	printf("\n        Please input index number to continue\n");
	
	do {
		input = serial_read();
		if (!input_status) {
			sec = get_tsc_to_sec();
			if(sec != sec_now) {
				sec_now = sec;
				times++;
				printf("\r  %d", INPUT_TIME_OUT-times);
			}
			
			/*default select start kernel*/
			if (INPUT_TIME_OUT == times) {
				input = dec_to_ascii(2);
			}
		} else {
			time_display();
		}
	} while(0 == input);
	
	input_status = 1;
	input = ascii_to_dec(input);
	
	do {
		if (input == p_menu->index) {
			p_menu->func();
		}
		p_menu++;
	} while(MENU_INDEX_END != p_menu->index);
}


/*
 * fuction:relocation the code at the memory
 * parameter：void
 *      in:
 *      out:
 * return:void
 * note:TO DO
 * 
*/
void relocate(void)
{
	/*TO DO*/
}


static void log_show(void)
{

	printf("\033[2J");
	printf("\033[0;0H");

	printf("\n\n");
	printf("\n        +---------------------------------+");
	printf("\n        |                                 |");
	printf("\n        |         \033[7myuan  bootloader\033[0m        |");
	printf("\n        |                                 |");
	printf("\n        +---------------------------------+");
	printf("\n");
}

static uint64_t get_tsc_to_sec(void)
{
	unsigned long long tsc, tsc_sec;

	tsc = rdtsc();
	tsc_sec = tsc/CPU_FREQ_HZ;

	return tsc_sec;
}

static uint8_t ascii_to_dec(uint8_t ascii)
{
	return ascii-0x30;
}


static uint8_t dec_to_ascii(uint8_t ascii)
{
	return ascii+0x30;
}

static void time_display(void)
{
	unsigned int time_d, time_h, time_m, time_s;
	unsigned long long tsc;
	static unsigned int tsc_old = 0;

	tsc = rdtsc();
	if (tsc_old != tsc/CPU_FREQ_HZ) {
		tsc_old = tsc/CPU_FREQ_HZ;
		time_s = tsc_old%60;
		time_m = (tsc_old/60)%60;
		time_h = (tsc_old/60/60)%24;
		time_d = (tsc_old/60/60)/24;
		
		printf("\r                            %d : %02d : %02d : %02d", time_d, time_h, time_m, time_s);
	}
}
