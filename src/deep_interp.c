//
// Created by xj on 2021/3/30.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "deep_interp.h"
#include "deep_loader.h"
#include "deep_opcode.h"


#define popS32() (int32_t)*(--sp)
#define popF32() (float)*(--sp)
#define popU32() (uint32_t)*(--sp)
#define pushS32(x)  *(sp) = (int32_t)(x);sp++
#define pushF32(x) *(sp) = (float)(x);sp++ 
#define pushU32(x) *(sp) = (uint32_t)(x);sp++ 

#define STACK_CAPACITY 100

//创建操作数栈
DEEPStack* stack_cons(void)
{
    DEEPStack *stack = (DEEPStack *) malloc(sizeof(DEEPStack));
    stack->capacity = STACK_CAPACITY;
    stack->sp= (uint32_t*) malloc(sizeof(uint32_t) * STACK_CAPACITY);
    stack->sp_end = stack->sp + stack->capacity;
    return stack;
}

//执行代码块指令
void exec_instructions(DEEPExecEnv* env){
    uint32_t* sp = env->cur_frame->sp;
    uint8_t* ip = env->cur_frame->function->code_begin;
    uint8_t* ip_end = ip + env->cur_frame->function->code_size - 1;
    while(ip < ip_end){
        //提取指令码
        //立即数存在的话，提取指令码时提取立即数
        uint32_t opcode = (uint32_t)*ip;
        switch(opcode){
            case op_end:{
                ip++;
                break;
            }
            case i32_eqz:{
                ip++;
                uint32_t a = popU32();
                pushU32(a==0?1:0);
                break;
            }
            case i32_add:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a+b);
                break;
            }
            case i32_sub:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a-b);
                break;
            }
            case i32_mul:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a*b);
                break; 
            }
            case i32_divs:{
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(a/b);
                break; 
            }
            case i32_divu:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a/b);
                break;
            }
            case i32_const:{
                uint32_t temp = read_leb_u32(&ip);
                pushU32(temp);
                ip++;
                break;
            }
            case i32_rems:{
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(a%b);
                break;
            }
            case i32_and:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a&b);
                break;
            }
            case i32_or:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a|b);
                break;
            }
            case i32_xor:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a^b);
                break;
            }
            case i32_shl:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a<<(b%32));
                break;
            }
            case i32_shrs:
            case i32_shru:{
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a>>(b%32));
                break;
            }
            case f32_add:{
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(a+b);
                break;
            }
            case f32_sub:{
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(a-b);
                break;
            }
            case f32_mul:{
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(a*b);
                break;
            }
            case f32_div:{
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(a/b);
                break;
            }
            case f32_min:{
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(a<b?a:b);
                break;
            }
            case f32_max:{
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(a>b?a:b);
                break; 
            }
            case f32_copysign:{
                ip++;
                float a =popF32();
                float b =popF32();
                pushF32(copysign(a,b));
                break;
            }
            default:break;
        }
        //检查操作数栈是否溢出
        if(sp > env->sp_end){
            printf("%s","warning! Operand stack overflow!");
            break;
        }
    }
    //更新env
    env->sp=sp;
}

//为main函数创建帧，执行main函数
int32_t call_main(DEEPExecEnv* current_env, DEEPModule* module)
{

    //为main函数创建DEEPFunction
    DEEPFunction* main_func = module->func_section[4];//module.start_index记录了main函数索引

    //为main函数创建帧
    struct DEEPInterpFrame frame;
    //初始化
    frame.sp = current_env->sp;
    frame.function = main_func;

    //更新env中内容
    current_env->cur_frame = &frame;

    //执行frame中函数
    //sp要下移，栈顶元素即为函数参数
    exec_instructions(current_env);

    //返回栈顶元素
    return  *(current_env->sp-1);
}