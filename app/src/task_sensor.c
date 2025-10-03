/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file   : task_sensor.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_sensor_attribute.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"

/********************** macros and definitions *******************************/
#define G_TASK_SEN_CNT_INIT     0ul
#define G_TASK_SEN_TICK_CNT_INI 0ul

#define DEL_BTN_XX_MIN          0ul
#define DEL_BTN_XX_MED          25ul
#define DEL_BTN_XX_MAX          50ul

/********************** internal data declaration ****************************/

const task_sensor_cfg_t task_sensor_cfg_list[] = {
    {ID_BTN_A,  BTN_A_PORT,       BTN_A_PIN,  BTN_A_PRESSED, DEL_BTN_XX_MAX, EV_SYS_IDLE, EV_SYS_IDLE},
    {ID_BTN_B,  BTN_B_GPIO_Port,  BTN_B_Pin,  BTN_A_PRESSED, DEL_BTN_XX_MAX, EV_SYS_NOT_LOOP_DET, EV_SYS_LOOP_DET},
    {ID_BTN_C,  BTN_C_GPIO_Port,  BTN_C_Pin,  BTN_A_PRESSED, DEL_BTN_XX_MAX, EV_SYS_NOT_MANUAL_BTN, EV_SYS_MANUAL_BTN},
    {ID_BTN_D,  BTN_D_GPIO_Port,  BTN_D_Pin,  BTN_A_PRESSED, DEL_BTN_XX_MAX, EV_SYS_NOT_IR_PHO_CELL, EV_SYS_IR_PHO_CELL}
};

#define SENSOR_CFG_QTY (sizeof(task_sensor_cfg_list)/sizeof(task_sensor_cfg_t))

task_sensor_dta_t task_sensor_dta_list[] = {
    {DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP},
    {DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP},
    {DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP},
    {DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP}
};

#define SENSOR_DTA_QTY (sizeof(task_sensor_dta_list)/sizeof(task_sensor_dta_t))

/********************** internal functions declaration ***********************/
void task_sensor_statechart(void);

/********************** internal data definition *****************************/
const char *p_task_sensor = "Task Sensor (Sensor Statechart)";
const char *p_task_sensor_ = "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_sensor_cnt;
volatile uint32_t g_task_sensor_tick_cnt;

/********************** external functions definition ************************/
void task_sensor_init(void *parameters)
{
    uint32_t index;
    task_sensor_dta_t *p_task_sensor_dta;
    task_sensor_st_t state;
    task_sensor_ev_t event;

    /* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_sensor_init), p_task_sensor);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_sensor), p_task_sensor_);

    /* Init & Print out: Task execution counter */
    g_task_sensor_cnt = G_TASK_SEN_CNT_INIT;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_sensor_cnt), g_task_sensor_cnt);

    for (index = 0; SENSOR_DTA_QTY > index; index++)
    {
        /* Update Task Sensor Data Pointer */
        p_task_sensor_dta = &task_sensor_dta_list[index];

        /* Init & Print out: Index & Task execution FSM */
        state = ST_BTN_XX_UP;
        p_task_sensor_dta->state = state;

        event = EV_BTN_XX_UP;
        p_task_sensor_dta->event = event;

        LOGGER_INFO(" ");
        LOGGER_INFO("   %s = %lu   %s = %lu   %s = %lu",
                GET_NAME(index), index,
                GET_NAME(state), (uint32_t)state,
                GET_NAME(event), (uint32_t)event);
    }
}

