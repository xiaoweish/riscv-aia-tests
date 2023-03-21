#include <rvh_test.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IMSICM              0
#define IMSICS              1
#define IMSICVS             2

#define DELEGATE_SRC        0x400
#define INACTIVE            0
#define DETACHED            1
#define EDGE1               4
#define EDGE0               5
#define LEVEL1              6
#define LEVEL0              7

#define CSR_MIE		          0x304
#define CSR_HSTATUS		      0x600

#define CSR_MISELECT		    0x350
#define CSR_MIREG			      0x351
#define CSR_MTOPEI			    0x35c

#define CSR_SISELECT			  0x150
#define CSR_SIREG			      0x151
#define CSR_STOPEI			    0x15c

#define CSR_VSISELECT			  0x250
#define CSR_VSIREG			    0x251
#define CSR_VSTOPEI			    0x25c

#define IMSIC_EIDELIVERY		0x70
#define IMSIC_EITHRESHOLD		0x72
#define IMSIC_EIP		        0x80
#define IMSIC_EIE		        0xC0

#define APLICM_ADDR         0xc000000
#define APLICS_ADDR         0xd000000
#define DOMAIN_OFF          0x0000
#define SOURCECFG_OFF       0x0004
#define APLIC_MMSICFGADDR              0x1bc0
#define APLIC_MMSICFGADDRH             0x1bc4
#define APLIC_SMSICFGADDR              0x1bc8
#define APLIC_SMSICFGADDRH             0x1bcc
#define SETIP_OFF           0x1C00
#define SETIE_OFF           0x1E00
#define SETIPNUM_OFF        0x1CDC
#define SETIENUM_OFF        0x1EDC
#define GENMSI_OFF          0x3000
#define DEBUG_OFF          0x2008
#define TARGET_OFF          0x3004

#define IDC_OFF              0x4000
#define IDELIVERY_OFF        IDC_OFF + 0x00

#define DELEGATE_SRC            0x400

static inline void touchread(uintptr_t addr) {
  asm volatile("" ::: "memory");
  volatile uint64_t x = *(volatile uint64_t *)addr;
}

static inline void touchwrite(uintptr_t addr) {
  *(volatile uint64_t *)addr = 0xdeadbeef;
}

void deleg_intp (uint8_t intp_id){
  sw(APLICM_ADDR+(SOURCECFG_OFF+(0x4*(intp_id-1))), DELEGATE_SRC);
}

void config_intp(uint8_t intp_id, uint8_t guest_index, uint32_t base_addr){
  bool cond_ctl;
  uint64_t hold;

  /** Config intp source intp_id by writing sourcecfg reg */
  sw(base_addr+(SOURCECFG_OFF+(0x4*(intp_id-1))), EDGE1);
  /** Config intp TARGET by writing target reg */
  // Im writing EEID to target. Only for MSI mode
  sw(base_addr+(TARGET_OFF+(0x4*(intp_id-1))), (guest_index << 12) | intp_id);

  /** Enable intp intp_id by writing setienum reg */
  sw(base_addr+SETIENUM_OFF, intp_id);
}

static inline void set_vgein(uint8_t vgein){
  CSRW(CSR_HSTATUS, vgein<<12);
}

void imsic_en_intp(uint8_t intp_id, uint8_t imsic_type){
  uint32_t prev_val = 0;

  if (imsic_type == IMSICM){
    CSRW(CSR_MISELECT, IMSIC_EIE);
    prev_val = CSRR(CSR_MIREG);
    CSRW(CSR_MIREG, prev_val | (1 << intp_id));
  } else if (imsic_type == IMSICS){
    CSRW(CSR_SISELECT, IMSIC_EIE);
    prev_val = CSRR(CSR_SIREG);
    CSRW(CSR_SIREG, prev_val | (1 << intp_id));
  } else if (imsic_type == IMSICVS){
    set_vgein(1);
    CSRW(CSR_VSISELECT, IMSIC_EIE);
    prev_val = CSRR(CSR_VSIREG);
    CSRW(CSR_VSIREG, prev_val | (1 << intp_id));
    set_vgein(0);
  }
}

// void aplic_trigger_src_intp(uint32_t val){
//   sw(APLICM_ADDR+DEBUG_OFF, val); 
// }

void aplic_trigger_intp(uint8_t intp_id, uint32_t base_addr){
  sw(base_addr+SETIPNUM_OFF, intp_id); 
}

void aplic_genmsi(uint8_t intp_id, uint32_t base_addr){
  /** trigger intp intp_id by writing genmsi reg */
  sw(base_addr+GENMSI_OFF, intp_id);
}

bool imsic_intp_arrive(uint8_t intp_id, uint8_t imsic_type){
  bool cond_ctl = false;

  do
  {
    if (imsic_type == IMSICM){
      cond_ctl = ((CSRR(CSR_MTOPEI) >> 16) == intp_id);    
    }
    else if (imsic_type == IMSICS){
      cond_ctl = ((CSRR(CSR_STOPEI) >> 16) == intp_id);    
    }
    else if (imsic_type == IMSICVS){
      set_vgein(1);
      cond_ctl = ((CSRR(CSR_VSTOPEI) >> 16) == intp_id);    
      set_vgein(0);
    }
  }while (cond_ctl == false);

  return cond_ctl;
}

