
#ifndef DATA_STORE_H__
#define DATA_STORE_H__


#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "math.h"
#include "uart_handler.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include <stdbool.h>
#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
#include "app_error.h"
#include "nrf_nvmc.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "boards.h"


#include "nrf_log_default_backends.h"

#include "app_timer.h"
#include "nrf_drv_clock.h"

//#include "nrf_cli.h"
//#include "nrf_cli_uart.h"


#include "nrf_sdh_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"

#include <string.h>


#include "nrf_soc.h"


#include "app_util.h"
#include "nrf_fstorage.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_fstorage_sd.h"
#else
#include "nrf_drv_clock.h"
#include "nrf_fstorage_nvmc.h"
#endif



//weekly log
struct{
  uint32_t time;
  uint16_t value;
  } load_log_index_p [1000],temperature_log_index_p [1000],voltage_log_index_p [1000],DoD_log_index_p [1000],charge_log_index_p [1000];

struct{
  uint32_t uhoh_time;
  uint8_t uhoh_type;
  uint8_t uhoh_message;
  uint16_t uhoh_value;
  } uhoh_index [1000];

int slope_buffer = 0;
int16_t hist_CC = 0;
uint32_t DoD_log_p_i = 0;
int32_t voltage_log_p_i = 0;
int32_t temperature_log_p_i = 0;
int32_t load_log_p_i = 0;
int32_t uhoh_p_i = 0;
int32_t charge_log_p_i = 0;
int32_t couls = 0;
int index_step_hist = 0;
int16_t number_of_complete_discharges_p = 0;
int32_t total_DoD = 0; //number of couls discharged

uint8_t    data_read[256] = {0};



void prev_step (){
  //index step convert
  if (index_step != 0){index_step_hist = index_step - 1;}else{index_step_hist = 11;}
  //
}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    .evt_handler = fstorage_evt_handler,
    .start_addr = 0x27000,
    .end_addr   = 0xfffff,
};

char     m_hello_world[] = "hello world";
int iterator = 1;

static uint32_t nrf5_flash_end_addr_get(){
    uint32_t const bootloader_addr = BOOTLOADER_ADDRESS;
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz         = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ?
            bootloader_addr : (code_sz * page_sz));
}


//#ifdef SOFTDEVICE_PRESENT
//static void ble_stack_init(void)
//{
//    ret_code_t rc;
//    uint32_t   ram_start;

//    rc = nrf_sdh_enable_request();
//    APP_ERROR_CHECK(rc);

//    rc = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
//    APP_ERROR_CHECK(rc);

//    rc = nrf_sdh_ble_enable(&ram_start);
//    APP_ERROR_CHECK(rc);
//}
//#else
//static void clock_init(void){
//    ret_code_t rc = nrf_drv_clock_init();
//    APP_ERROR_CHECK(rc);
//    nrf_drv_clock_lfclk_request(NULL);
//    while (!nrf_clock_lf_is_running()) {;}
//}
//#endif


static void timer_init(void){
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}



static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt){
    if (p_evt->result != NRF_SUCCESS)
    {return;}

    switch (p_evt->id){
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {p_evt->len, p_evt->addr;} break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {p_evt->len, p_evt->addr;} break;

        default:
            break;
    }
}

void new_thing_to_write (){
ret_code_t rc;
switch (iterator){
case 1:
m_hello_world [0] = 'H';
break;

case 2:
m_hello_world [0] = 'J';
break;

default:
break;
}
nrf_fstorage_api_t * p_fs_api;

#ifdef SOFTDEVICE_PRESENT
    p_fs_api = &nrf_fstorage_sd;
#else
    p_fs_api = &nrf_fstorage_nvmc;
#endif

    rc = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
    APP_ERROR_CHECK(rc);
nrf_fstorage_erase(&fstorage,0x28000,1,NULL);
    rc = nrf_fstorage_write(&fstorage, 0x28000, m_hello_world, sizeof(m_hello_world), NULL);
    APP_ERROR_CHECK(rc);

    //nrf_delay_ms(1000);
    nrf_sdh_resume();
    if (iterator>1){iterator = 1;}else{iterator ++;}
}

int ble_stack_resolver(int *writeVal)
{
//    // disable the BLE stack

    nrf_sdh_suspend();



//    ret_code_t rc;

//#ifndef SOFTDEVICE_PRESENT
//    clock_init();
//#endif

//    //timer_init();

//    nrf_fstorage_api_t * p_fs_api;

//#ifdef SOFTDEVICE_PRESENT
//    p_fs_api = &nrf_fstorage_sd;
//#else
//    p_fs_api = &nrf_fstorage_nvmc;
//#endif

    //rc = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
    //APP_ERROR_CHECK(rc);

    //(void) nrf5_flash_end_addr_get();

     uint8_t    data_read[256] = {0};

    //for (;;)
    //{
        new_thing_to_write ();
      //nrf_fstorage_read(&fstorage,0x3f000,data_read,12);
      nrf_sdh_resume();
//if (data_read[0] == 'J'){
//nrf_gpio_pin_toggle(LED_1);
//}else{}
//if (data_read[0] == 'H'){
//nrf_gpio_pin_toggle(LED_3);
//}else{}

    //}
}




