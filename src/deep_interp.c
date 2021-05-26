//
// Created by xj on 2021/3/30.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "deep_interp.h"
#include "deep_loader.h"
#include "deep_opcode.h"

#define READ_VALUE(Type, p) \
    (p += sizeof(Type), *(Type*)(p - sizeof(Type)))
#define READ_UINT32(p)  READ_VALUE(uint32_t, p)
#define READ_BYTE(p) READ_VALUE(uint8_t, p)

//执行代码块指令
void exec_instructions(DEEPFrame* cur_frame, DEEPModule *module, uint8_t* memory) {
    uint8_t *ip = cur_frame->ip;
    uint8_t *ip_end = cur_frame->ip_end;
    while (ip < ip_end) {
        //提取指令码
        //立即数存在的话，提取指令码时提取立即数
        uint32_t opcode = (uint32_t) *ip;
        switch (opcode) {
            case op_end: {
                ip++;
                break;
            }
            case op_call: {
                ip++;
                uint32_t func_index = read_leb_u32(&ip);//被调用函数index
                //需要一个同步操作
                current_env->sp = sp;
                call_function(current_env, module, func_index);
                sp = current_env->sp;
                break;
            }
            //内存指令
            case i32_load:{
                ip++;
                uint32_t base = popU32();
                uint32_t align = read_leb_u32(&ip);
                ip++;
                uint32_t offset = read_leb_u32(&ip);
                uint32_t number = read_mem32(memory+base,offset);
                pushU32(number);
            }
            case i32_store:{
                ip++;
                uint32_t base = popU32();
                uint32_t align = read_leb_u32(&ip);
                ip++;
                uint32_t offset = read_leb_u32(&ip);
                uint32_t number = popU32();
                write_mem32(memory+base,number,offset);
            }
            case op_local_get: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_get指令的立即数
                uint32_t a = current_env->local_vars[index];
                pushU32(a);
                break;
            }
            case op_local_set: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_set指令的立即数
                current_env->local_vars[index] = popU32();
                break;
            }
            case op_local_tee: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_tee指令的立即数
                uint32_t num = *(sp - 1);
                current_env->local_vars[index] = num;
                break;
            }
            case op_global_get: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//global_get指令的立即数
                uint32_t a = current_env->global_vars[index];
                pushU32(a);
                break;
            }
            case op_global_set: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//global_set指令的立即数
                current_env->global_vars[index] = popU32();
                break;
            }


                //算术指令
            case i32_eqz: {
                ip++;
                uint32_t a = popU32();
                pushU32(a == 0 ? 1 : 0);
                break;
            }
            case i32_add: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(a + b);
                break;
            }
            case i32_sub: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b - a);
                break;
            }
            case i32_mul: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b * a);
                break;
            }
            case i32_divs: {
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b / a);
                break;
            }
            case i32_divu: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b / a);
                break;
            }
            case i32_const: {
                ip++;
                uint32_t temp = read_leb_u32(&ip);
                pushU32(temp);
                break;
            }
            case i32_rems: {
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b % a);
                break;
            }
            case i32_and: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b & a);
                break;
            }
            case i32_or: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b | a);
                break;
            }
            case i32_xor: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b ^ a);
                break;
            }
            case i32_shl: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b << (a % 32));
                break;
            }
            case i32_shrs:
            case i32_shru: {
                ip++;
                uint32_t a = popU32();
                uint32_t b = popU32();
                pushU32(b >> (a % 32));
                break;
            }
            case f32_add: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(b + a);
                break;
            }
            case f32_sub: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(b - a);
                break;
            }
            case f32_mul: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(b * a);
                break;
            }
            case f32_div: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(b / a);
                break;
            }
            case f32_min: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(b < a ? b : a);
                break;
            }
            case f32_max: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(b > a ? b : a);
                break;
            }
            case f32_copysign: {
                ip++;
                float a = popF32();
                float b = popF32();
                pushF32(copysign(b, a));
                break;
            }
            default:
                break;
        }
        //检查操作数栈是否溢出
        if (sp > current_env->sp_end) {
            printf("warning! Operand stack overflow!\r\n");
            return;
        }
    }
    //更新env
    current_env->sp = sp;
}

//找到main函数的索引
int32_t find_main_index(DEEPModule *module) {
    int32_t main_index = -1;
    int32_t export_count = module->export_count;
    for (int32_t i = 0; i < export_count; i++) {
        if (strcmp((module->export_section[i])->name, "main") == 0) {
            main_index = module->export_section[i]->index;
            return main_index;
        }
    }
    return main_index;
}

//初始化某个函数，主要是初始化ip指针，返回栈帧
DEEPFrame* init_func(uint32_t index, DEEPModule *module) {
    DEEPFrame* cur_frame = (DEEPFrame*)malloc(sizeof(DEEPFrame));
    cur_frame->perv_frm = NULL;
    cur_frame->ip = module->func_section[index]->code_begin;
    cur_frame->ip_end = cur_frame->ip + module->func_section[index]->code_size - 1;
    //TODO:局部变量的初始化
    return cur_frame;
}