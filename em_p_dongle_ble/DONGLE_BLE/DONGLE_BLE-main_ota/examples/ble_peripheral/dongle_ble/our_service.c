
#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "our_service.h"
#include "ble_srv_common.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"



// BLE_WRITE:
/**@brief Function for handling the Write event.
 *
 * @param[in] p_our_service     Our Service structure.
 * @param[in] p_ble_evt         Event received from the BLE stack.
 */
static void on_write1(ble_os_t * p_our_service, ble_evt_t const * p_ble_evt)
{
    //NRF_LOG_INFO("on_write: called");
    
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    

    if ((p_evt_write->handle == p_our_service->Char_Log_Handle.value_handle))
        {
        NRF_LOG_INFO("Write on the charactoristic!");
        
       
        int8_t len = p_evt_write->len;
        if (len != 13)
        {
            NRF_LOG_INFO("ERROR: incomplete package");
            NRF_LOG_INFO("len: %d", len);
            return;
        }
            
        int test_val [13] = {0};

        for (int i = 0; i<13; i++){
           test_val [i] = p_evt_write->data[i];
        }
        


        // Call the write handler function. Implementation is in the main.
        p_our_service->log_value_write_handler(test_val);
       
    }

}

// Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    // Switch case handling BLE events related to our service. 
		ble_os_t * p_our_service =(ble_os_t *) p_context;
		switch (p_ble_evt->header.evt_id)
{
    case BLE_GAP_EVT_CONNECTED:
        p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
        break;

        // BLE_WRITE:
        // Write: Data was received to the module
        case BLE_GATTS_EVT_WRITE:
            on_write1(p_our_service, p_ble_evt);
            break;

    default:
        // No implementation needed.
        break;
}
}


//Error Char
static uint32_t char_add_Error(ble_os_t * p_our_service)
{
   // Characteristic UUID
		uint32_t            err_code;
		ble_uuid_t          char_uuid;
		ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
		char_uuid.uuid      = CHARACTERISTC_UUID_ERROR;
		err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
		APP_ERROR_CHECK(err_code);  
    
    //Read/write properties
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read = 0;
		char_md.char_props.write = 0;

    
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
		cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
		char_md.p_cccd_md           = &cccd_md;
		char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));  
		attr_md.vloc        = BLE_GATTS_VLOC_STACK;
    
        // Read/write security levels characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
		attr_char_value.p_uuid      = &char_uuid;
		attr_char_value.p_attr_md   = &attr_md;
    
    // Set characteristic length in number of bytes
		attr_char_value.max_len     = 32;
		attr_char_value.init_len    = 32;
		uint8_t value[32]            = {0x12,0x34,0x56,0x78};
		attr_char_value.p_value     = value;

    // Add characteristic to the service
		err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->Char_Error_Handle);
		APP_ERROR_CHECK(err_code);
    return NRF_SUCCESS;
}


//latest char
static uint32_t char_add_Latest(ble_os_t * p_our_service)
{
   // Characteristic UUID
		uint32_t            err_code;
		ble_uuid_t          char_uuid;
		ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
		char_uuid.uuid      = CHARACTERISTC_UUID_LATEST;
		err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
		APP_ERROR_CHECK(err_code);  
    
    //Read/write properties
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read = 0;
		char_md.char_props.write = 0;

    
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
		cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
		char_md.p_cccd_md           = &cccd_md;
		char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));  
		attr_md.vloc        = BLE_GATTS_VLOC_STACK;
    
        // Read/write security levels characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
		attr_char_value.p_uuid      = &char_uuid;
		attr_char_value.p_attr_md   = &attr_md;
    
    // Set characteristic length in number of bytes
		attr_char_value.max_len     = 64;
		attr_char_value.init_len    = 64;
		uint8_t value[64]            = {0x12,0x34,0x56,0x78};
		attr_char_value.p_value     = value;

    // Add characteristic to the service
		err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->Char_Latest_Handle);
		APP_ERROR_CHECK(err_code);
    return NRF_SUCCESS;
}
//Log Char
static uint32_t char_add_Log(ble_os_t * p_our_service)
{
    // Characteristic UUID
		uint32_t            err_code;
		ble_uuid_t          char_uuid;
		ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
		char_uuid.uuid      = CHARACTERISTC_UUID_LOG;
		err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
		APP_ERROR_CHECK(err_code);  
    
    // Read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read = 1;
		char_md.char_props.write = 1;

    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
		cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
		char_md.p_cccd_md           = &cccd_md;
		char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));  
		attr_md.vloc        = BLE_GATTS_VLOC_STACK;
    
    // Read/write security levels characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
		attr_char_value.p_uuid      = &char_uuid;
		attr_char_value.p_attr_md   = &attr_md;
    
    // Set characteristic length in number of bytes
		attr_char_value.max_len     = 128;//4
		attr_char_value.init_len    = 128;//4
		uint8_t value[128]            = {0x00};
		attr_char_value.p_value     = value;

    // Add characteristic to the service
		err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->Char_Log_Handle);
		APP_ERROR_CHECK(err_code);
    return NRF_SUCCESS;
}

void our_service_init(ble_os_t * p_our_service, ble_os_init_t * init)//void our_service_init(ble_os_t * p_our_service)
{
    uint32_t   err_code; // Variable to hold return codes from library and softdevice functions

    // Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_UUID_OUR_BASE_UUID;
    service_uuid.uuid = BLE_UUID_OUR_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    
    
    // Sett Connection handle to default value. It is an invalid handle since we are not yet in a connection.
		  // BLE_WRITE: transfer the pointers from the init instance to the module instance
                 p_our_service->log_value_write_handler = init->log_value_write_handler;//p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;

		err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_our_service->service_handle);
    
    APP_ERROR_CHECK(err_code);

    char_add_Error(p_our_service);
    char_add_Latest(p_our_service);
    char_add_Log(p_our_service);
}

//Charactoristic 1 (Error)
void error_characteristic_update(ble_os_t *p_our_service, int8_t *alert_array)
{
		if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
		{
				uint16_t               len = 32;
				ble_gatts_hvx_params_t hvx_params;
				memset(&hvx_params, 0, sizeof(hvx_params));

				hvx_params.handle = p_our_service->Char_Error_Handle.value_handle;
				hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
				hvx_params.offset = 0;
				hvx_params.p_len  = &len;
				hvx_params.p_data = (uint8_t*)alert_array;  

				sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
		}

}
//Charactoristic 2 (latest)
void current_characteristic_update(ble_os_t *p_our_service, int8_t *current_array)
{
		if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
		{
                
				uint16_t               len = 64;
				ble_gatts_hvx_params_t hvx_params;
				memset(&hvx_params, 0, sizeof(hvx_params));

				hvx_params.handle = p_our_service->Char_Latest_Handle.value_handle;
				hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
				hvx_params.offset = 0;
				hvx_params.p_len  = &len;
				hvx_params.p_data = (uint8_t*)current_array;  

				sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
		}

}

//Charactoristic 3 (log)
void log_characteristic_update(ble_os_t *p_our_service,short *data_dump)
{
		if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
		{
				uint16_t               len = 128;
				ble_gatts_hvx_params_t hvx_params;
				memset(&hvx_params, 0, sizeof(hvx_params));

				hvx_params.handle = p_our_service->Char_Log_Handle.value_handle;
				hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
				hvx_params.offset = 0;
				hvx_params.p_len  = &len;
				hvx_params.p_data = (uint8_t*)data_dump;  
                           
				sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
		}
                for (int i = 0; i < 64; i++){data_dump[i] = 0;}

}



