#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define LED_PIO           PIOC
#define LED_PIO_ID        ID_PIOC
#define LED_PIO_IDX       8
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)

#define BUT1_PIO			PIOD
#define BUT1_PIO_ID			ID_PIOD
#define BUT1_PIO_IDX		28
#define BUT1_PIO_IDX_MASK (1 << BUT1_PIO_IDX)

#define BUT2_PIO			PIOC
#define BUT2_PIO_ID			ID_PIOC
#define BUT2_PIO_IDX		31
#define BUT2_PIO_IDX_MASK	(1 << BUT2_PIO_IDX)

#define BUT3_PIO			PIOA
#define BUT3_PIO_ID			ID_PIOA
#define BUT3_PIO_IDX		19
#define BUT3_PIO_IDX_MASK	(1 << BUT3_PIO_IDX)

volatile int freq = 1;
volatile int but1_time_pressed;
volatile int but3_time_pressed;
volatile int active = 1;

int max_twitch = 30;
volatile int current_twitch = 0;

void twitch(void);
void but1_callback(void);
void but1_pressed(void);
void but1_released(void);
void but2_callback(void);
void but3_callback(void);


void twitch()
{
	double delay_ms = (1.0 / freq) * 1000;
	pio_clear(LED_PIO, LED_PIO_IDX_MASK);
	delay_ms(delay_ms);
	pio_set(LED_PIO, LED_PIO_IDX_MASK);
	delay_ms(delay_ms);
	
	current_twitch += 1;
	
	double w = ((double)current_twitch / max_twitch)*120;
	gfx_mono_generic_draw_filled_rect(1, 0, w, 5, 1);
}

void but1_callback() {
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		but1_pressed();
	} else {
		but1_released();
	}
}

void but1_pressed() 
{		
	while (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		but1_time_pressed += 1;
		delay_ms(1);
	}
}

void but1_released() {	
	if (but1_time_pressed > 500) {
		if (freq > 1) { freq -= 1; }
		} else {
		freq += 1;
	}
	
	but1_time_pressed = 0;
	
	char str[128];
	sprintf(str, "%d", freq);
	gfx_mono_draw_string("   ", 50,16, &sysfont);
	gfx_mono_draw_string(str, 50,16, &sysfont);
}

void but3_pressed()
{
	while (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
		but3_time_pressed += 1;
		delay_ms(1);
	}
}

void but3_released() {
	if (but3_time_pressed < 500) {
		if (freq > 1) { freq -= 1; }
	}
	
	but3_time_pressed = 0;
	
	char str[128];
	sprintf(str, "%d", freq);
	gfx_mono_draw_string("   ", 50,16, &sysfont);
	gfx_mono_draw_string(str, 50,16, &sysfont);
}

void but3_callback() {
	if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
		but3_pressed();
		} else {
		but3_released();
	}
}

void but2_callback()
{
	if (active) {
		active = 0;
	} else {
		active = 1;
	}
}

int init (void)
{
	board_init();
	sysclk_init();
	delay_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_PIO_IDX_MASK, PIO_DEFAULT);
	
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	pmc_enable_periph_clk(BUT2_PIO_ID);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);
	
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but1_callback);

	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);

	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4);
	
	
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but2_callback);

	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);

	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4);
	
	
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but3_callback);

	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);

	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4);
	
	gfx_mono_ssd1306_init();
	
}

//int configure_pushbutton()

int main (void)
{
	init();
	
	char str[128];
	sprintf(str, "%d", freq);
	gfx_mono_draw_string(str, 50,16, &sysfont);
	gfx_mono_draw_string("Hz", 100,16, &sysfont);
  
	while(1)
	{	
		if (active && current_twitch <= max_twitch) {
			twitch();
		}
	}
}
