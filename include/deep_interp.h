//
// Created by xj on 2021/3/30.
//

#ifndef _DEEP_INTERP_H
#define _DEEP_INTERP_H

#include "deep_loader.h"

#define MAX_STACK_SIZE 100
#define MAX_GLOBAL_COUNT 100

extern AnyData operand_stack[MAX_STACK_SIZE];
extern int32_t sp = 0;
extern uint8_t* memory;
extern AnyData global_vars[MAX_GLOBAL_COUNT];

typedef struct AnyData {
    uint8_t data_type;
    union {
        uint32_t u32_val;
        int32_t i32_val;
        float f32_val;
    } value;
} AnyData;

typedef struct DEEPFrame {
    struct DEEPFrame* perv_frm;
    uint8_t* ip;        //指令指针
    uint8_t* ip_end; 
    AnyData* local_vars;//局部变量
} DEEPFrame;

//创建操作数栈
int32_t find_main_index(DEEPModule *module);

DEEPFrame* init_func(uint32_t index, DEEPModule *module);

void exec_instructions(DEEPFrame* cur_frame, DEEPModule *module, uint8_t* memory);

AnyData pop(void);

#endif /* _DEEP_INTERP_H */


