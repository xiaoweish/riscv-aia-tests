#include <rvh_test.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "apb_timer.h"
#include "aplic.h"
#include "imsic.h"

#define APB_TIMER_RST_CMP 0x05
#define APB_TIMER_IRQ 5

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

bool timer_config_test(){
  TEST_START();

  /** Instantiate and map the timer */
  struct apb_timer *timer_ut = (void*)APB_TIMER_BASE_ADDR;

  // Initialize the timer into a well defined state
  apb_timer_init(timer_ut);

  // Configure the timer to count APB_TIMER_RST_CMP
  apb_timer_set_compare(timer_ut, APB_TIMER_RST_CMP);

  // Start the timer count
  apb_timer_start(timer_ut);

  // Wait for timer to reach compare value
  while (apb_timer_get_timer(timer_ut) < APB_TIMER_RST_CMP);

  // Stop the timer count
  apb_timer_stop(timer_ut);

  // Write to compare register to see if the timer register is being reseted
  apb_timer_set_compare(timer_ut, APB_TIMER_RST_CMP);

  TEST_ASSERT("Timer configuration", true);

  TEST_END();
}

void timer_simple_config(void){
  /** Instantiate and map the timer */
  struct apb_timer *timer_ut = (void*)APB_TIMER_BASE_ADDR;

  // Initialize the timer into a well defined state
  apb_timer_init(timer_ut);

  // Configure the timer to count APB_TIMER_RST_CMP
  apb_timer_set_compare(timer_ut, 1000);

  // Start the timer count
  apb_timer_start(timer_ut);
}

void main() {

  INFO("CVA6 w/ H-ext + AIA");

  if(check_misa_h()){
    reset_state();
  }

  aplic_init();
  // aplic_idc();
  imsic_init();

  /** Configure timer intp (5)*/
  aplic_deleg_intp(APB_TIMER_IRQ);
  // ==============================================
  // use this config function to test APLIC only platforms
  // aplic_config_intp_direct_mode(APB_TIMER_IRQ, 1, APLICS_ADDR);
  // ==============================================

  // ==============================================
  // use this to test APLIC + IMSIC platforms
  // The configuration function assumes that the MSI ID is = to the physical intp ID
  aplic_config_intp_msi_mode(APB_TIMER_IRQ, 0, APLICS_ADDR);
  imsic_en_intp(APB_TIMER_IRQ, IMSICS);
  // ==============================================

  // apb_timer basic configuration test
  // Count to APB_TIMER_RST_CMP (5)
  timer_config_test();

  // aplic_clr_intp(APLICS_ADDR);
  imsic_clr_intp(IMSICS);

  INFO("end");
  exit(0);
}
