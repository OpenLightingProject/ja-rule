/*******************************************************************************
  System Initialization File

  File Name:
    system_init.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, configuration bits, and allocates
    any necessary global system resources, such as the systemObjects structure
    that contains the object handles to all the MPLAB Harmony module objects in
    the system.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system_config.h"
#include "system_definitions.h"
#include "usb_descriptors.h"
#include "app.h"


// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************

/*** DEVCFG0 ***/

#pragma config DEBUG =      OFF
#pragma config ICESEL =     ICS_PGx1
#pragma config PWP =        0xf9
#pragma config BWP =        OFF
#pragma config CP =         OFF

/*** DEVCFG1 ***/

#pragma config FNOSC =      PRIPLL
#pragma config FSOSCEN =    OFF
#pragma config IESO =       OFF
#pragma config POSCMOD =    XT
#pragma config OSCIOFNC =   OFF
#pragma config FPBDIV =     DIV_1
#pragma config FCKSM =      CSDCMD
#pragma config WDTPS =      PS1048576
#pragma config FWDTEN =     OFF

/*** DEVCFG2 ***/

#pragma config FPLLIDIV =   DIV_2
#pragma config FPLLMUL =    MUL_20
#pragma config FPLLODIV =   DIV_1
#pragma config UPLLIDIV =   DIV_2
#pragma config UPLLEN =     ON

/*** DEVCFG3 ***/

#pragma config USERID =     0xffff
#pragma config FSRSSEL =    PRIORITY_7
#pragma config FMIIEN =     OFF
#pragma config FETHIO =     OFF
#pragma config FUSBIDIO =   OFF
#pragma config FVBUSONIO =  OFF


// *****************************************************************************
// *****************************************************************************
// Section: Library/Stack Initialization Data
// *****************************************************************************
// *****************************************************************************/

//<editor-fold defaultstate="collapsed" desc="USB Stack Configuration">

/**************************************************
 * USB Device Function Driver Init Data
 **************************************************/
    const USB_DEVICE_CDC_INIT cdcInit0 =
    {
        .queueSizeRead = 1,
        .queueSizeWrite = 1,
        .queueSizeSerialStateNotification = 1
    };
/**************************************************
 * USB Device Layer Function Driver Registration 
 * Table
 **************************************************/
const USB_DEVICE_FUNCTION_REGISTRATION_TABLE funcRegistrationTable[2] =
{
    /* Function 1 */
    { 
        .configurationValue = 1,    /* Configuration value */ 
        .interfaceNumber = 0,       /* First interfaceNumber of this function */ 
        .numberOfInterfaces = 2,    /* Number of interfaces */
        .speed = USB_SPEED_FULL,    /* Function Speed */ 
        .funcDriverIndex = 0,  /* Index of CDC Function Driver */
        .driver = (void*)USB_DEVICE_CDC_FUNCTION_DRIVER,    /* USB CDC function data exposed to device layer */
        .funcDriverInit = (void*)&cdcInit0    /* Function driver init data */
    },
    /* Function 2 */
    {
        .configurationValue = 1,    /* Configuration value */
        .interfaceNumber = 2,       /* First interfaceNumber of this function */
        .numberOfInterfaces = 1,    /* Number of interfaces */
        .speed = USB_SPEED_FULL,    /* Function Speed */ 
        .funcDriverIndex = 0,  /* Index of Vendor Driver */
        .driver = NULL,            /* No Function Driver data */ 
        .funcDriverInit = NULL     /* No Function Driver Init data */
    },
};

/*******************************************
 * USB Device Layer Descriptors
 *******************************************/
//TODO - Copy USB Descriptors - Device descriptor,
//Configuration Descriptors and String Descriptors. 


/*******************************************
 * USB Device Layer Master Descriptor Table 
 *******************************************/
const USB_DEVICE_MASTER_DESCRIPTOR usbMasterDescriptor =
{
    //TODO: Add Master Descriptor here. 
};

