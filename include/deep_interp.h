//
// Created by xj on 2021/3/30.
//

#ifndef _DEEP_INTERP_H
#define _DEEP_INTERP_H
#define MAX_STACK_SIZE 100


#include "deep_loader.h"

typedef struct AnyData {
    uint8_t data_type;
    union {
        uint32_t u32_val;
        int32_t i32_val;
        float f32_val;
    } value;
} AnyData;

typedef struct DEEPFrame {
    struct DEEPFrame *perv_frm;
    uint8_t *ip;        //指令指针
    uint8_t *ip_end;
    AnyData *local_vars;//局部变量
} DEEPFrame;

//创建操作数栈
int32_t find_main_index(DEEPModule *module);

DEEPFrame *init_func(uint32_t index, DEEPModule *module, DEEPFrame *pre_frame);

void exec_instructions(DEEPFrame *cur_frame, DEEPModule *module);

AnyData pop(void);

void call_function(DEEPModule *module, int func_index, DEEPFrame *pre_frame, int32_t *sp, AnyData *operand_stack,
                   uint8_t *memory);

#endif /* _DEEP_INTERP_H */


