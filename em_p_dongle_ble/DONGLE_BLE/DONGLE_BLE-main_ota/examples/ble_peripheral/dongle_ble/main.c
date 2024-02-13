//dongle 2 360
//OTA DFU enabled


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "DF_ble_nus.h"
#include "DF_handles.h"

#include "nrf_delay.h"

#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"

// BEGIN Block Added for DFU  
#include "nrf_dfu_ble_svci_bond_sharing.h" 
#include "nrf_svci_async_function.h" 
#include "nrf_svci_async_handler.h" 
#include "ble_dfu.h" 
#include "nrf_bootloader_info.h"  
// END Block Added for DFU 

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "ble_store.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                0                                           /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};


// Handling DFU events  
static void ble_dfu_buttonless_evt_handler(ble_dfu_buttonless_evt_type_t event) {
     
  switch (event){ 
          
    case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:             
      NRF_LOG_INFO("Device is preparing to enter bootloader mode\r\n");             
      break;           
    case BLE_DFU_EVT_BOOTLOADER_ENTER:            
      NRF_LOG_INFO("Device will enter bootloader mode\r\n");             
      break;           
    case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:             
      NRF_LOG_ERROR("Device failed to enter bootloader mode\r\n");             
      break;         
    default:             
      NRF_LOG_INFO("Unknown event from ble_dfu.\r\n");             
      break;     
      
  } 

} 

// Handling bootloader power management events  
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event) {     
  switch (event)     {         
    case NRF_PWR_MGMT_EVT_PREPARE_DFU:             
      NRF_LOG_INFO("Power management wants to reset to DFU mode\r\n");              
      // Change this code to tailor to your reset strategy.             
      // Returning false here means that the device is not ready              
      // to jump to DFU mode yet.             
      //              
      // Here is an example using a variable to delay resetting the device:             
      //             
      /* if (!im_ready_for_reset){
         return false;                
      }             
      */             
      break; 

    default:             
     // Implement any of the other events available              
     // from the power management module:             
     // -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF             
     // -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP             
     // -NRF_PWR_MGMT_EVT_PREPARE_RESET             
     return true;     
  }     

  NRF_LOG_INFO("Power management allowed to reset to DFU mode\r\n");     
  return true; 

}  
 
 NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0); // persistent register for determining DFU status on startup 


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) battery_def.serial,
                                          strlen(battery_def.serial));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        nrf_delay_ms(100);
        uint32_t err_code;

        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
            
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);



                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);

        }
        //app_uart_put(0xFF);
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
        {
            while (app_uart_put('\n') == NRF_ERROR_BUSY);
        }
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);

    // BEGIN Block Added for DFU 
    // ONLY ADD THIS BLOCK TO THE EXISTING FUNCTION     
    // Initialize the DFU service
         
    ble_dfu_buttonless_init_t dfus_init = {   
          
      .evt_handler = ble_dfu_buttonless_evt_handler  
         
    };     

    err_code = ble_dfu_buttonless_init(&dfus_init);     
    APP_ERROR_CHECK(err_code); 
    
    // END Block Added for DFU 
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
uint8_t send_len = 0;
int col_index = 0;
int col_buff = 0;
int shunt_index = 0;
int shunt_buff = 0;
bool log_request_flag = false;
int send_position_i = 0;

 int8_t calibration_array [256] = {0};
 int8_t chunk_array [16] = {0};

void log_request_chunker (int8_t id_0, int8_t id_1, int8_t *data, int8_t chunk_len){
  calibration_array[send_position_i] = id_0;
  calibration_array[send_position_i + 1] = id_1;
  calibration_array[send_position_i + 2 + chunk_len] = '\r';

for (int i = 0; i<chunk_len; i++){
  calibration_array [send_position_i + 2 + i] = data[i];
  }
  send_position_i += (chunk_len + 3);
  }


uint8_t data_array[128] = {0};
uint8_t delim_index = 0;
uint8_t data_index = 0;
int8_t cr_get = 0;
int8_t ee_index = 0;
int8_t ww_index = 0;
int8_t ee_buff = 0;
int8_t ww_buff = 0;
uint8_t M1_index = 0;



void reset_vars (){
  data_index = 0;
  ee_index = 0;
  ww_index = 0;
  ee_buff = 0;
  ww_buff = 0;
  col_index = 0;
  col_buff = 0;
  shunt_buff = 0;
  shunt_index = 0;
  send_len = 0;                    
  for(int i = 0; i<128;i++){data_array[i] = 0;}
}



