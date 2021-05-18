//
// Created by xj on 2021/3/30.
//

#ifndef _DEEP_INTERP_H
#define _DEEP_INTERP_H

#include "deep_loader.h"

//操作数栈
typedef struct AnyData {
    int m_datatype;//0表示int,1表示uint,2表示float
    union {
        uint32_t m_uintval;
        int32_t m_intval;
        float m_floatval;
    } value;
} AnyData;

typedef struct DEEPStack {
    int32_t capacity;
    AnyData **sp;
    AnyData **sp_end;
} DEEPStack;


//帧
typedef struct DEEPInterpFrame {  //DEEP帧
    struct DEEPInterpFrame *prev_frame;//指向前一个帧
    struct DEEPFunction *function;//当前函数实例
} DEEPInterpFrame;


typedef struct DEEPExecEnv {

    struct DEEPInterpFrame *cur_frame;//当前函数帧
    AnyData **sp_end;//操作数栈大小
    AnyData **sp;//sp指针
    AnyData **local_vars;//函数局部变量
    AnyData **global_vars;//全局变量
    uint8_t *memory;//内存
} DEEPExecEnv;


//创建操作数栈
DEEPStack *stack_cons(void);

int32_t call_main(DEEPExecEnv *current_env, DEEPModule *module);

void call_function(DEEPExecEnv *current_env, DEEPModule *module, int func_index);

#endif /* _DEEP_INTERP_H */