void aplic_init(){
  /** Enable APLIC M Domain */
  sw(APLICM_ADDR+DOMAIN_OFF, (0x1<<8)|(0x1<<2));
  sw(APLICS_ADDR+DOMAIN_OFF, (0x1<<8)|(0x1<<2));

  /** Enable APLIC IDC 0 */
  sw(APLICM_ADDR+IDELIVERY_OFF, 0x1);
  sw(APLICS_ADDR+IDELIVERY_OFF, 0x1);

  sw(APLICM_ADDR+APLIC_MMSICFGADDR, 0x24000);
  sw(APLICM_ADDR+APLIC_MMSICFGADDRH, 0x0);
  sw(APLICM_ADDR+APLIC_SMSICFGADDR, 0x28000);
  sw(APLICM_ADDR+APLIC_SMSICFGADDRH, 0x0);
}

void imsic_init(){
  /** Enable interrupt delivery */
  CSRW(CSR_MISELECT, IMSIC_EIDELIVERY);
  CSRW(CSR_MIREG, 1);
  CSRW(CSR_SISELECT, IMSIC_EIDELIVERY);
  CSRW(CSR_SIREG, 1);
  set_vgein(1);
  CSRW(CSR_VSISELECT, IMSIC_EIDELIVERY);
  CSRW(CSR_VSIREG, 1);
  set_vgein(0);

  /** Every intp is triggrable */
  CSRW(CSR_MISELECT, IMSIC_EITHRESHOLD);
  CSRW(CSR_MIREG, 0);
  CSRW(CSR_SISELECT, IMSIC_EITHRESHOLD);
  CSRW(CSR_SIREG, 0);
  set_vgein(1);
  CSRW(CSR_VSISELECT, IMSIC_EITHRESHOLD);
  CSRW(CSR_VSIREG, 0);
  set_vgein(0);
}

bool irqc_test() {
  TEST_START();
  bool cond_ctl;
  uint64_t hold;

  aplic_init();
  imsic_init();

  /** Configure intp 7 */
  config_intp(0x07, 0, APLICM_ADDR);
  /** Enable intp 7 */
  imsic_en_intp(0x07, IMSICM);

  /** Configure intp 13 */
  config_intp(13, 1, APLICS_ADDR);
  /** Enable intp 13 */
  imsic_en_intp(13, IMSICVS);

  /** Configure intp 25 */
  config_intp(25, 0, APLICS_ADDR);
  /** Enable intp 25 */
  imsic_en_intp(25, IMSICS);

  /** Configure intp 10 */
  config_intp(10, 0, APLICS_ADDR);
  /** Enable intp 10 */
  imsic_en_intp(10, IMSICS);

  /** Trigger intp 7 by writing setipnum reg */
  aplic_trigger_intp(0x07, APLICM_ADDR);
  /** Check if intp arraive to IMSIC */
  cond_ctl = false;
  cond_ctl = imsic_intp_arrive(0x07, IMSICM);
  TEST_ASSERT("[w/r] m mtopei value: 0x07, ", cond_ctl);
  /** Clear the intp by writting to mtopei */
  CSRW(CSR_MTOPEI, 0);

  /** Trigger intp 25 by writing setipnum reg */
  aplic_genmsi(25, APLICS_ADDR);  
  /** Check if intp arraive to IMSIC */
  cond_ctl = false;
  cond_ctl = imsic_intp_arrive(25, IMSICS);
  TEST_ASSERT("[w/r] s stopei value: 25, ", cond_ctl);
  CSRW(CSR_STOPEI, 0);   

  /** Test a lot of write to guarantee that the cross bar handle it*/
  for (int i = 0; i < 15; i++){
    aplic_trigger_intp(10, APLICS_ADDR);  
    cond_ctl = false;
    cond_ctl = imsic_intp_arrive(10, IMSICS);
    CSRW(CSR_STOPEI, 0);   
  }
  TEST_ASSERT("[w/r] s stopei value: 10, ", cond_ctl);

  /** Trigger intp 13 by writing setipnum reg */
  aplic_trigger_intp(13, APLICS_ADDR);
  /** Check if intp arraive to IMSIC */
  cond_ctl = false;
  cond_ctl = imsic_intp_arrive(13, IMSICVS);
  TEST_ASSERT("[w/r] vs vstopei value: 13, ", cond_ctl);
  /** Clear the intp by writting to vstopei */
  set_vgein(1);
  CSRW(CSR_VSTOPEI, 0);
  set_vgein(0);

  TEST_END();
}

void main() {

  INFO("RISC-V AIA (IMSIC + APLIC + Smaia/Ssaia) test ");

  irqc_test();

  INFO("end");
  exit(0);
}
