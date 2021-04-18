//
// Created by xj on 2021/3/30.
//

#ifndef _DEEP_INTERP_H
#define _DEEP_INTERP_H
#include "deep_loader.h"

//帧
struct DEEPInterpFrame {  //DEEP帧
    struct DEEPInterpFrame *prev_frame;//指向前一个帧
    struct DEEPFunction *function;//当前函数实例
    int *sp;  //操作数栈指针
} DEEPInterpFrame;

//操作数栈
typedef struct DEEPStack{
    int capacity;
    int *sp;
    int *sp_end;
} DEEPStack;


typedef struct DEEPExecEnv {

    struct DEEPInterpFrame* cur_frame;//当前函数帧
    int *sp_end;//操作数栈大小
    int *sp;//sp指针
} DEEPExecEnv;


//创建操作数栈
DEEPStack *stack_cons(void);
int call_main(DEEPExecEnv* current_env, DEEPModule* module);
#endif /* _DEEP_INTERP_H */


