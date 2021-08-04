/// @file main.c
/*
 * Filename:            main.c
 * Description:         This file is the entry point for program execution.
 * Author:              M. Malyska
 */
#include "stm32f1xx.h"
#include "spi.h"
#include "rcc.h"
#include "gpio.h"
#include "timer.h"
#include "protocol.h"
#include  <stdio.h>

int send_frames(uint8_t* framebuffer, int framecount);
int set_modtype(int modtype);
int adjust_xtal(int8_t adj);
int set_freq(double freq, int16_t adj);
int set_power(uint8_t power);

/**
 * After the startup code finishes it will call this function, here is the entry point for user code exeution.
 */
void main(void)
{
  sys_init();
  rcc_init();
  gpio_init();
  spi1_init();
  timer_init();

  si446x_reset();
  
  uint8_t comm_hal_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;
  si446x_configuration_init(comm_hal_cfg);

  set_modtype(0);
  adjust_xtal(0x02);

  set_power(0x10);

  set_freq(403.0, 0x00);

  uint8_t buffer[35] = {0x45, 0xcf, 0x6c, 0xb8, 0xa5, 0x2d, 0x1c, 0x25, 0x0c, 0x4d, 0x03, 0xc0, 0x1d, 0xc3, 0x12, 0x08, 0x60, 0xd9, 0x86, 0xf0, 0x08, 0x41, 0x24, 0x72, 0x6a, 0x85, 0xa2, 0x25, 0x34, 0x94, 0x10, 0x31, 0xee, 0xef, 0xf5};
  send_frames(buffer, 1);

}

int send_frames(uint8_t* framebuffer, int framecount) {
  
  si446x_write_tx_fifo(35, framebuffer);
  
  if(framecount == 200) {
    si446x_start_tx(0x00, 0x00, 0x1B85);
  } else if(framecount == 1000) {
    si446x_start_tx(0x00, 0x00, 0x00);
  } else if(framecount == 1) {
    si446x_start_tx(0x00, 0x00, 0x35);
    return -1;
  }

  uint8_t tx_fifo_space;
  for(int i = 0; i < framecount; i++){
    do{
      si446x_fifo_info(0b00);
      tx_fifo_space = Si446xCmd.FIFO_INFO.TX_FIFO_SPACE;
    } while(tx_fifo_space < 35);

    si446x_write_tx_fifo(35, framebuffer);
  }

  return 0;
}

int set_modtype(int modtype){
  switch(modtype){
    case 0://Set Modulation type to 2GFSK, Set Modulation Source to Packet Handler
      si446x_set_property(0x20, 0x01, 0x00, 0b00000011); 
      return 0;
    case 1://Set Modulation type to CW, Set Modulation Source to PN9
      si446x_set_property(0x20, 0x01, 0x00, 0b00010000); 
      return 0;
    case 2://Set Modulation type to 2GFSK, Set Modulation Source to PN9
      si446x_set_property(0x20, 0x01, 0x00, 0b00010011);  
      return 0;
    case 3://Set Modulation type to 2FSK, Set Modulation Source to Packet Handler
      si446x_set_property(0x20, 0x01, 0x00, 0b00000010);
      return 0;
    default:
      return -1;
  }
}

int adjust_xtal(int8_t adj){
  //Positive Adjustment decreases frequency
  si446x_set_property(0x00, 0x01, 0x00, 0x40 + adj);
}

int set_freq(double freq, int16_t adj){
  if (freq < 400.0 || freq > 406.00) return -1;
  uint8_t Reg_Int = (freq < 403.0) ? 61 : 62;

  double F_Frac = (freq / 6.5) - (double) Reg_Int;
  uint32_t Reg_Frag = (int) (F_Frac * 524288);
  Reg_Frag += adj;

  //printf("RegFrag: %x", Reg_Frag);

  si446x_set_property(0x40, 0x04, 0x00, Reg_Int - 1, (Reg_Frag >> 16) & 0x0F, (Reg_Frag >> 8) & 0xFF, (Reg_Frag >> 0) & 0xFF);
  //si446x_set_property(0x40, 0x04, 0x00, 0x3C, 0x0C, 0x4E, 0xFF);

  return 0;
}

int set_power(uint8_t power){
  si446x_set_property(0x22, 0x01, 0x01, power); //Set Output Power Level
}