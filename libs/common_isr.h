/***************************************************************************//**
 * @file    common_isr.h
 * @author  Christopher Haas
 * @date    21.06.23
 *
 * @brief   To implement the callbacks
 *
 ******************************************************************************/

#ifndef LIBS_COMMON_ISR_H
#define LIBS_COMMON_ISR_H

typedef void (*ISR_callback)(void);

void tx_callback(ISR_callback tx_callback);
void rx_callback(ISR_callback rx_callback);

#endif /* LIBS_COMMON_ISR_H_ */
