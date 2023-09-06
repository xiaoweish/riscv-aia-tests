#include "apb_timer.h"

void apb_timer_set_compare(struct apb_timer* timer_ptr, uint32_t new_compare_val){
    write32((uintptr_t)&timer_ptr->compare, new_compare_val);
}

void apb_timer_init(struct apb_timer* timer_ptr){
    write32((uintptr_t)&timer_ptr->timer_ctl, 0);
    write32((uintptr_t)&timer_ptr->timer, 0);
    write32((uintptr_t)&timer_ptr->compare, 0);
}