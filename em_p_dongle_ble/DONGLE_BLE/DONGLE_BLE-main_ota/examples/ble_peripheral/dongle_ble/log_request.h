
#ifndef LOG_REQUEST_H__
#define LOG_REQUEST_H__


#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "math.h"
#include "data_store.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "uart_handler.h"

//void persistence_check (ble_os_t *m_our_service){
         
//        log_characteristic_update(m_our_service, *p_addr);
//}

void log_pull (int *writeVal, ble_os_t *m_our_service)//this funtion will write the data to the charactoristic based on the values
{
  short data_dump[64] = {0};
  int send_length = 128;//nuber of bits in the send
  int page_number = 0;
  int period = (writeVal[1]*100) + (writeVal[2]*10) + writeVal[3];
  int serial_val = writeVal[9]*10000 + writeVal[10]*1000 + writeVal[11]*100 + writeVal[12]*10 + writeVal[13];
  int send_counter = 4;
  int data_size_page = 30;//size of data per send //this should be made so that the max page size is dynamic

if (writeVal[0] == 0xFD){ //Write serial
    //nrf_gpio_pin_toggle(LED_2);
      for(int i = 0; i<12; i++){  
        data_dump[i+4] = writeVal[i+1];
        }
                data_dump[0] = writeVal[0];
        log_characteristic_update(m_our_service, data_dump);

        }
        
        else if (writeVal[0] == 0xFE){ //Write serial
    //nrf_gpio_pin_toggle(LED_3);
      //for(int i = 0; i<12; i++){  
        //data_dump[i+4] = writeVal[i+1];
        //}
                //data_dump[0] = writeVal[0];
                ble_stack_resolver(writeVal);
        //        data_dump[4] = *p_addr;
        //log_characteristic_update(m_our_service, data_dump);
        }
        
        else {


  if (period > 0 ) {
  //
    for (int i=1;i <= period;i++){

    switch (writeVal[0]){

          
      case 0xFF://All values,
      //this doesnt do anything ATM
      break;

      case 0xA0: //batt voltage
        data_dump[send_counter] = voltage_log_index_p[voltage_log_p_i - i].time;
        data_dump[send_counter + 1] = voltage_log_index_p[voltage_log_p_i - i].value;
      break;

      case 0xA1: //mod 1 voltage
        data_dump[send_counter] = m_v_log[index_time-i][0][0];
        data_dump[send_counter + 1] = m_v_log[index_time-i][1][1];
      break;

      case 0xA2: //mod 2 voltage
        data_dump[send_counter] = m_v_log[index_time-i][0][0];
        data_dump[send_counter + 1] = m_v_log[index_time-i][2][1];
      break;

      case 0xA3: //mod 3 voltage
        data_dump[send_counter] = m_v_log[index_time-i][0][0];
        data_dump[send_counter + 1] = m_v_log[index_time-i][3][1];
      break;

      case 0xA4: //mod 4 voltage 
        data_dump[send_counter] = m_v_log[index_time-i][0][0];
        data_dump[send_counter + 1] = m_v_log[index_time - i][4][1];
      break;

      case 0xAB: //mod delta voltage
        data_dump[send_counter] = m_v_log[index_time-i][0][0];
        data_dump[send_counter + 1] = m_v_log[index_time-i][0][1];
      break;

      case 0xB0: //Coulumbs(load)
        data_dump[send_counter + 0] = load_log_index_p[load_log_p_i - i].time;
        data_dump[send_counter + 1] = load_log_index_p[load_log_p_i - i].value;
      break;

      case 0xB1: //Amps(load)
        data_dump[send_counter + 0] = load_log_index_p[load_log_p_i - i].time;
        data_dump[send_counter + 1] = load_log_index_p[load_log_p_i - i].value / COULOMB_CONV;
      break;

      case 0xC0: //State of Charge
        data_dump[send_counter + 0] = load_log_index_p[load_log_p_i - i].time;
        data_dump[send_counter + 1] = (CAPACITY - ((load_log_index_p[load_log_p_i - i].value / COULOMB_CONV)/100))* 100;
      break;

      case 0xC1: //State of Charge %
        data_dump[send_counter + 0] = load_log_index_p[load_log_p_i - i].time;
        data_dump[send_counter + 1] = (CAPACITY - ((load_log_index_p[load_log_p_i - i].value / COULOMB_CONV)/100)) / CAPACITY * 10000;
      break;

      case 0xEE: //Errors
        data_dump[send_counter + 0] = uhoh_index [uhoh_p_i - i].uhoh_time;
        data_dump[send_counter + 1] = uhoh_index [uhoh_p_i - i].uhoh_type;
        data_dump[send_counter + 2] = uhoh_index [uhoh_p_i - i].uhoh_message;
        data_dump[send_counter + 3] = uhoh_index [uhoh_p_i - i].uhoh_value;
        send_counter +=2;
      break;

      case 0xCF: //Temperature
      if (temperature_log_p_i - i > 0){
        data_dump[send_counter] = temperature_log_index_p[temperature_log_p_i - i].time;
        data_dump[send_counter + 1] = temperature_log_index_p[temperature_log_p_i - i].value;
      }
      else{
        data_dump[send_counter] = 0;
        data_dump[send_counter + 1] = 0;
      }
      break;

      case 0xD0: //Dod
        data_dump[send_counter + 0] = DoD_log_index_p[DoD_log_p_i - i].time;
        data_dump[send_counter + 1] = DoD_log_index_p[DoD_log_p_i - i].value;
      break;

      case 0xD1: //State log
        data_dump[send_counter + 0] = charge_log_index_p[charge_log_p_i - i].time;
        data_dump[send_counter + 1] = charge_log_index_p[charge_log_p_i - i].value;
      break;

      case 0xD2: //Complete Discharges
        data_dump[send_counter + 1] = number_of_complete_discharges_p;
      break;

    
      default:
      break;
      }

      send_counter +=2;
        
      if (send_counter == (send_length/2) || i == period){ //this is if the page limit has been reached or if the remainder has been reached.
        send_counter = 4;
        int total_pages = ceil(period / data_size_page) + 1;
        page_number ++;
        data_dump[0] = writeVal[0];
        data_dump[1] = total_pages + (page_number * 256) ;
        data_dump[2] = period;
        data_dump[3] = 0xFFFF; // this should be a more useful thing, but what?
        log_characteristic_update(m_our_service, data_dump); 
        }

    }//end for
   }//end if for period check
   else {//only one send
     switch (writeVal[0]){
     case 0x0F:
      data_dump[4] = index_time;
      nrf_fstorage_read(&fstorage,0x28000,data_read,12);
      data_dump[5] = data_read[0];
      break;

     case 0x0D:
      data_dump[4]=number_of_complete_discharges_p;
      break;

      case 0x1F:
        NRF_LOG_INFO("State of Health");
        //TODO: calculate state of health
        data_dump[4]=777;
        break;

      case 0x2F:
        NRF_LOG_INFO("Battery Starting Capacity");
        data_dump[4]=CAPACITY;
        break;

   case 0xFA: //ID
              for(int i = 0; i<12; i++){  
        data_dump[i+4] = SERIAL[i];
        }
        data_dump[16] = DEV_HANDLE[0];
        data_dump[17] = DEV_HANDLE[1];
      break;

    default:
        break;
        }

        data_dump[0] = writeVal[0];
        log_characteristic_update(m_our_service, data_dump);

    }// end else
    }

     }//end dump function

#endif  /* _ LOG_REQUEST_H__ */
