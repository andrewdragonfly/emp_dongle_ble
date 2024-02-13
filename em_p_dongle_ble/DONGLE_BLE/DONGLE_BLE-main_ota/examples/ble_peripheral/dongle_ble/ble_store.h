#ifndef BLE_STORE_H__
#define BLE_STORE_H__


#include "nrf_nvmc.h"


const uint32_t serial_addr  = 0x00027000;//27000-27004
  uint32_t *p_serial_addr   = (uint32_t *)serial_addr;

const uint32_t date_addr    = 0x00027008;
  uint32_t *p_date_addr     = (uint32_t *)date_addr;

const uint32_t fwv_addr     = 0x0002700C;//2700c-270010
  uint32_t *p_fwv_addr      = (uint32_t *)fwv_addr;

const uint32_t hwv_addr     = 0x00027014;//27014-27018
  uint32_t *p_hwv_addr      = (uint32_t *)hwv_addr;

const uint32_t volt_addr    = 0x0002701C;
  uint32_t *p_volt_addr     = (uint32_t *)volt_addr;

const uint32_t cap_addr     = 0x0002703C;
  uint32_t *p_cap_addr      = (uint32_t *)cap_addr;

const uint32_t cc_addr      = 0x0002705C;
  uint32_t *p_cc_addr       = (uint32_t *)cc_addr;

const uint32_t sv_addr      = 0x00027060;
  uint32_t *p_sv_addr       = (uint32_t *)sv_addr;


struct {
  char serial[12];
  char date[8];
  char fwv_s[18];
  char hwv_s[9];
  char fwv[16];
  char hwv[8];
  char mod_volt[8][8];//[mod no][string]
  char mod_cap[8][8];//[mod no][string]
  char cc_coef[8];
  char sv_coef[8];
}battery_def;

void set_serial (){
  nrf_nvmc_page_erase(serial_addr);
    //this does nothing now. set could be from ble command
  //nrf_nvmc_write_word(serial_addr,serial1);

}

unsigned char string_buffer[12];
unsigned char string_buffer2[12];

//int ascii_convert (int val, int type){
//  switch (type){
//    case 0:
//      if (val<0x40){
//      return val-0x30;}
//        else {
//          return val-0x37;
//        }
//    break;

//    case 1:
//      if (val<0x40){
//        return val-0x0;}
//          else {   
//            return val-0x0;
//        }
//    break;

//    default:
//    break;
//  }
//  }

void get_serial (){
  sprintf(string_buffer,"%06x",*(p_serial_addr));
  sprintf(string_buffer2,"%08x",*(p_serial_addr + 1));
  for (int i = 0; i<4 ;i++){
    string_buffer[8+i]=string_buffer2[i];
  }
  for (int i = 0; i<12;i++){
    battery_def.serial[i] = string_buffer[i];
  }
}

void get_date(){
  sprintf(string_buffer,"%08x",*p_date_addr);
  for (int i = 0; i<8;i++){
    battery_def.date[i] = string_buffer[i];
  }
}

void get_firmware_vers(){//FFFF.FFFF | FFFFFFFF
  sprintf(string_buffer,"%08x",*p_fwv_addr);
  sprintf(string_buffer2,"%08x",*(p_fwv_addr + 1));

  for (int i = 0; i < 8; i++){
    battery_def.fwv[i] = string_buffer[i] ;
    battery_def.fwv[i+7] = string_buffer2[i] ;
  }
  int j = 0;
  for(int i = 0; i < 16;i++){
  
    if (i==4 || i==8){
      battery_def.fwv_s[j] = '.';
      j++;
    }

    if (battery_def.fwv[i] != 'f'){
      battery_def.fwv_s[j] = battery_def.fwv[i];
      j++;
    }
  }
}

void get_hardware_vers(){//FFFF.FFFF
  sprintf(string_buffer,"%08x",*p_hwv_addr);
  for (int i = 0; i<8; i++){
    battery_def.hwv[i] =string_buffer[i] ;
    }
  int j = 0;
  for(int i = 0; i < 8;i++){
    if (i==4){
      battery_def.hwv_s[j] = '.';
      j++;
    }
    if (battery_def.hwv[i] != 'f'){
      battery_def.hwv_s[j] = battery_def.hwv[i];
      j++;
    }
  }
}

void get_cycler_data (){
  for (int mod_i = 0; mod_i < 8; mod_i++){
    sprintf(string_buffer,"%08x",*(p_cap_addr + (mod_i*4)));
    for (int i = 0; i<8;i++){
      if(string_buffer[i] == 'f'){string_buffer[i] = '0';}
      battery_def.mod_cap[mod_i][i] = string_buffer[i];
    }

    sprintf(string_buffer,"%08x",*(p_volt_addr + (mod_i*4)));
    for (int i = 0; i<8;i++){
      if(string_buffer[i] == 'f'){string_buffer[i] = '0';}
      battery_def.mod_volt[mod_i][i] = string_buffer[i];
    }
  }
}

void get_sv_coef (){
  sprintf(string_buffer,"%08x",*p_sv_addr);
  for (int i = 0; i<8;i++){
    battery_def.sv_coef[i] = string_buffer[i];
  }
}

void get_cc_coef (){
  sprintf(string_buffer,"%08x",*p_cc_addr);
  for (int i = 0; i<8;i++){
    battery_def.cc_coef[i] = string_buffer[i];
  }
}


void get_all_defs(){
  get_serial();
  //get_date();
  //get_firmware_vers();
  //get_hardware_vers();
  //get_sv_coef();
  //get_cc_coef();
  //get_cycler_data();
}




#endif /* ZB_STORE_H__ */