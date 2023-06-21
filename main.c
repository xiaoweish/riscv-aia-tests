#include <rvh_test.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DELEGATE_SRC        0x400
#define INACTIVE            0
#define DETACHED            1
#define EDGE1               4
#define EDGE0               5
#define LEVEL1              6
#define LEVEL0              7

#define APLICM_ADDR         0xc000000
#define APLICS_ADDR         0xd000000

#define DOMAIN_OFF          0x0000
#define SOURCECFG_OFF       0x0004
#define APLIC_MMSICFGADDR   0x1bc0
#define APLIC_MMSICFGADDRH  0x1bc4
#define APLIC_SMSICFGADDR   0x1bc8
#define APLIC_SMSICFGADDRH  0x1bcc
#define SETIP_OFF           0x1C00
#define SETIE_OFF           0x1E00
#define SETIPNUM_OFF        0x1CDC
#define SETIENUM_OFF        0x1EDC
#define GENMSI_OFF          0x3000
#define TARGET_OFF          0x3004

#define IDC_OFF              0x4000
#define IDELIVERY_OFF        IDC_OFF + 0x00
#define TOPI_OFF             IDC_OFF + 0x18
#define CLAIMI_OFF           IDC_OFF + 0x1C

#define SSTATUS_SIE          (1ULL << 1)
#define DELEGATE_SRC         0x400

static inline void touchread(uintptr_t addr) {
  asm volatile("" ::: "memory");
  volatile uint64_t x = *(volatile uint64_t *)addr;
}

static inline void touchwrite(uintptr_t addr) {
  *(volatile uint64_t *)addr = 0xdeadbeef;
}

bool check_misa_h(){

    TEST_START();

    uint64_t misa = CSRR(misa);
    CSRS(misa, (1ULL << 7));

    bool hyp_ext_present = CSRR(misa) & (1ULL << 7);
    TEST_ASSERT("check h bit after setting it",  hyp_ext_present, "hypervisor extensions not present");

    if(!hyp_ext_present)
        return false;

    CSRC(misa, (1ULL << 7));
    if(((CSRR(misa) & (1ULL << 7)))){
        VERBOSE("misa h bit is hardwired");
    }

    CSRW(misa, misa);

    TEST_END();
}

void deleg_intp (uint8_t intp_id){
  sw(APLICM_ADDR+(SOURCECFG_OFF+(0x4*(intp_id-1))), DELEGATE_SRC);
}

void aplic_config_intp(uint8_t intp_id, uint8_t intp_prio, uint32_t base_addr){

  /** Config intp source intp_id by writing sourcecfg reg */
  sw(base_addr+(SOURCECFG_OFF+(0x4*(intp_id-1))), EDGE1);
  /** Config intp priority and TARGET by writing target reg */
  /** thatget hart is always 0*/
  sw(base_addr+(TARGET_OFF+(0x4*(intp_id-1))), intp_prio);

  /** Enable intp intp_id by writing setienum reg */
  sw(base_addr+SETIENUM_OFF, intp_id);
}

void aplic_trigger_intp(uint8_t intp_id, uint32_t base_addr){
  sw(base_addr+SETIPNUM_OFF, intp_id); 
}

void aplic_genmsi(uint8_t intp_id, uint32_t base_addr){
  /** trigger intp intp_id by writing genmsi reg */
  sw(base_addr+GENMSI_OFF, intp_id);
}

void aplic_init(){
  /** Enable APLIC M Domain */
  sw(APLICM_ADDR+DOMAIN_OFF, (0x1<<8)|(0x1<<2));
  sw(APLICS_ADDR+DOMAIN_OFF, (0x1<<8)|(0x1<<2));

  /** Enable APLIC IDC 0 */
  sw(APLICM_ADDR+IDELIVERY_OFF, 0x1);
  sw(APLICS_ADDR+IDELIVERY_OFF, 0x1);
}

uint64_t aplic_read_topi(){
  uint64_t addr = APLICS_ADDR + TOPI_OFF;
  return *(volatile uint64_t *)addr;
}

bool aplic_test(){
  TEST_START();
  
  uint64_t topi = 0;
  bool test_passed = false;

  /** enable M and S domains and enable IDC 0 in both*/
  aplic_init();

  /** Delegate interrupt 1 to S domain*/
  deleg_intp(1);
  /** In S domain, configure the interrupt 1 with priority 1*/
  aplic_config_intp(1, 1, APLICS_ADDR);

  /** We now trigger interrupt 1 */
  aplic_trigger_intp(1, APLICS_ADDR);

  while((topi = aplic_read_topi()) == 0){}

  if (topi == 1){
    test_passed = true;
  }

  TEST_ASSERT("interrupt was generated and topi has value 1",  test_passed);

  TEST_END();
}

void main() {

  INFO("RISC-V APLIC test ");

  if(check_misa_h()){
    reset_state();
    aplic_test();
  }
  
  INFO("end");
  exit(0);
}
