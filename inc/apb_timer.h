#ifndef APB_TIMER_H
#define APB_TIMER_H

#include <rvh_test.h>
#include "page_tables.h" // PAGE_SIZE macro

#define APB_TIMER_BASE_ADDR 0x18000000

struct apb_timer{
    uint32_t timer;
    uint32_t timer_ctl;
    uint32_t compare;
}__attribute__((__packed__, aligned(PAGE_SIZE)));

static inline void apb_timer_start(struct apb_timer* timer_ptr){
    write32((uintptr_t)&timer_ptr->timer_ctl, 1);
}

static inline void apb_timer_stop(struct apb_timer* timer_ptr){
    write32((uintptr_t)&timer_ptr->timer_ctl, 0);
}

static inline uint32_t apb_timer_get_timer(struct apb_timer* timer_ptr){
    return read32((uintptr_t)&timer_ptr->timer);
}

void apb_timer_set_compare(struct apb_timer* timer_ptr, uint32_t new_compare_val);

void apb_timer_init(struct apb_timer* timer_ptr);

#endif //APB_TIMER_H