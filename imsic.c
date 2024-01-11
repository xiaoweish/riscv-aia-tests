#include "imsic.h"

static inline void set_vgein(uint8_t vgein){
  CSRW(CSR_HSTATUS, vgein<<12);
}

void imsic_init(){
  /** Enable interrupt delivery */
  CSRW(CSR_MISELECT, IMSIC_EIDELIVERY);
  CSRW(CSR_MIREG, 1);
  CSRW(CSR_SISELECT, IMSIC_EIDELIVERY);
  CSRW(CSR_SIREG, 1);
  // set_vgein(1);
  // CSRW(CSR_VSISELECT, IMSIC_EIDELIVERY);
  // CSRW(CSR_VSIREG, 1);
  // set_vgein(0);

  /** Every intp is triggrable */
  CSRW(CSR_MISELECT, IMSIC_EITHRESHOLD);
  CSRW(CSR_MIREG, 0);
  CSRW(CSR_SISELECT, IMSIC_EITHRESHOLD);
  CSRW(CSR_SIREG, 0);
  // set_vgein(1);
  // CSRW(CSR_VSISELECT, IMSIC_EITHRESHOLD);
  // CSRW(CSR_VSIREG, 0);
  // set_vgein(0);
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

void imsic_clr_intp(uint8_t imsic_type) 
{
if (imsic_type == IMSICM){
    CSRW(CSR_MTOPEI, 0);
  } else if (imsic_type == IMSICS){
    CSRW(CSR_STOPEI, 0);
  } else if (imsic_type == IMSICVS){
    set_vgein(1);
    CSRW(CSR_VSTOPEI, 0);
    set_vgein(0);
  }
}