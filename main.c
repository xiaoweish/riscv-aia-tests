#include <rvh_test.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Performance class events encoding
#define MEM_EVENT 0x0
#define CMT_EVENT 0x1
#define MARCH_EVENT 0x2
#define STORE_EVENT 9
#define LOAD_EVENT 8

// IOPMP global defines
#define NR_MEMORY_DOMAINS_MAX   63
#define NR_ENTRIES_MAX          32

// implementation specific
#define NR_ENTRIES_PER_MD 8
#define NR_MD 2
#define NR_ENTRY_CFG_PER_REG 8

#define MDCFG_OFFSET         8 * (NR_MEMORY_DOMAINS_MAX)
#define ENTRY_ADDR_OFFSET    MDCFG_OFFSET + (8 * (NR_ENTRIES_MAX))
#define ENTRY_CFG_OFFSET     (ENTRY_ADDR_OFFSET + 8 *  (NR_ENTRIES_MAX))

#define IOPMP_BASE_ADDR         0x50000000
#define IOPMP_CTL_OFF           0x0                       
#define IOPMP_RCD_OFF           0x8                       
#define IOPMP_RCD_ADDR_OFF      0x10                      
#define IOPMP_MDMASK_OFF        0x18                      
#define IOPMP_MDLCK_OFF         0x20                      
#define IOPMP_MDCFG_OFF         0x100                        
#define IOPMP_ENTRY_ADDR_OFF    (0x108 + MDCFG_OFFSET)        
#define IOPMP_ENTRY_CFG_OFF     (0x110 + ENTRY_ADDR_OFFSET) 
#define IOPMP_SRCMD_OFF         (0x118 + ENTRY_CFG_OFFSET)  

static inline void touchread(uintptr_t addr) {
  asm volatile("" ::: "memory");
  volatile uint64_t x = *(volatile uint64_t *)addr;
}

static inline void touchwrite(uintptr_t addr) {
  *(volatile uint64_t *)addr = 0xdeadbeef;
}

