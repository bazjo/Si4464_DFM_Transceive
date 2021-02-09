/// @file comm_hal.c
/*
 * Filename:            comm_hal.c
 * Description:         This file containts a hardware abstraction for the communication stack to interface the radio. Those functions provided here
 *                      are functionalities from the stack point of view. These functions itself implement the necessary actions and commands of the 
 *                      corresponding radio library. For different radio hardware and libraries these functions need to be updated.
 * Author:              M. Malyska
 */

#include "comm_hal.h"

static uint8_t comm_hal_frequency = 0x00;
static uint16_t comm_hal_datarate = 0x12c0;

/**
 * This array get initialized with the prepared register settings from radio_config_Si446x.h which is
 * automatically generated by WDS-Studio. It provides the arrary initial value as the define RADIO_CONFIGURATION_DATA_ARRAY.
 */
uint8_t comm_hal_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;

/**
 * This function initializes the radio and loads all radio registers with its presets.
 */
void comm_hal_init(void){
  si446x_reset();
  
  si446x_configuration_init(comm_hal_cfg);

  si446x_part_info();
  if(Si446xCmd.PART_INFO.PART != RADIO_IC_TYPE){
    sys_error_handler(sys_error_radio);
  }
}

/**
 * This functions sends the necessary command to the radio in order to start frame transmission.
 */
void comm_hal_tx_start(void){
  si446x_start_tx(comm_hal_frequency, 0x00, COMM_FRAME_BUFFER_SIZE);
}

/**
 * This functions returns the amount of empty bytes in the radio fifo memory.
 * @return fifo memory empty space in bytes.
 */
uint8_t comm_hal_fifo_get_space(void){
  si446x_fifo_info_fast_read();
  return Si446xCmd.FIFO_INFO.TX_FIFO_SPACE;
}

/**
 * This functions writes multiple bytes into the radio fifo memory.
 * @param buffer Pointer to the data which is to be loaded into radio fifo
 * @param buffer_length How many bytes will be loaded into radio fifo.
 */
void comm_hal_fifo_write(uint8_t* buffer, uint8_t buffer_length){
  si446x_write_tx_fifo(buffer_length, buffer);
}

/**
 * This function will set the radio to a specific data rate based on MODEM_TX_NCO_MODE and TXOSR,
 * so both of these words have to be set prior to this function calling.
 * @param datarate This is the datarate in bps which the radio will be set to.
 */
void comm_hal_datarate_set(uint16_t datarate){
  uint8_t TXOSR = 0;
  uint32_t MODEM_TX_NCO_MODE = 0;
  uint32_t MODEM_DATA_RATE = 0;
  si446x_get_property(0x20, 0x07, 0x03);

  switch(((Si446xCmd.GET_PROPERTY.DATA[3] & 0x0c) >> 2)){ //Switch TXOR bits of property MODEM_TX_NCO_MODE
    case 0x00:{ //ENUM_0
      TXOSR = 10U; //TX Gaussian filter oversampling ratio is 10x.
      break;
    }
    case 0x01:{ //ENUM_1
      TXOSR = 40U; //TX Gaussian filter oversampling ratio is 40x.
      break;
    }
    case 0x02:{ //ENUM_2
      TXOSR = 20U; //TX Gaussian filter oversampling ratio is 20x.
      break;
    }
    default:{
      TXOSR = 10U;
      break;
    }
  };

  MODEM_TX_NCO_MODE |= ((Si446xCmd.GET_PROPERTY.DATA[3] & 0x03) << 24);
  MODEM_TX_NCO_MODE |= (Si446xCmd.GET_PROPERTY.DATA[4] << 16);
  MODEM_TX_NCO_MODE |= (Si446xCmd.GET_PROPERTY.DATA[5] << 8);
  MODEM_TX_NCO_MODE |= Si446xCmd.GET_PROPERTY.DATA[6];

  MODEM_DATA_RATE = (MODEM_TX_NCO_MODE / RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ); //First divide the biggest numbers to avoid overflow in multiplication.
  MODEM_DATA_RATE *= (datarate * TXOSR);
  si446x_set_property(0x20, 0x03, 0x03, (MODEM_DATA_RATE >> 16), ((MODEM_DATA_RATE >> 8) & (0xff)), (MODEM_DATA_RATE & 0xff));
}

/**
 * This functions sets the carrier frequency the radio will transmit on.
 * @param frequency This integer number will icrement the carrier frequency in 50kHz steps beginning at 400 MHz. f_c = 400MHz + (50kHz * frequency)
 */
void comm_hal_frequency_set(uint8_t frequency){
  if(frequency > RADIO_FREQUENCY_LIMIT_UPPER){
    frequency = RADIO_FREQUENCY_LIMIT_UPPER;
  }
  comm_hal_frequency = frequency;
}