/****************************************************
 * Endpoint Table needed by the Device Layer.
 ****************************************************/
uint8_t __attribute__((aligned(512))) endPointTable[USB_DEVICE_ENDPOINT_TABLE_SIZE];

/****************************************************
 * USB Device Layer Initialization Data
 ****************************************************/

const USB_DEVICE_INIT usbDevInitData =
{
    /* System module initialization */
    .moduleInit = {SYS_MODULE_POWER_RUN_FULL},

	/* Identifies peripheral (PLIB-level) ID */
    .usbID = USB_ID_1,

    /* Stop in idle */
    .stopInIdle = false,

    /* Suspend in sleep */
    .suspendInSleep = false,
    /* Interrupt Source for USB module */
    .interruptSource = INT_SOURCE_USB_1,

    /* Endpoint table */
    .endpointTable= endPointTable,

    /* Number of function drivers registered to this instance of the
       USB device layer */
    .registeredFuncCount = 2,

    /* Function driver table registered to this instance of the USB device layer*/
    .registeredFunctions = (USB_DEVICE_FUNCTION_REGISTRATION_TABLE*)funcRegistrationTable,

    /* Pointer to USB Descriptor structure */
    .usbMasterDescriptor = (USB_DEVICE_MASTER_DESCRIPTOR*)&usbMasterDescriptor,

    /* USB Device Speed */
    .deviceSpeed = USB_SPEED_FULL,

    /* Specify queue size for vendor endpoint read */
    .queueSizeEndpointRead = 1,

    /* Specify queue size for vendor endpoint write */
    .queueSizeEndpointWrite= 1,
};

// </editor-fold>


// *****************************************************************************
// *****************************************************************************
// Section: Driver Initialization Data
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: System Data
// *****************************************************************************
// *****************************************************************************

/* Structure to hold the object handles for the modules in the system. */
SYSTEM_OBJECTS sysObj;

// *****************************************************************************
// *****************************************************************************
// Section: Module Initialization Data
// *****************************************************************************
// *****************************************************************************


//<editor-fold defaultstate="collapsed" desc="SYS_DEVCON Configuration">

/*** System Device Control Initialization Data ***/

const SYS_DEVCON_INIT sysDevconInit =
{
    .moduleInit = {0},
};
// </editor-fold>

// *****************************************************************************
// *****************************************************************************
// Section: Static Initialization Functions
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: System Initialization
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void SYS_Initialize ( SYS_INIT_DATA *data )

  Summary:
    Initializes the board, services, drivers, application and other modules.

  Remarks:
    See prototype in system/common/sys_module.h.
 */

void SYS_Initialize ( void* data )
{
  /* Core Processor Initialization */
  SYS_CLK_Initialize( NULL );
  sysObj.sysDevcon = SYS_DEVCON_Initialize(SYS_DEVCON_INDEX_0, (SYS_MODULE_INIT*)&sysDevconInit);
  SYS_DEVCON_PerformanceConfig(SYS_CLK_SystemFrequencyGet());
  SYS_DEVCON_JTAGDisable();
  SYS_PORTS_Initialize();

  /* Initialize Drivers */

  /* Initialize System Services */
  SYS_INT_Initialize();

  /* Initialize Middleware */
  /* Set priority of USB interrupt source */
  SYS_INT_VectorPrioritySet(INT_VECTOR_USB1, INT_PRIORITY_LEVEL4);

  /* Set Sub-priority of USB interrupt source */
  SYS_INT_VectorSubprioritySet(INT_VECTOR_USB1, INT_SUBPRIORITY_LEVEL0);

  /* Initialize the USB device layer */
  sysObj.usbDevObject0 = USB_DEVICE_Initialize(
      USB_DEVICE_INDEX_0, (SYS_MODULE_INIT*) USBDescriptor_GetDeviceConfig());

  /* Enable Global Interrupts */
  SYS_INT_Enable();

  /* Initialize the Application */
  APP_Initialize();

}

/*******************************************************************************
 End of File
*/

