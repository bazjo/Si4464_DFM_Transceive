/// @file comm_hal.h
/*
 * Filename:            comm_hal.h
 * Description:         This file containts comm_hal.c related function headers.
 * Author:              M. Malyska
 */

#ifndef COMM_HAL_H
#define COMM_HAL_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*INCLUDES*/
#include "stm32f1xx.h"
#include <stdint.h>
#include "sys.h"
#include "radio/Si446x/si446x_api_lib.h"
#include "radio/Si446x/radio_config_Si4464.h"
#include "comm.h"

//Todo: move wds generated file radio_congig... into radio folder

/*GLOBAL VARIABLES*/

/*enumERATORS*/

/*typedefINITIONS*/

/*DEFINES*/
#define RADIO_IC_TYPE 0x4464

//Todo: implement data rate switching functions
#define RADIO_DATA_RATE 4800

#define RADIO_FIFO_SIZE 0x3b //!< Si4x3x FIFO is 64 byte long minus 5 byte ae
#define RADIO_FIFO_ALMOST_EMPTY_Msk 0x01

/*PUBLIC PROTOTYPES*/
void comm_hal_init(void);
void comm_hal_tx_start(void);
uint8_t comm_hal_fifo_get_space(void);
void comm_hal_fifo_write(uint8_t* buffer, uint8_t buffer_length);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RADIO_H */
