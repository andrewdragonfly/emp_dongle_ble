
#ifndef UART_HANDLER_H__
#define UART_HANDLER_H__

#include "dev_defines.h"
//#include "data_store.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#include "our_service.h"
#include "app_fifo.h"

#include "app_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "boards.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "app_util_platform.h"
#include <string.h>


int8_t buffer_uart_array [80] = {0}; 
int32_t m_v_log [12][5][2] = {0};
int32_t voltage_log [12][2] = {0};
int32_t temperature_log [12][2] = {0};
int32_t load_log [12][2] = {0};
int32_t coul_log [12][2] = {0};
int32_t shunt_log [12][2] = {0};
int32_t charge_log [12][2] = {0};
uint8_t trans_len_i = 0;
bool uart_set = false;
bool charge_set = false;
bool uart_error = false;
int error_code = 0;
int eeww_flag = 0;
int32_t index_time = 0;
int index_step = 0;
//int32_t couls = 0;

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        //APP_ERROR_HANDLER(p_event->data.error_communication);
        
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        //APP_ERROR_HANDLER(p_event->data.error_code);
    }
}






void log_time (){
      index_time ++; 
      index_step = index_time % 12;
      }

void uart_toggle (){// use this when switching
        while(app_uart_close() != NRF_SUCCESS);
        while(app_uart_flush() != NRF_SUCCESS);
        nrf_delay_ms(100);
}

int convert_stream_val (int index_offset){
  int convert_buffer = 0;
 for (int i = 0; i<6; i++){
    convert_buffer = convert_buffer + (buffer_uart_array[i+index_offset+2]-48)*pow(10,(5-i));
    }
  return convert_buffer;
  }

void v_bw_filter (int voltage_log[][2], int filter);//declaration
void t_bw_filter (int temperature_log[][2], int filter);//declaration
void charge_counter (int charge_log[][2]);//declaration
void uhoh_filter (int error_code,int32_t m_v_log[][5][2],int eeww_flag);
void l_bw_filter (int coul_log[][2], int filter, int ch_state);//declaration
void c_bw_filter (int charge_log[][2], int ch_state);//declaration