uint8_t request_len = 0;

void external_log_request_enable (uint8_t request_id, uint8_t lenth){
  log_request_flag = true;
  reset_vars();
  //data_index = 1;
  
  request_len = lenth;

}

void interupt_wake (int time){//this should have an out of loop if it gets called and cant break
  //app_uart_put(0xFF);
  
  char uart_get = 0;
  do{
  while(app_uart_get(&uart_get) != NRF_SUCCESS);
  NRF_LOG_INFO("recvd uart: %c",uart_get)
  }
  while (uart_get != 0);
  nrf_delay_ms(time);
  app_uart_put(0xDF);
  app_uart_put(0x0);
  app_uart_put(0x0);
  app_uart_put(0x0);
  app_uart_put(0xD);
}

void internal_log_request_enable (){// the number of chars may not make sense. should it be 6 or 8 depends on data input. struct allows for 8

  for(int mod_no = 1; mod_no==8; mod_no++){
    log_request_chunker('C',(mod_no + 0x30),battery_def.mod_cap[mod_no],6);
  }
  for(int mod_no = 1; mod_no==8; mod_no++){
    log_request_chunker('C',(mod_no + 0x30),battery_def.mod_volt[mod_no],6);
  }

  log_request_chunker('C','C',battery_def.cc_coef,6);
  log_request_chunker('S','V',battery_def.sv_coef,6);


  uint16_t length = (uint16_t)send_position_i;
    
    ble_nus_data_send_rx(&m_nus, calibration_array, &length, m_conn_handle); //this is the log char
for (int i = 0; i<256; i++){calibration_array [i] = 0;}
  send_position_i = 0;
}