//void ble_stack_resolver (int *writeVal){

     

//}


//void data_shifter (int *data_stream){
//for (int i = 0; i>12; i++){data_stream[i+1]=data_stream[i];}
//data_stream [0]=0;
//}

//void NV_DoD_handler (int *DoD_Val)//break this by funtion
//{
////format : //total discharges, time 1, discharge 1, time 2, discharge 2, time n, dicharge n
//uint32_t buffer0 = 0;
//buffer0 =*p_DoD_addr + 1;//in this case buffer is the total number of errors, represents the address too!
//     nrf_nvmc_write_word((buffer0 * 2 + 1),*DoD_Val);//this is a new location so doesnt need a mem clear
//     nrf_nvmc_write_word((buffer0 * 2 + 2),*p_DoD_addr);//this is a new location so doesnt need a mem clear
//     //nrf_nvmc_page_partial_erase_start()//need to figure out partial erase to write a new value for the total
//}

//void NV_error_handler (int *error_Val)//break this by funtion
//{
////format : total number, time 1, error 1, time 2, error 2, time n, error n
//uint32_t buffer0 = 0;
//buffer0 = *p_Error_addr + 1;//in this case buffer is the total number of errors, represents the address too!
//     nrf_nvmc_write_word((buffer0 * 2 + 1),*error_Val);//this is a new location so doesnt need a mem clear
//     nrf_nvmc_write_word((buffer0  *2 + 2),*p_Time_addr);//this is a new location so doesnt need a mem clear
//     //nrf_nvmc_page_partial_erase_start()//need to figure out partial erase to write a new value for the total
//}

//void NV_warning_handler (int *warning_Val)//break this by funtion
//{
////format : total number, time 1, warning 1, time 2, warning 2, time n, warning n
//uint32_t buffer0 = 0;
//buffer0 = *p_Warning_addr + 1;//in this case buffer is the total number of errors, represents the address too!
//     nrf_nvmc_write_word((buffer0 * 2 + 1),*warning_Val);//this is a new location so doesnt need a mem clear
//     nrf_nvmc_write_word((buffer0 * 2 + 2),*p_Time_addr);//this is a new location so doesnt need a mem clear
//     //nrf_nvmc_page_partial_erase_start()//need to figure out partial erase to write a new value for the total
//}

//void NV_time_handler (bool time_sync)//bool synced time
//{
////format : total number of periods lifetime, week period, sync time
//int time_reset_mod = 12*60*24*7; // 12 (periods per min) * 60 (per hr) * 24 (per day) * 7 (1 week)
//uint32_t buffer0 = 0;
//    buffer0 = *p_Time_addr + 1;
//     //nrf_nvmc_page_erase(Time_addr);
//     Time_Store[0] = buffer0;
//     Time_Store[1] = buffer0 % time_reset_mod;//
//     if (time_sync == true){
//      //Time_Store[2] = //not sure what I want to do here yet
//     }
//     else {
//     Time_Store[2] = Time_Store[1];
//     }
//     //nrf_nvmc_write_words(Time_addr, Time_Store, 3);
//     NRF_LOG_INFO("Time = %d", *(p_Time_addr));
     
//}

//int bw_filter (int filter,int *data_stream, int32_t iterator){//filter is the decider, data stream is the data to handle

////if the slope of the data is unchanged, do not log information. slope is calculated from the change in all values of the data_stream/the time constant
////if the slope has changed log the values above the average filter
////the data stored is the time stamp and the value

//uint32_t buffer0 = 0;
//int data =  0;
//int data_store [2] = {0};// this is the data that is added to the data array, 0 = time, 1 is the data

//for (int i = 0;i >= 12; i++){int data = data + data_stream[i];}//sum of the data stream
//data = data / 12; //this is the average
//  for (int i = 0; i >= 12; i++){
//  int slope =  abs(data_stream [12] - data_stream [0])/12; //slope of the stream
//    if (abs(slope_buffer - slope) <= filter){
//    slope_buffer = slope;
//      if (abs(data_stream[i]-data) > filter){
//      iterator ++;
//      data_store [0] = *(p_Time_addr + 1) - i;
//      data_store [1] = data_stream [i];
//}//end if
//}//end if
//}//end for
////return *data_store;
//return iterator;
//}

void v_bw_filter (int voltage_log[][2], int filter){
int data = 0;
int delta = 0;
for (int i = 0; i < 12; i++){ //sum
data = data + voltage_log[i][1];
}
data = data / 12; //ave
delta = abs(voltage_log[index_step][1] - data);
if (delta>filter){
voltage_log_p_i ++;
voltage_log_index_p [voltage_log_p_i].value = voltage_log[index_step][1];
voltage_log_index_p [voltage_log_p_i].time = index_time;
}
else {}
}


