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

#define IOPMP_CTL 0x50000000

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
  //uintptr_t ptr = 0x81000000;
  TEST_START();
  sw(IOPMP_CTL, 0xffffffff);
  //touchwrite(ptr);
  bool cond_ctl = (lw(IOPMP_CTL) == 0xc0000001);
  TEST_ASSERT("When iopmp_ctl == 0xc0000001, ", cond_ctl);
  TEST_END();

  /* sw(IOPMP_MDMASK, 0x8000000000010001); */
  /* bool cond_mdmask = (lw(IOPMP_MDMASK) == 0xc0000001); */
  /* TEST_ASSERT("When iopmp_mdmask == 0, ", cond_mdmask); */

  /* sw(IOPMP_ENTRY_ADDR, 0); */
  /* bool cond_entry_addr = (lw(IOPMP_ENTRY_ADDR) == 0xc0000001); */
  /* TEST_ASSERT("When iopmp_entry_addr == 0, ", cond_entry_addr); */

  /* sw(IOPMP_ENTRY_CFG, 0x10100111); */
  /* bool cond_entry_cfg = (lw(IOPMP_ENTRY_CFG) == 0xc0000001); */
  /* TEST_ASSERT("When iopmp_entry_cfg == 0, ", cond_entry_cfg); */

  /* sw(IOPMP_SRCMD, 1); */
  /* bool cond_srcmd = (lw(IOPMP_SRCMD) == 0xc0000001); */
  /* TEST_ASSERT("When iopmp_srcmd == 0, ", cond_srcmd); */

  /* sw(IOPMP_RCD_OFF, ); */
  /* sw(IOPMP_RCD_ADDR_OFF, ); */
  /* sw(IOPMP_MDLCK_OFF, 0x7fffffff); */
  /* sw(IOPMP_MDCFG_OFF, 0); */
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