void uart_event_handle(app_uart_evt_t * p_event){
  uint32_t       err_code;
  switch (p_event->evt_type){
    case APP_UART_DATA_READY:
       app_uart_get(&cr_get);
      
       data_array[data_index] = cr_get; 

        if(data_array[data_index] == 'C' && data_array[data_index - 1] == 'C'){col_index = data_index; }  
        if (col_index != 0 && data_index == (col_index + 6)){for (int i = 0; i<6; i++){ col_buff = col_buff + (data_array[data_index - i]-48);}}
        if(data_array[data_index] == 'V' && data_array[data_index - 1] == 'S'){shunt_index = data_index;}
        if (shunt_index != 0 && data_index == (shunt_index + 6)){for (int i = 0; i<6; i++){shunt_buff = shunt_buff + (data_array[data_index - i]-48);}}   
        
        if(data_array[data_index] == 'E' && data_array[data_index - 1] == 'E'){ee_index = data_index;}
        if (ee_index != 0 && data_index == (ee_index + 6)){for (int i = 0; i<6; i++){ee_buff = ee_buff + (data_array[data_index - i]-48);}}  
  
        if(data_array[data_index] == 'W' && data_array[data_index - 1] == 'W'){ee_index = data_index;}
        if (ww_index != 0 && data_index == (ww_index + 6)){for (int i = 0; i<6; i++){ww_buff = ww_buff + (data_array[data_index - i]-48);}}   
  
        send_len = 62;


     if(data_array[data_index] == '1' && data_array[data_index - 1] == 'M'){//fix next send
        data_index = 1;
        send_len = 62;
          
        }

        
  
        if(col_buff > 0 || shunt_buff > 0){
          send_len = 72;
        } 

        if (ee_buff > 0 || ww_buff > 0){
          send_len = 8;
        }
   
        if (log_request_flag == true){
          send_len = request_len*4;
          //data_array[send_len-1] = '\r';
          }
   
               if (data_index == send_len){
                   NRF_LOG_HEXDUMP_DEBUG(data_array, data_index);

                    do
                    {                        
                        uint16_t length = (uint16_t)data_index;
                        if(log_request_flag == false){
                        err_code = ble_nus_data_send(&m_nus, data_array, &length, m_conn_handle);//this is the periodic char
                        reset_vars();
                        }
                        else {
                        log_request_flag = false;
                        err_code = ble_nus_data_send_rx(&m_nus, data_array, &length, m_conn_handle); //this is the log char
                        reset_vars();
                        }
                        if ((err_code != NRF_ERROR_INVALID_STATE) &&
                            (err_code != NRF_ERROR_RESOURCES) &&
                            (err_code != NRF_ERROR_NOT_FOUND))
                        {
                            APP_ERROR_CHECK(err_code);
                        }
                    } while (err_code == NRF_ERROR_RESOURCES);
                              
            }
            else{

            data_index++;
            
           }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}


char buffer [128];
int buff_index = 0;

void apend_buff (char data[], int size){

  for (int i = 0; i < size; i++){
    if(data[i] != 0){
      buffer [buff_index] = data[i];
    }
    else {  
      buffer [buff_index] = '\r';
    }
    buff_index ++;
  }
  
}

void send_dev_defines (){
  apend_buff(battery_def.serial,sizeof(battery_def.serial));
  apend_buff(battery_def.date,sizeof(battery_def.date));
  //hwv (hardware version) and fwv(firmware version) _s is the string version of this that has the '.' included. if the other format is wanted use the other stuct
  apend_buff(battery_def.hwv_s,sizeof(battery_def.hwv_s));
  apend_buff(battery_def.fwv_s,sizeof(battery_def.fwv_s));
  uint16_t length = buff_index;
  ble_nus_data_send_rx(&m_nus,buffer,&length,m_conn_handle);
  buff_index = 0;
}

void send_dev_defines_uart (uint8_t data_enum, uint8_t sub_enum);

void send_all_defines(){
  int time = 100; //ms

  //wake up as 'new' command every call
  //send_dev_defines_uart (1, 0);
  //    app_uart_put(0XDF);
  //    app_uart_put(0XDF);
  //nrf_delay_ms(400);
  //interupt_wake(time);

  //send_dev_defines_uart (2, 0);

  
  app_uart_put(0xDF);
  app_uart_put(0x0);
  app_uart_put(0x0);
  app_uart_put(0x0);
  app_uart_put(0xD);

  send_dev_defines_uart (1, 0);
  interupt_wake(time);
  send_dev_defines_uart (2, 0);
  interupt_wake(time);
  send_dev_defines_uart (3, 0);
  //interupt_wake(time);
  //for(int i = 1; i < 9; i++){
  //  send_dev_defines_uart (5, i);
  //  interupt_wake(time);
  //  }
  //for(int i = 1; i < 9; i++){
  //  send_dev_defines_uart (6, i);
  //  interupt_wake(time);
  //  }
  //send_dev_defines_uart (7, 0);
  //interupt_wake(time);
  //send_dev_defines_uart (8, 0);
}

void send_dev_defines_uart (uint8_t data_enum, uint8_t sub_enum){
  app_uart_put(0XDF);
  app_uart_put(data_enum);
  switch (data_enum){
    case 0xF:
      apend_buff(battery_def.serial,sizeof(battery_def.serial));
      break;    
    case 0:
      send_all_defines();
      break;
    case 1:
      apend_buff(battery_def.serial,sizeof(battery_def.serial));
    break;

    case 2:
      apend_buff(battery_def.date,sizeof(battery_def.date));
    break;

    case 3:
      apend_buff(battery_def.hwv_s,sizeof(battery_def.hwv_s));
    break;

    case 4:
      apend_buff(battery_def.fwv_s,sizeof(battery_def.fwv_s));
    break;

    case 5://voltage, I made the size the same for simplicity
      apend_buff(battery_def.mod_volt[sub_enum],sizeof(8));
    break;

    case 6: //cap
    apend_buff(battery_def.mod_cap[sub_enum],sizeof(8));
    
    case 7:
     apend_buff(battery_def.cc_coef,sizeof(battery_def.cc_coef));
    break;

    case 8:
     apend_buff(battery_def.sv_coef,sizeof(battery_def.sv_coef));
    break;
  }

  uint16_t length = buff_index;
  for(int i = 0; i<buff_index; i++){
    app_uart_put(buffer[i]);
    NRF_LOG_INFO("%c",buffer[i]);
  }
  NRF_LOG_FLUSH();
  buff_index = 0;
  app_uart_put(0xFF);

}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}


/**@brief Application main function.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    
    get_all_defs();//from mem
    uart_init();
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();

    // Start execution.
    printf("\r\n Debug started.\r\n");
    advertising_start();

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
       
    }
}



/**
 * @}
 */