void task_sensor_update(void *parameters)
{
    bool b_time_update_required = false;

    /* Protect shared resource */
    __asm("CPSID i");	/* disable interrupts */
    if (G_TASK_SEN_TICK_CNT_INI < g_task_sensor_tick_cnt)
    {
        /* Update Tick Counter */
        g_task_sensor_tick_cnt--;
        b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts */

    while (b_time_update_required)
    {
        /* Update Task Counter */
        g_task_sensor_cnt++;

        /* Run Task Statechart */
        task_sensor_statechart();

        /* Protect shared resource */
        __asm("CPSID i");	/* disable interrupts */
        if (G_TASK_SEN_TICK_CNT_INI < g_task_sensor_tick_cnt)
        {
            /* Update Tick Counter */
            g_task_sensor_tick_cnt--;
            b_time_update_required = true;
        }
        else
        {
            b_time_update_required = false;
        }
        __asm("CPSIE i");	/* enable interrupts */
    }
}

void task_sensor_statechart(void)
{
    uint32_t index;
    uint8_t sensor_pin_status;
    const task_sensor_cfg_t *p_task_sensor_cfg;
    task_sensor_dta_t *p_task_sensor_dta;

    for (index = 0; SENSOR_DTA_QTY > index; index++)
    {
        /* Update Task Sensor Configuration & Data Pointer */
        p_task_sensor_cfg = &task_sensor_cfg_list[index];
        p_task_sensor_dta = &task_sensor_dta_list[index];

        sensor_pin_status = HAL_GPIO_ReadPin(p_task_sensor_cfg->gpio_port, p_task_sensor_cfg->pin);
        if (sensor_pin_status == p_task_sensor_cfg->pressed)
        {
            p_task_sensor_dta->event = EV_BTN_XX_DOWN;
        }
        else
        {
            p_task_sensor_dta->event = EV_BTN_XX_UP;
        }

        switch (p_task_sensor_dta->state)
        {
            case ST_BTN_XX_UP:
                if (EV_BTN_XX_DOWN == p_task_sensor_dta->event)
                {
                    p_task_sensor_dta->state = ST_BTN_XX_FALLING;
                    p_task_sensor_dta->tick  = DEL_BTN_XX_MAX;
                }
            break;

            case ST_BTN_XX_FALLING:
                if (EV_BTN_XX_UP == p_task_sensor_dta->event)
                {
                    if (0 == p_task_sensor_dta->tick) {
                        p_task_sensor_dta->state = ST_BTN_XX_UP;
                    }

                    if (p_task_sensor_dta->tick > 0) {
                        p_task_sensor_dta->state = ST_BTN_XX_FALLING;
                        p_task_sensor_dta->tick--;
                    }
                }

                if (EV_BTN_XX_DOWN == p_task_sensor_dta->event)
                {
                    if (0 == p_task_sensor_dta->tick) {
                        p_task_sensor_dta->state = ST_BTN_XX_DOWN;
                        put_event_task_system(p_task_sensor_cfg->signal_down);
                    }

                    if (p_task_sensor_dta->tick > 0) {
                        p_task_sensor_dta->tick--;
                    }
                }
            break;

            case ST_BTN_XX_DOWN:
                if (EV_BTN_XX_UP == p_task_sensor_dta->event)
                {
                    p_task_sensor_dta->state = ST_BTN_XX_RISING;
                    p_task_sensor_dta->tick  = DEL_BTN_XX_MAX;
                }
            break;

            case ST_BTN_XX_RISING:
                if (EV_BTN_XX_UP == p_task_sensor_dta->event)
                {
                    if (0 == p_task_sensor_dta->tick) {
                        put_event_task_system(p_task_sensor_cfg->signal_up);
                        p_task_sensor_dta->state = ST_BTN_XX_UP;
                    }

                    if (p_task_sensor_dta->tick > 0) {
                        p_task_sensor_dta->tick--;
                    }
                }

                if (EV_BTN_XX_DOWN == p_task_sensor_dta->event)
                {
                    if (0 == p_task_sensor_dta->tick) {
                        p_task_sensor_dta->state = ST_BTN_XX_DOWN;
                    }

                    if (p_task_sensor_dta->tick > 0) {
                        p_task_sensor_dta->tick--;
                    }
                }
            break;

            default:
                p_task_sensor_dta->tick  = DEL_BTN_XX_MIN;
                p_task_sensor_dta->state = ST_BTN_XX_UP;
                p_task_sensor_dta->event = EV_BTN_XX_UP;
            break;
        }
    }
}

/********************** end of file ******************************************/
