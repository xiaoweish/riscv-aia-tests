#ifndef IMSIC_H
#define IMSIC_H
#include <rvh_test.h>

#define IMSICM              0
#define IMSICS              1
#define IMSICVS             2

#define CSR_MIE		            0x304
#define CSR_HSTATUS		        0x600

#define CSR_MISELECT		    0x350
#define CSR_MIREG			    0x351
#define CSR_MTOPEI			    0x35c

#define CSR_SISELECT			0x150
#define CSR_SIREG			    0x151
#define CSR_STOPEI			    0x15c

#define CSR_VSISELECT			0x250
#define CSR_VSIREG			    0x251
#define CSR_VSTOPEI			    0x25c

#define IMSIC_EIDELIVERY		0x70
#define IMSIC_EITHRESHOLD		0x72
#define IMSIC_EIP		        0x80
#define IMSIC_EIE		        0xC0

void imsic_init();
void imsic_en_intp(uint8_t intp_id, uint8_t imsic_type);

#endif