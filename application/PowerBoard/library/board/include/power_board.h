//-----------------------------------------------------------------------------
// Title         : Power Board Header
// Project       : Marmote Power Supply Board
//-----------------------------------------------------------------------------
// File          : power_board.h
// Author        : Sandor Szilvasi
// Company       : Vanderbilt University, ISIS
// Created       : 2011-11-02 20:13
// Last update   : 2011-11-02 20:15
// Platform      : Marmote
// Target device : STM32F102CB
// Tool version  : ARM uVision 4 (v4.22.22.0)
// Standard      : CMSIS
//-----------------------------------------------------------------------------
// Description   : <+description+>
//-----------------------------------------------------------------------------
// Copyright (c) 2006-2011, Vanderbilt University
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice, the following
// two paragraphs and the author appear in all copies of this software.
//
// IN NO EVENT SHALL THE VANDERBILT UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE VANDERBILT
// UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE VANDERBILT UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
// ON AN "AS IS" BASIS, AND THE VANDERBILT UNIVERSITY HAS NO OBLIGATION TO
// PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//-----------------------------------------------------------------------------
// Revisions     :
// Date            Version  Author			Description
// 2011-11-02      1.0      Sandor Szilvasi	Created
//-----------------------------------------------------------------------------

#ifndef __POWER_BOARD_H
#define __POWER_BOARD_H

#include "stm32f10x.h"
#include "power_control.h"

// Board - Initialize board peripherals and GPIOs with default values
void Board_Init(void); // 


// LEDs

typedef enum 
{
  LED1 = 1,
  LED2 = 2
} Led_TypeDef;

#define POWER_BOARD_LED1_Pos    15    
#define POWER_BOARD_LED1_Msk    (0x1UL << POWER_BOARD_LED1_Pos)
#define POWER_BOARD_LED2_Pos    2    
#define POWER_BOARD_LED2_Msk    (0x1UL << POWER_BOARD_LED2_Pos)

void LED_Init(void);
void LED_On (uint32_t led);
void LED_Off (uint32_t led);
void LED_Toggle (uint32_t led);

// SmartFusion connector

typedef enum
{
    CON_GPIO0 = 1,
    CON_GPIO1 = 2,
    CON_GPIO2 = 4,
    CON_GPIO3 = 8,
    CON_GPIO4 = 16
} CON_GPIO_TypeDef;

// * means limited IO capabilities
#define CON_GPIO0_Pos    14 // PC14* - CON PIN30
#define CON_GPIO0_Msk    (0x1UL << CON_GPIO0_Pos)
#define CON_GPIO1_Pos    15 // PC15* - CON PIN32
#define CON_GPIO1_Msk    (0x1UL << CON_GPIO1_Pos)
#define CON_GPIO2_Pos     9 // PA9 - CON PIN34
#define CON_GPIO2_Msk    (0x1UL << CON_GPIO2_Pos)
#define CON_GPIO3_Pos    10 // PA10 - CON PIN36
#define CON_GPIO3_Msk    (0x1UL << CON_GPIO3_Pos)
#define CON_GPIO4_Pos     4 // PB4 - CON PIN38
#define CON_GPIO4_Msk    (0x1UL << CON_GPIO4_Pos)

void CON_GPIO_Init(void);
void CON_GPIO_Set(uint32_t gpio);
void CON_GPIO_Clear(uint32_t gpio);

#endif // __POWER_BOARD_H