void t_bw_filter (int temperature_log[][2], int filter){
  int data = 0;
  int delta = 0;
  for (int i = 0; i < 12; i++){ //sum
    data = data + temperature_log[i][1];
    }
  data = data / 12; //ave
  delta = abs(temperature_log[index_step][1] - data);
  if (delta>filter && load_log[index_step][1]<3000){
    temperature_log_p_i ++;
    temperature_log_index_p [temperature_log_p_i].value = temperature_log[index_step][1];
    temperature_log_index_p [temperature_log_p_i].time = index_time;
    }
  else {}
  }



void l_bw_filter (int coul_log[][2], int filter, int ch_state){
  int data = 0;
  int delta = 0;
  if (ch_state == 1){couls = couls + coul_log[index_step][1];}
  if (ch_state == 2){couls = couls - coul_log[index_step][1];}
   if (couls < 0){couls = 0;}
  for (int i = 0; i < 12; i++){ //sum
  data = data + coul_log[i][1];
  }
  data = data / 12; //ave
  delta = abs(coul_log[index_step][1] - data);
  if (delta>filter && coul_log[index_step][1] != coul_log[index_step_hist][1]){
  load_log_p_i ++;
  load_log_index_p [load_log_p_i].value = couls;
  load_log_index_p [load_log_p_i].time = index_time;
  }
  else {}
  //
  }


void c_bw_filter (int charge_log[12][2], int ch_state){
prev_step();
  //if (charge_log_index_p[index_step - 1].value != charge_log_index_p[index_step].value){
  if(charge_log[index_step_hist][1] != charge_log[index_step][1]){
    charge_log_p_i ++;
    charge_log_index_p [charge_log_p_i].value = ch_state;
    charge_log_index_p [charge_log_p_i].time = index_time;
    }

  //calc number of discharges and DOD
  int total_c = CAPACITY * COULOMB_CONV * 100;

  if (charge_log[index_step_hist][1] != 1 && charge_log[index_step][1] == 1){
   //if (ch_state == 1){
    hist_CC = couls;
    //nrf_gpio_pin_toggle(LED_2);
    }else{//nrf_gpio_pin_toggle(LED_3)
    ;}

//if (ch_state == 2){
  if (charge_log[index_step_hist][1] != 2 && charge_log[index_step][1] == 2){
    total_DoD = total_DoD + (couls - hist_CC); 
    DoD_log_p_i ++;
    DoD_log_index_p [DoD_log_p_i].time = index_time;
    DoD_log_index_p [DoD_log_p_i].value = couls;
    number_of_complete_discharges_p = total_DoD / total_c;
    //nrf_gpio_pin_write(LED_3,50);
    }//else{nrf_gpio_pin_write(LED_3,0);}
//end calc

}//end fun

void uhoh_filter(int error_code,int32_t m_v_log[][5][2],int eeww_flag){
  
  uhoh_index[uhoh_p_i].uhoh_time = index_time;
  uhoh_index[uhoh_p_i].uhoh_type = eeww_flag;
  uhoh_index[uhoh_p_i].uhoh_message = error_code;

  switch (error_code){
    case 0xA5://overvoltage
    case 0xAB://Undervoltage
      uhoh_index [uhoh_p_i].uhoh_value = voltage_log [index_step][1];//last voltage
    break;

    case 0xA3://overtemp
    case 0xA9://undertemp
      uhoh_index [uhoh_p_i].uhoh_value = temperature_log [index_step][1];
    break;

    case 0x74://Over current 1
    case 0x75://Over current 2
    case 0x96://Short
      uhoh_index [uhoh_p_i].uhoh_value = load_log [index_step][1]; //load
    break;

    case 0x7E: uhoh_index [uhoh_p_i].uhoh_value = m_v_log [index_step][1][1];break;//IB M1
    case 0x7F: uhoh_index [uhoh_p_i].uhoh_value = m_v_log [index_step][2][1];break;//IB M2
    case 0x80: uhoh_index [uhoh_p_i].uhoh_value = m_v_log [index_step][3][1];break;//IB M3
    case 0x81: uhoh_index [uhoh_p_i].uhoh_value = m_v_log [index_step][4][1];break;//IB M4
  
    default:
    break;
   }

    //this is to check for repeats and will allow if the time is long enough
    int same_msg_check_new = uhoh_index[uhoh_p_i].uhoh_message + uhoh_index[uhoh_p_i].uhoh_type;  
    int same_msg_check_last = uhoh_index[uhoh_p_i - 1].uhoh_message + uhoh_index[uhoh_p_i - 1].uhoh_type; 
    int time_delta = uhoh_index[uhoh_p_i].uhoh_time - uhoh_index[uhoh_p_i - 1].uhoh_time; 
    if (same_msg_check_new != same_msg_check_last){uhoh_p_i ++;}
      else if (time_delta > 10){uhoh_p_i ++;}

}//end fun



#endif  /* _ DATA_STORE_H__ */
