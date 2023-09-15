#include "aplic.h"

void aplic_init(){
  /** Enable APLIC M Domain */
  sw(APLICM_ADDR+DOMAIN_OFF, (0x1<<8)|(0x1<<2));
  sw(APLICS_ADDR+DOMAIN_OFF, (0x1<<8)|(0x1<<2));

//   /** Enable APLIC IDC 0 */
//   sw(APLICM_ADDR+IDELIVERY_OFF, 0x1);
//   sw(APLICS_ADDR+IDELIVERY_OFF, 0x1);

//   sw(APLICM_ADDR+MMSICFGADDR_OFF, 0x24000);
//   sw(APLICM_ADDR+MMSICFGADDRH_OFF, 0x0);
//   sw(APLICM_ADDR+SMSICFGADDR_OFF, 0x28000);
//   sw(APLICM_ADDR+SMSICFGADDRH_OFF, 0x0);
}

void aplic_config_intp(uint8_t intp_id, uint8_t guest_index, uint32_t base_addr){
  /** Config intp source intp_id by writing sourcecfg reg */
  sw(base_addr+(SOURCECFG_OFF+(0x4*(intp_id-1))), EDGE1);
  /** Config intp TARGET by writing target reg */
  // Im writing EEID to target. Only for MSI mode
  sw(base_addr+(TARGET_OFF+(0x4*(intp_id-1))), (guest_index << 12) | intp_id);

  /** Enable intp intp_id by writing setienum reg */
  sw(base_addr+SETIENUM_OFF, intp_id);
}

void aplic_deleg_intp (uint8_t intp_id){
  sw(APLICM_ADDR+(SOURCECFG_OFF+(0x4*(intp_id-1))), DELEGATE_SRC);
}

uint32_t aplic_get_counter(uint32_t base_addr){
  return read32(base_addr+AIA_COUNTER_OFF);
}

void aplic_reset_counter(uint32_t base_addr){
  sw(base_addr+(AIA_COUNTER_OFF), 0);
}

void aplic_start_interf_0(uint32_t base_addr){
  sw(base_addr+(AIA_INTERF_OFF), 0);
}

void aplic_start_interf_1(uint32_t base_addr){
  lw(base_addr+(AIA_INTERF_OFF));
}