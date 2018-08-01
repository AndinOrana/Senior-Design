/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
// #include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
// #include <ti/drivers/USBMSCHFatFs.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

/* Board Header file */
#include "Board.h"

#define TASK0STACKSIZE   512
#define TASK1STACKSIZE   2048
#define UARTBAUDRATE    115200

Task_Struct task0Struct;
Task_Struct task1Struct;
Char task0Stack[TASK0STACKSIZE];
Char task1Stack[TASK1STACKSIZE];

/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void heartBeatFxn(UArg arg0, UArg arg1)
{
    while (1) {
        Task_sleep((unsigned int)arg0);
        GPIO_toggle(Board_LED0);
    }
}


Void UARTFxn(UArg arg0, UArg arg1)
{
    UART_Handle uart;
    UART_Params uartParams;

    char rxBuffer[8];

    //const char ledOn[] = {'L', ',', '1', 0x0D};
    //const char ledOff[] = {'L', ',', '0', 0x0D};
    const char C_0[] = {'C', ',', '0', 0x0D};       //disable continuous reading
    const char takeReading[] = {'R' , 0x0D};

    //uint8_t ledOn[] = {'L', ',', '1', 0x0D};
    //uint8_t ledOff[] = {'L', ',', '0', 0x0D};
    //const char prompt[] = "-----------UART Start------------\r\n";
    //const char done[] = "Done\r\n";



    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readTimeout    = 200;//ms
    uartParams.baudRate = UARTBAUDRATE;
    uart = UART_open(Board_UART3, &uartParams);

    if (uart == NULL) {
        System_abort("Error opening the UART");
    }

    //UART_write(uart, &prompt, sizeof(prompt));

    UART_write(uart, &C_0, sizeof(C_0));        //stop continuous reading

    //UART_write(uart, &ledOn, sizeof(ledOn));


    while(1)
    {
    Task_sleep(2200);   //wait 2 seconds


    //UART_read(uart, &rxBuffer, sizeof(rxBuffer));
    //UART_write(uart, &ledOff, sizeof(ledOff));

    UART_write(uart, &takeReading, sizeof(takeReading));       //send command to sensor to take a reading (LED turns cyan for 600ms)
    Task_sleep(500);
    UART_read(uart, &rxBuffer, sizeof(rxBuffer));             //read from UART and store to rxBuffer

    System_printf("Value is: %s\n",rxBuffer);                 //print rxBuffer to console
    System_flush();

    //Task_sleep(1000);
    //UART_write(uart, &ledOn, sizeof(ledOn));
   // for(i = 0; i = sizeof(rxBuffer); i++)
   // {
     //   if (rxBuffer[i] == "\r")

    //}
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;
    /* Call board init functions */
    Board_initGeneral();
    // Board_initEMAC();
    Board_initGPIO();
    // Board_initI2C();
    // Board_initSDSPI();
    // Board_initSPI();
    Board_initUART();
    // Board_initUSB(Board_USBDEVICE);
    // Board_initUSBMSCHFatFs();
    // Board_initWatchdog();
    // Board_initWiFi();

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASK0STACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);

    /* Construct UART Task  thread */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASK1STACKSIZE;
    taskParams.stack = &task1Stack;
    Task_construct(&task1Struct, (Task_FuncPtr)UARTFxn, &taskParams, NULL);



     /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