void bms_uart_builder (ble_os_t *m_our_service){
 int8_t uart_array[80] = {0};
 int8_t uart_error_array[8] = {0};
  int index_of_m1 = 0;
  int index_of_m2 = 0;
  int index_of_m3 = 0;
  int index_of_m4 = 0;
  int index_of_cc = 0;
  int index_of_tt = 0;
  int index_of_sv = 0;
  int index_of_ch = 0;//charge
  int index_of_ee = 0;
  int index_of_ww = 0;
  bool error_flag = false;
  bool discharge_flag = false;
  bool charge_flag = false;
  int ch_state = 0;

  for (int i=0; i<80;i++){
    if (buffer_uart_array[i] == 'M' && buffer_uart_array[i+1] == '1'){index_of_m1 = i;}
    if (buffer_uart_array[i] == 'M' && buffer_uart_array[i+1] == '2'){index_of_m2 = i;}
    if (buffer_uart_array[i] == 'M' && buffer_uart_array[i+1] == '3'){index_of_m3 = i;}
    if (buffer_uart_array[i] == 'M' && buffer_uart_array[i+1] == '4'){index_of_m4 = i;}
    if (buffer_uart_array[i] == 'C' && buffer_uart_array[i+1] == 'C'){index_of_cc = i;}
    if (buffer_uart_array[i] == 'S' && buffer_uart_array[i+1] == 'V'){index_of_sv = i;}
    if (buffer_uart_array[i] == 'T' && buffer_uart_array[i+1] == 'T'){index_of_tt = i;}
    if (buffer_uart_array[i] == 'C' && buffer_uart_array[i+1] == 'M'){index_of_ch = i;
    charge_set = true;
    }
    if (buffer_uart_array[i] == 'D' && buffer_uart_array[i+1] == 'C'){discharge_flag = true;
    ch_state = 1;
    }
    if (buffer_uart_array[i] == 'A' && buffer_uart_array[i+1] == 'C'){charge_flag = true;
    ch_state = 2;
    }
    
    if (buffer_uart_array[i] == 'E' && buffer_uart_array[i+1] == 'E'){
      index_of_ee = i;
      eeww_flag = 1;
      error_code = buffer_uart_array[i+2] + buffer_uart_array[i+3];
      uhoh_filter(error_code,m_v_log,eeww_flag);
      error_flag = true;

      }//end if error
    if (buffer_uart_array[i] == 'W' && buffer_uart_array[i+1] == 'W'){
      index_of_ww = i;
      eeww_flag = 2;
      error_code = buffer_uart_array[i+2] + buffer_uart_array[i+3];
      uhoh_filter(error_code,m_v_log,eeww_flag);
      error_flag = true;
      }//end if warn
      }//end for


      if (error_flag == false){log_time ();}//this is to ++ the time, acknolege that the peroidic was recieved.


      if (index_time > (10*12)){//turn off the led for the bluetooth, 10 min
            err_code = bsp_indication_set(BSP_INDICATE_USER_STATE_OFF);
            APP_ERROR_CHECK(err_code);
      }
 //       voltage_log [index_step][0] = index_time;
 //       voltage_log [index_step][1] = convert_stream_val(index_of_m1) + convert_stream_val(index_of_m2) + convert_stream_val(index_of_m3) + convert_stream_val(index_of_m4);
 //       v_bw_filter (voltage_log, 1);

 //       m_v_log [index_step][0][0] = index_time;
 //       m_v_log [index_step][1][1] = convert_stream_val(index_of_m1);
 //       m_v_log [index_step][2][1] = convert_stream_val(index_of_m2);
 //       m_v_log [index_step][3][1] = convert_stream_val(index_of_m3);
 //       m_v_log [index_step][4][1] = convert_stream_val(index_of_m4);
 //       //delta
 //       int mod_delta [4] = {1};
 //       for (int i = 0; i < 3; i++){
 //       mod_delta [i] = abs((voltage_log [index_step][1] / 4) - m_v_log [index_step][i+1][1]);
 //       }
 //        for (int i = 1; i<4; i++){
 //        if(mod_delta [0] < mod_delta [i]){
 //         mod_delta [0] = mod_delta [i];
 //         }
 //       }
 //       m_v_log [index_step][0][1] = mod_delta [0];

 //       //load
 //       coul_log [index_step][0] = index_time;
 //       //if (charge_flag == true){couls =  couls - convert_stream_val(index_of_cc);}
 //       //if (discharge_flag == true){couls = couls + convert_stream_val(index_of_cc);}
 //       //if (couls < 0) {couls = 0;}
 //       //coul_log [index_step][1] = couls;
 //       coul_log [index_step][1] = convert_stream_val(index_of_cc);
 //       l_bw_filter (coul_log, 5,ch_state);

 //       //shunt_log [index_step][0] = index_time;
 //       //shunt_log [index_step][1] = convert_stream_val(index_of_sv);
        
 //       //temperature
 //       temperature_log [index_step][0] = index_time;
 //       temperature_log [index_step][1] = convert_stream_val(index_of_tt);
 //       t_bw_filter (temperature_log, 10);

 //       charge_log [index_step][0] = index_time;
 //       if (charge_flag == true){charge_log [index_step][1] = 2;} 
 //       if (discharge_flag == true){charge_log [index_step][1] = 1;}
         
 //       c_bw_filter (charge_log, ch_state);

        
      
 //   for (int i = 0; i<8; i++){
 //     if (error_flag == false && uart_array != 0){
 //       uart_array [2  + i] = buffer_uart_array [i + index_of_m1];
 //       uart_array [10 + i] = buffer_uart_array [i + index_of_m2];
 //       uart_array [18 + i] = buffer_uart_array [i + index_of_m3];
 //       uart_array [26 + i] = buffer_uart_array [i + index_of_m4];
 //       uart_array [34 + i] = buffer_uart_array [i + index_of_cc];
 //       uart_array [42 + i] = buffer_uart_array [i + index_of_sv];
 //       uart_array [50 + i] = buffer_uart_array [i + index_of_tt];
 //       if (index_of_ch > 0){uart_array [58 + i] = buffer_uart_array [i + index_of_ch];}

 //       if (i == 7){
 //       uart_array [0] = DEV_HANDLE[0];//move this to be a feature of the charactoristic?
 //       uart_array [1] = DEV_HANDLE[1];//move this to be a feature of the charactoristic?
 //         current_characteristic_update(m_our_service, uart_array);
 //       for (int i = 0; i<80; i++){uart_array [i] = 0;}
 //       for (int i = 0; i<100; i++){buffer_uart_array [i] = 0;}
 //       trans_len_i = 0;
 //       }//end if last loop
 //     }//end period send
 //     if (error_flag == true){
 //       uart_error_array [2 + i] = buffer_uart_array [i + index_of_ee];
 //       //uart_error_array [10 + i] = buffer_uart_array [i + index_of_ww];
 //       if (i == 7){
 //       uart_error_array [0] = DEV_HANDLE[0];//move this to be a feature of the charactoristic?
 //       uart_error_array [1] = DEV_HANDLE[1];//move this to be a feature of the charactoristic?
 //       error_characteristic_update(m_our_service, uart_error_array);
 //       for (int i = 0; i<8; i++){uart_error_array [i] = 0;}
 //       for (int i = 0; i<100; i++){buffer_uart_array [i] = 0;}
 //       eeww_flag = 0;
 //       trans_len_i = 0;
 //       }//end error if last loop
 //       }//end error true
 //     }//end for
      }//end fun

void bms_uart_get (app_uart_evt_t * p_event){
uart_set = false;

uint32_t       err_code;
switch (p_event->evt_type)
    {
case APP_UART_DATA_READY:
           UNUSED_VARIABLE(app_uart_get(&buffer_uart_array[trans_len_i]));
           trans_len_i++;

 if (buffer_uart_array[trans_len_i - 1] == '\r')
             {
                if (trans_len_i > 1)
                {
                    //NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                    NRF_LOG_HEXDUMP_DEBUG(buffer_uart_array, trans_len_i);

                    
                    do
                    {
                        uint16_t length = (uint16_t)trans_len_i;
                        if ((err_code != NRF_ERROR_INVALID_STATE) &&
                            (err_code != NRF_ERROR_RESOURCES) &&
                            (err_code != NRF_ERROR_NOT_FOUND))
                        {
                            APP_ERROR_CHECK(err_code);
                        }
                    } while (err_code == NRF_ERROR_RESOURCES);
                }

               
            }
            uart_set = true;
            if (buffer_uart_array[trans_len_i - 7] == 'E' || buffer_uart_array[trans_len_i - 7] == 'W'){
            uart_error = true;
            break;
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


 void uart_send_array (ble_os_t *m_our_service){
 while(app_uart_flush() != NRF_SUCCESS);
  if (uart_error == true || (uart_set == true && trans_len_i>=80)){
  bms_uart_builder(m_our_service);
  uart_set = false;
  charge_set = false;
  uart_error = false;
  trans_len_i = 0;
  }

  }


#endif  /* _ UART_HANDLER_H__ */
