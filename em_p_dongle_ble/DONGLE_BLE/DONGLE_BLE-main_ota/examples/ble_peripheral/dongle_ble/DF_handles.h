
#ifndef DF_HANDLES_H__
#define DF_HANDLES_H__

#include <stdint.h>
#include "app_uart.h"
#include "boards.h"
#include "nrf_uart.h"

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


////Global Vars
//extern uint8_t data_array[128];
//extern uint8_t delim_index;
//extern uint8_t data_index;
//extern int8_t cr_get;
//extern int8_t ee_index;;
//extern int8_t ww_index;
//extern int8_t ee_buff;
//extern int8_t ww_buff;
//extern uint8_t M1_index;

//extern uint8_t send_len;
//extern int col_index;
//extern int col_buff;
//extern int shunt_index;
//extern int shunt_buff;
//extern bool log_request_flag;
//extern int send_position_i;
//

/////functions
void uart_event_handle ();
void internal_log_request_enable();
void external_log_request_enable(uint8_t request_id, uint8_t lenth);

//void reset_vars (){
//  data_index = 0;
//  ee_index = 0;
//  ww_index = 0;
//  ee_buff = 0;
//  ww_buff = 0;
//  col_index = 0;
//  col_buff = 0;
//  shunt_buff = 0;
//  shunt_index = 0;
//  send_len = 0;                    
//  for(int i = 0; i<128;i++){data_array[i] = 0;}
//}


static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = UART_PIN_DISCONNECTED,
        .cts_pin_no   = UART_PIN_DISCONNECTED,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = NRF_UART_BAUDRATE_115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}





#endif  /* _ DF_HANDLES_H__ */
