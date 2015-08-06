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
#include "constants.h"
#include "app.h"


// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************

/*** DEVCFG0 ***/

#pragma config DEBUG =      OFF
#pragma config ICESEL =     ICS_PGx2
#pragma config PWP =        0xff
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
#pragma config FCANIO =     OFF
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
/**************************************************
 * USB Device Layer Function Driver Registration
 * Table
 **************************************************/
const USB_DEVICE_FUNCTION_REGISTRATION_TABLE funcRegistrationTable[1] =
{
    /* Function 1 */
    {
        .configurationValue = 1,    /* Configuration value */
        .interfaceNumber = 0,       /* First interfaceNumber of this function */
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
const USB_DEVICE_DESCRIPTOR fullSpeedDeviceDescriptor = {
  0x12, // Size of this descriptor in bytes
  USB_DESCRIPTOR_DEVICE, // DEVICE descriptor type
  0x0100, // USB Spec Release Number in BCD format
  0x00, // Class Code
  0x00, // Subclass code
  0x00, // Protocol code
  USB_MAX_PACKET_SIZE, // Max packet size for EP0
  USB_DEVICE_VENDOR_ID, // Vendor ID: 0x04D8 is Microchip's Vendor ID
  USB_DEVICE_PRODUCT_ID, // Product ID: 0x0053
  0x0000, // Device release number in BCD format
  0x01, // Manufacturer string index
  0x02, // Product string index
  0x00, // Device serial number string index
  0x01 // Number of possible configurations
};

/*******************************************
 *  USB Device Configuration Decriptor
 *******************************************/
const uint8_t fullSpeedConfigurationDescriptor1[] = {
  /* Configuration Descriptor Header */
  0x09, // Size of this descriptor in bytes
  USB_DESCRIPTOR_CONFIGURATION, // CONFIGURATION descriptor type
  0x1b, 0x00, // Total length of data for this cfg
  1, // Number of interfaces in this cfg
  1, // Index value of this configuration
  0, // Configuration string index
  USB_ATTRIBUTE_DEFAULT | USB_ATTRIBUTE_SELF_POWERED, // Attributes, see usb_device.h
  50, // Max power consumption (2X mA)

  0x09, // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE, // Descriptor Type
  USB_DFU_INTERFACE_INDEX, // Interface Number
  0x00, // Alternate Setting Number
  0x00, // Number of endpoints in this intf
  0xfe, // Class code
  0x01, // Subclass code
  0x02, // Protocol code
  0x00, // Interface string index

  // DFU functional descriptor
  0x09,  // size
  0x21,  // DFU functional descriptor
  0x01,  // download capable
  0x00, 0x00,  // detatch timeout
  0x10, 0x00,  // transfer size, 16 bytes?
  0x01, 0x10   // Rev 1.1
};

/**************************************
 *  String descriptors.
 *************************************/

/* Language code string descriptor 0 */
const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[1];
}

sd000 = {
  sizeof(sd000),
  USB_DESCRIPTOR_STRING, {
    0x0409
  }
};

/* Manufacturer string descriptor 1*/
const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[21];
}

sd001 = {
  sizeof(sd001),
  USB_DESCRIPTOR_STRING, {
    'O', 'p', 'e', 'n', ' ', 'L', 'i', 'g', 'h', 't', 'i', 'n', 'g', ' ',
    'P', 'r', 'o', 'j', 'e', 'c', 't'
  }
};

/* Product string descriptor */
const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[7];
}

sd002 = {
  sizeof(sd002),
  USB_DESCRIPTOR_STRING, {
    'J', 'a', ' ', 'R', 'u', 'l', 'e'
  }
};

/***************************************
 * Array of string descriptors
 ***************************************/
USB_DEVICE_STRING_DESCRIPTORS_TABLE stringDescriptors[3] = {
  (const uint8_t * const) &sd000,
  (const uint8_t * const) &sd001,
  (const uint8_t * const) &sd002
};

/*******************************************
 * Array of full speed config descriptors
 *******************************************/
USB_DEVICE_CONFIGURATION_DESCRIPTORS_TABLE fullSpeedConfigDescSet[1] = {
  fullSpeedConfigurationDescriptor1
};

/*******************************************
 * USB Device Layer Master Descriptor Table
 *******************************************/
const USB_DEVICE_MASTER_DESCRIPTOR usbMasterDescriptor = {
  &fullSpeedDeviceDescriptor, // Full Speed Device Descriptor.
  1, // Total number of full speed configurations available.
  &fullSpeedConfigDescSet[0], // Pointer to array of full speed configurations descriptors.

  NULL, // High speed device desc is not supported.
  0, // Total number of high speed configurations available.
  NULL, // Pointer to array of high speed configurations descriptors.

  4, // Total number of string descriptors available.
  stringDescriptors, // Pointer to array of string descriptors

  NULL, // Pointer to full speed dev qualifier.
  NULL, // Pointer to high speed dev qualifier.
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
    .registeredFuncCount = 1,

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

    /* Board Support Package Initialization */
    BSP_Initialize();

    /* Initialize Drivers */

    /* Initialize System Services */
    SYS_INT_Initialize();

    /* Initialize Middleware */
    /* Set priority of USB interrupt source */
    SYS_INT_VectorPrioritySet(INT_VECTOR_USB1, INT_PRIORITY_LEVEL4);

    /* Set Sub-priority of USB interrupt source */
    SYS_INT_VectorSubprioritySet(INT_VECTOR_USB1, INT_SUBPRIORITY_LEVEL0);

    /* Initialize the USB device layer */
    sysObj.usbDevObject0 = USB_DEVICE_Initialize (USB_DEVICE_INDEX_0 , ( SYS_MODULE_INIT* ) & usbDevInitData);
    /* Enable Global Interrupts */
    SYS_INT_Enable();

    /* Initialize the Application */
    APP_Initialize();
}

/*******************************************************************************
 End of File
*/

