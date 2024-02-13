
#ifndef OUR_SERVICE_H__
#define OUR_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

// FROM_SERVICE_TUTORIAL: Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_OUR_BASE_UUID              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID
#define BLE_UUID_OUR_SERVICE_UUID                0xDFE0 //  but recognizable value


// Defining 16-bit characteristic UUID
#define CHARACTERISTC_UUID_ERROR        0xEEFD 
#define CHARACTERISTC_UUID_LATEST       0xFEED 
#define CHARACTERISTC_UUID_LOG          0xFFED 


// BLE_WRITE:
// This is used to pass the write handlers for different characteristics from main.c. All of the content will be copied to instance.
typedef void (*ble_os_log_value_write_handler_t) (uint32_t log_value);//(uint32_t log_value);

typedef struct
{
    ble_os_log_value_write_handler_t log_value_write_handler; 
} ble_os_init_t;

// This structure contains various status information for the service. 
typedef struct
{
    uint16_t                    conn_handle;    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t                    service_handle; /**< Handle of Our Service (as provided by the BLE stack). */
    // Handles for the characteristic attributes to struct
		ble_gatts_char_handles_t Char_Error_Handle;
                ble_gatts_char_handles_t Char_Latest_Handle;
                ble_gatts_char_handles_t Char_Log_Handle;
                ble_os_log_value_write_handler_t log_value_write_handler;  /**< Event handler to be called when the log is written. */
    // Add other handlers here...
}ble_os_t;

void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

void our_service_init(ble_os_t * p_our_service, ble_os_init_t * init);

void error_characteristic_update(ble_os_t *p_our_service, int8_t *alert_array);
void current_characteristic_update(ble_os_t *p_our_service, int8_t *current_array);
void log_characteristic_update(ble_os_t *p_our_service, short *data_dump);

#endif  /* _ OUR_SERVICE_H__ */
