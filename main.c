#include <rvh_test.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "idma.h"

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

bool idma_single_transfer(){

  TEST_START();

  /** Instantiate and map the DMA */
  struct idma *dma_ut = (void*)IDMA_BASE_ADDR;

  /** set iDMA source and destiny adresses */
  uintptr_t idma_src_addr = get_addr_base(0);
  uintptr_t idma_dest_addr = get_addr_base(1);

  /** Write known values to memory */
  /** Source memory position has 0xdeadbeef */
  write64(idma_src_addr, 0xdeadbeef);
  /** Clear the destiny memory position */
  write64(idma_dest_addr, 0x00);

  idma_config_single_transfer(dma_ut, idma_src_addr, idma_dest_addr);

  // Check if iDMA was set up properly and init transfer
  uint64_t trans_id = read64((uintptr_t)&dma_ut->next_transfer_id);
  if (!trans_id){
    ERROR("iDMA misconfigured")
  }

  // Poll transfer status
  while (read64((uintptr_t)&dma_ut->last_transfer_id_complete) != trans_id);

  bool check = (read64(idma_dest_addr) == 0xdeadbeef);

  TEST_ASSERT("iDMA Single transfer", check);

  TEST_END();
}

void main() {

  INFO("CVA6 w/ H-ext + Pulp iDMA");

  if(check_misa_h()){
    reset_state();
  }

  // iDMA tests
  idma_single_transfer();

  INFO("end");
  exit(0);
}
