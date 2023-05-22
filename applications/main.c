/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2018-11-19     flybreak     add stm32f407-atk-explorer bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>

static rt_thread_t thread_1 = RT_NULL;
static rt_thread_t thread_2 = RT_NULL;

void app_1()
{
    static uint32_t count = 0;
    while (1)
    {
       // rt_kprintf("hellow world %u\n",count++);
        rt_thread_mdelay(1000);
    }
}

void app_2()
{
    while (1)
    {
        rt_thread_mdelay(1000);
    }
}

/* defined the LED0 pin: PF9 */
// #define LED0_PIN    GET_PIN(F, 9)

int main(void)
{
    rt_err_t err;

    /* set LED0 pin mode to output */
    // rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT
    thread_1 = rt_thread_create("app_1", app_1, NULL, 256, 5, 1);
    if (thread_1 == RT_NULL)
    {
    }

    err = rt_thread_startup(thread_1);
    if (err)
    {
        printf("start rhtread 1 faild\n");
    }

    thread_2 = rt_thread_create("app_2", app_2, NULL, 256, 9, 5);
    if (thread_2 == RT_NULL)
    {
    }

    err = rt_thread_startup(thread_2);
    if (err)
    {
        printf("start rhtread 2 faild\n");
    }

    while (1)
    {
        //   rt_pin_write(LED0_PIN, PIN_HIGH);
        //  rt_thread_mdelay(500);
        //  rt_pin_write(LED0_PIN, PIN_LOW);
        // rt_thread_mdelay(500);
        rt_thread_mdelay(1000);
    }
}