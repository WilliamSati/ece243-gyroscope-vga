
#include "address_map_arm.h"
#include "interrupt.h"

// useful constants
#define KEYS_IRQ 73 	
#define IRQ_MODE 0b10010//18	
#define INT_DISABLE 0b11000000//192
#define INT_ENABLE 0b01000000
#define ENABLE 0x1		
#define SVC_MODE 0b10011


//configure the keys to generate interrupts
void config_KEYs();
//Initialize the banked stack pointer register for IRQ mode
void set_A9_IRQ_stack(void);
//configre the GIC to generate the interrupts for certain devices
void config_GIC(void);
//allow the board to generate interupts
void enable_A9_interrupts(void);

//declare the board's exeption handlers
void __attribute__((interrupt)) __cs3_isr_irq(void);
void __attribute__((interrupt)) __cs3_reset(void);
void __attribute__((interrupt)) __cs3_isr_undef(void);
void __attribute__((interrupt)) __cs3_isr_swi(void);
void __attribute__((interrupt)) __cs3_isr_pabort(void);
void __attribute__((interrupt)) __cs3_isr_dabort(void);
void __attribute__((interrupt)) __cs3_isr_fiq(void);