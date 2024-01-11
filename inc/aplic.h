#ifndef APLIC_H
#define APLIC_H
#include <rvh_test.h>

#define APLICM_ADDR         (0xc000000)
#define APLICS_ADDR         (0xd000000)
#define DOMAIN_OFF          (0x0000)
#define SOURCECFG_OFF       (0x0004)
#define MMSICFGADDR_OFF     (0x1bc0)
#define MMSICFGADDRH_OFF    (0x1bc4)
#define SMSICFGADDR_OFF     (0x1bc8)
#define SMSICFGADDRH_OFF    (0x1bcc)
#define SETIP_OFF           (0x1C00)
#define SETIE_OFF           (0x1E00)
#define SETIPNUM_OFF        (0x1CDC)
#define SETIENUM_OFF        (0x1EDC)
#define GENMSI_OFF          (0x3000)
#define DEBUG_OFF           (0x2008)
#define TARGET_OFF          (0x3004)

#define IDC_OFF             (0x4000)
#define IDELIVERY_OFF       (IDC_OFF + 0x00)
#define CLAIMI_OFF          (IDC_OFF + 0x1C)

#define DELEGATE_SRC        0x400
#define INACTIVE            0
#define DETACHED            1
#define EDGE1               4
#define EDGE0               5
#define LEVEL1              6
#define LEVEL0              7

void aplic_init();
void aplic_idc();
void aplic_config_intp_direct_mode(uint8_t intp_id, uint8_t prio, uint32_t base_addr);
void aplic_config_intp_msi_mode(uint8_t intp_id, uint8_t guest_index, uint32_t base_addr);
void aplic_deleg_intp (uint8_t intp_id);
void aplic_clr_intp(uint32_t base_addr);
#endif