bool hpm_tests() {
  uintptr_t ptr = 0x81000000;
  TEST_START();
  CSRW(CSR_MCOUNTINHIBIT, (uint64_t)-1);
  CSRW(CSR_MHPMEVENT3, 0);
  CSRW(CSR_MHPMCOUNTER3, 0);
  // If MHPEVENT disable, MHPMCOUNTER does not increment
  CSRW(CSR_MHPMEVENT3, CMT_EVENT | (1 << LOAD_EVENT) | (1 << STORE_EVENT));
  touchwrite(ptr);
  bool cond1 = (CSRR(CSR_MHPMCOUNTER3) == 0);
  TEST_ASSERT("When mhpmevent == x, mhpmcounter increment", !cond1);
  // If MHPEVENT enabled and MCOUNTINHIBIT disable, MHPMCOUNTER does not
  // increment Clear counterinhibit the counter should not count
  CSRW(CSR_MCOUNTINHIBIT, 0);
  // Clear event triggers for counter 3
  CSRW(CSR_MHPMEVENT3, 0);
  // Clear counter 3
  CSRW(CSR_MHPMCOUNTER3, 0);
  // Execute a LOAD instruction again to show that the counter do not increment
  CSRW(CSR_MHPMEVENT3, CMT_EVENT | (1 << LOAD_EVENT) | (1 << STORE_EVENT));
  touchwrite(ptr);
  // Test the consition
  cond1 = (CSRR(CSR_MHPMCOUNTER3) == 0);
  TEST_ASSERT("When mhpmevent == 0, mhpmcounter does not increment", cond1);
  TEST_END();
}
bool iopmp_tests() {
  TEST_START();
  uint64_t handler = 0x0;
  //  Test the control register (iopmp_ctl): 32 bits
  //  Status: validated
  /*  Register format:
      .------.----.-------.----------.----.
      | -    | L  | RCALL | reserved | EN |
      :------+----+-------+----------+----:
      | bits | 31 | 30    | 29-1     | 0  |
      '------'----'-------'----------'----'
  */
  sw(IOPMP_BASE_ADDR+IOPMP_CTL_OFF, 0xffffffff);
  bool cond_ctl = (lw(IOPMP_BASE_ADDR+IOPMP_CTL_OFF) == 0xc0000001);
  TEST_ASSERT("[w/r] iopmp_ctl value: 0xc0000001, ", cond_ctl);

  //  Test the memory domain mask.
  //  Status: validated
  /*  Register format:
      .------.----.------.
      | -    | L  | MDs  |
      :------+----+------:
      | bits | 63 | 62-0 |
      '------'----'------'
  */
  sd(IOPMP_BASE_ADDR+IOPMP_MDMASK_OFF, 0x8000000000010001);
  bool cond_mdmask = (ld(IOPMP_BASE_ADDR+IOPMP_MDMASK_OFF) == 0x8000000000010001);
  TEST_ASSERT("[w/r] iopmp_mdmask value: 0x8000000000010001, ", cond_mdmask);
  
  //  Test record register control
  //  Notes: The only bit that can be written is the ILLCGT
  //  and id W1C tyo clear the interrupt
  /*  Register format:
    .------.--------.-------.-------.----.------.
    | -    | ILLCGT | EXTRA | LEN   | R  | SID  |
    :------+--------+-------+-------+----+------:
    | bits | 31     | 30-28 | 27-15 | 14 | 13-0 |
    '------'--------'-------'-------'----'------'
  */

  //  Test memory domain lock register
  //  Status: validated
  /* Register format:
    .------.------.
    | -    | F    |
    :------+------:
    | bits | 31-0 |
    '------'------'
  */

  bool cond_mdlck = (lw(IOPMP_BASE_ADDR+IOPMP_MDLCK_OFF) == 0x8000FFFF);
  TEST_ASSERT("[ro] iopmp_mdlck value: 0x8000FFFF, ", cond_mdlck);

  //  Test memory domain config register
  //  Status: VALIDATED
  //  Notes: in this implementation is read only 
  /*
      .------.----.-------.------.
      | -    | L  | F     | T    |
      :------+----+-------+------:
      | bits | 31 | 30-16 | 15-0 |
      '------'----'-------'------'
  */
  // test for MD0. It should assert L and put the value 8 into T. F is 0.
  handler = lw(IOPMP_BASE_ADDR+IOPMP_MDCFG_OFF);
  bool cond_mdcfg = (handler == 0x80000008);
  TEST_ASSERT("[ro] iopmp_mdcfg value: 0x80000008, ", cond_mdcfg);
  printf("Read value: 0x%llx\r\nOn address: 0x%llx\r\n", handler, IOPMP_BASE_ADDR+IOPMP_MDCFG_OFF);

  // test for MD1. It should assert L and put the value 16 into T. F is 0.
  handler = lw(IOPMP_BASE_ADDR+IOPMP_MDCFG_OFF+8);
  cond_mdcfg = (handler == 0x80000010); //0x80000010
  TEST_ASSERT("[ro] iopmp_mdcfg value: 0x80000010, ", cond_mdcfg);
  printf("Read value: 0x%llx\r\nOn address: 0x%llx\r\n", handler, IOPMP_BASE_ADDR+IOPMP_MDCFG_OFF+8);
  
  //  Test entry address register: 64 bits
  //  Status: validated
  /*  Register format:
      .------.------. 
      | -    | ADDR |
      :------+------:
      | bits | 63-0 |
      '------'------'
  */
  // This loop will test all 16 entries to guaratee that all of them can be configurable.
  bool cond_entry_addr;
  for(uint64_t i = IOPMP_ENTRY_ADDR_OFF; i < IOPMP_ENTRY_ADDR_OFF + ((NR_ENTRIES_PER_MD*NR_MD)*8) ; i = i + 8){
    sd(IOPMP_BASE_ADDR + i, 0x0000000000FFFFFF+i);
    cond_entry_addr = (ld(IOPMP_BASE_ADDR+i) == 0x0000000000FFFFFF+i);
    TEST_ASSERT("[w/r] iopmp_entry_addr value: 0x0000000000FFFFFF+i, ", cond_entry_addr);
  }

  //  Test entry configuration register
  //  Status: validated
  /* Register format:
    .------.---.----------.-----.---.---.---.
    | -    | L | reserved | A   | I | W | R |
    :------+---+----------+-----+---+---+---:
    | bits | 7 | 6-5      | 4-3 | 2 | 1 | 0 |
    '------'---'----------'-----'---'---'---'
  */
 // This loop will test all 16 entries to guaratee that all of them can be configurable.
  bool cond_entry_confg;
  for(uint64_t i = IOPMP_ENTRY_CFG_OFF; i < IOPMP_ENTRY_CFG_OFF + (((NR_ENTRIES_PER_MD*NR_MD)/NR_ENTRY_CFG_PER_REG)*8) ; i = i + 8){
    sw(IOPMP_BASE_ADDR + i, 0xFFFFFFFF);
    handler = lw(IOPMP_BASE_ADDR+i);
    cond_entry_confg = (handler == 0x9F9F9F9F);
    TEST_ASSERT("[w/r] iopmp_entry_cfg value: 0x9F9F9F9F, ", cond_entry_confg);
    printf("Read value: 0x%llx\r\nOn address: 0x%llx\r\n", handler, IOPMP_BASE_ADDR+i);
  }


  //  Test source memory domaind access: 64 bits
  //  Status: Need more tests having in account mdmask
  /*  Register format:
      +------+----+------+
      |  -   | L  |  MD  |
      +------+----+------+
      | bits | 63 | 62-0 |
      +------+----+------+
  */
  bool cond_srcmd;
  for(uint64_t i = IOPMP_SRCMD_OFF; i < IOPMP_SRCMD_OFF + (NR_MD*8) ; i = i + 8){
    sd(IOPMP_BASE_ADDR + i, 0x00d0300400FFFFFF);
    handler = ld(IOPMP_BASE_ADDR+i);
    cond_srcmd = (handler == (0x00d0300400FFFFFF & ~(0x0000000000010001))); // 0x0000000000010001 comes from mdmask register.
    TEST_ASSERT("[w/r] iopmp_srcmd value: 0x00d0300400FFFFFF+i, ", cond_srcmd);
    printf("Read value: 0x%llx\r\nOn address: 0x%llx\r\n", handler, IOPMP_BASE_ADDR+i);
  }

  TEST_END();
}

