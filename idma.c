#include "idma.h"

void idma_config_single_transfer(struct idma *dma_ut, uint64_t src_addr, uint64_t dest_addr)
{
  write64((uintptr_t)&dma_ut->src_addr, src_addr);   // Source address
  write64((uintptr_t)&dma_ut->dest_addr, dest_addr); // Destination address
  write64((uintptr_t)&dma_ut->num_bytes, 8);         // N of bytes to be transferred
  write64((uintptr_t)&dma_ut->config, (1<<3));       // iDMA config: Disable decouple, deburst and serialize
}