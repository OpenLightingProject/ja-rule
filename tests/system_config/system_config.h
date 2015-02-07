/*
 * This is the system_config.h used for the tests.
 */

#ifndef TESTS_SYSTEM_CONFIG_SYSTEM_CONFIG_H_
#define TESTS_SYSTEM_CONFIG_SYSTEM_CONFIG_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*** USB Driver Configuration ***/

/* Enables Device Support */
#define DRV_USB_DEVICE_SUPPORT      true

/* Enables Device Support */
#define DRV_USB_HOST_SUPPORT        false


/* Maximum USB driver instances */
#define DRV_USB_INSTANCES_NUMBER    1

/* Interrupt mode enabled */
#define DRV_USB_INTERRUPT_MODE      true

/* Number of Endpoints used */
#define DRV_USB_ENDPOINTS_NUMBER    2

/*** USB Device Stack Configuration ***/

/* Maximum device layer instances */
#define USB_DEVICE_INSTANCES_NUMBER     1

/* EP0 size in bytes */
#define USB_DEVICE_EP0_BUFFER_SIZE      64

/* Endpoint Transfer Queue Size combined for Read and write */
#define USB_DEVICE_ENDPOINT_QUEUE_DEPTH_COMBINED    2

#define USB_MAKE_BUFFER_DMA_READY

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_SYSTEM_CONFIG_SYSTEM_CONFIG_H_