bool check_csr_field_spec() {

  TEST_START();

  /* this assumes machine mode */
  // check_csr_wrrd("mstatus", mstatus, (uint64_t) -1, 0x800000ca007e79aaULL);
  // check_csr_wrrd("mideleg", mideleg, (uint64_t) -1, 0x1666);
  // check_csr_wrrd("medeleg", medeleg, (uint64_t) -1, 0xb15d);
  check_csr_wrrd("mideleg", mideleg, (uint64_t)0, 0x444);
  // check_csr_wrrd("mip", mip, (uint64_t) -1, 0x6e6);
  // check_csr_wrrd("mie", mie, (uint64_t) -1, 0x1eee);
  check_csr_wrrd("mtinst", CSR_MTINST, (uint64_t)-1, (uint64_t)-1);
  check_csr_wrrd("mtval2", CSR_MTVAL2, (uint64_t)-1, (uint64_t)-1);
  // check_csr_wrrd("hstatus", CSR_HSTATUS, (uint64_t) -1, 0x30053f3e0);
  // check_csr_wrrd("hideleg", CSR_HIDELEG, (uint64_t) -1, 0x444);
  // check_csr_wrrd("hedeleg", CSR_HEDELEG, (uint64_t) -1, 0xb1ff);
  // check_csr_wrrd("hvip", CSR_HVIP, (uint64_t) -1, 0x444);
  // check_csr_wrrd("hip", CSR_HIP, (uint64_t) -1, 0x4);
  check_csr_wrrd("hie", CSR_HIE, (uint64_t)-1, 0x444);
  check_csr_wrrd("htval", CSR_HTVAL, (uint64_t)-1, (uint64_t)-1);
  check_csr_wrrd("htinst", CSR_HTINST, (uint64_t)-1, (uint64_t)-1);
  // check_csr_wrrd("hgatp", CSR_HGATP, (8ULL << 60) | (1ULL << 60)-1,
  // 0x80000000000fffffULL); check_csr_wrrd("vsstatus", CSR_VSSTATUS, (uint64_t)
  // -1, 0x80000000000c6122ULL); check_csr_wrrd("vsip", CSR_VSIP, (uint64_t) -1,
  // 0x0); check_csr_wrrd("vsie", CSR_VSIE, (uint64_t) -1, 0x0);
  // check_csr_wrrd("vstvec", CSR_VSTVEC, (uint64_t) -1, 0xffffffffffffff01ULL);
  check_csr_wrrd("vsscratch", CSR_VSSCRATCH, (uint64_t)-1, (uint64_t)-1);
  // check_csr_wrrd("vsepc", CSR_VSEPC, (uint64_t) -1, 0xfffffffffffffffeULL);
  // check_csr_wrrd("vscause", CSR_VSCAUSE, (uint64_t) -1,
  // 0x800000000000001fULL);
  check_csr_wrrd("vstval", CSR_VSTVAL, (uint64_t)-1, 0xffffffffffffffffULL);
  // check_csr_wrrd("vsatp", CSR_VSATP, (uint64_t) -1, 0x0);
  // check_csr_wrrd("vsatp", CSR_VSATP, (8ULL << 60) | (1ULL << 60)-1,
  // 0x80000000000fffffULL);

  TEST_END();
}

bool check_misa_h() {

  TEST_START();

  uint64_t misa = CSRR(misa);
  CSRS(misa, (1ULL << 7));

  bool hyp_ext_present = CSRR(misa) & (1ULL << 7);
  TEST_ASSERT("check h bit after setting it", hyp_ext_present,
              "hypervisor extensions not present");

  if (!hyp_ext_present)
    return false;

  CSRC(misa, (1ULL << 7));
  if (((CSRR(misa) & (1ULL << 7)))) {
    VERBOSE("misa h bit is hardwired");
  }

  CSRW(misa, misa);

  TEST_END();
}

void main() {

  INFO("RISC-V IOPMP read and write test.");

  if (check_misa_h()) {
    reset_state();
    //hpm_tests();
    iopmp_tests();
  }

  INFO("end");
  exit(0);
}
