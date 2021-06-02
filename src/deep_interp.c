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
#define READ_FLOAT(p)  READ_VALUE(float, p)
#define READ_UINT32(p)  READ_VALUE(uint32_t, p)
#define READ_BYTE(p) READ_VALUE(uint8_t, p)


#define MAX_STACK_SIZE 100
#define MAX_GLOBAL_COUNT 100

extern AnyData operand_stack[MAX_STACK_SIZE];
extern int32_t sp;
extern uint8_t *memory;
extern AnyData global_vars[MAX_STACK_SIZE];

void pushU32(uint32_t s) {
    struct AnyData data;
    data.data_type = 0;
    data.value.u32_val = s;
    operand_stack[sp++] = data;
    return;
}

void pushS32(int32_t s) {
    struct AnyData data;
    data.data_type = 1;
    data.value.i32_val = s;
    operand_stack[sp++] = data;
    return;
}

void pushF32(float s) {
    struct AnyData data;
    data.data_type = 2;
    data.value.f32_val = s;
    operand_stack[sp++] = data;
    return;
}


int popU32() {
    return operand_stack[--sp].value.u32_val;
}

int popS32() {
    return operand_stack[--sp].value.i32_val;
}

float popF32() {
    return operand_stack[--sp].value.f32_val;
}


//执行代码块指令
void exec_instructions(DEEPFrame *cur_frame, DEEPModule *module) {
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
                DEEPFrame *frame_new = init_func(func_index, module, cur_frame);
                exec_instructions(frame_new, module);
                free(frame_new->local_vars);
                free(frame_new);
                break;
            }
                //内存指令
            case i32_load: {
                ip++;
                uint32_t base = popU32();
                uint32_t align = read_leb_u32(&ip);
                ip++;
                uint32_t offset = read_leb_u32(&ip);
                uint32_t number = read_mem32(memory + base, offset);
                pushU32(number);
            }
            case i32_store: {
                ip++;
                uint32_t base = popU32();
                uint32_t align = read_leb_u32(&ip);
                ip++;
                uint32_t offset = read_leb_u32(&ip);
                uint32_t number = popU32();
                write_mem32(memory + base, number, offset);
            }
            case op_local_get: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_get指令的立即数
                AnyData temp = cur_frame->local_vars[index];
                operand_stack[sp++] = temp;
                break;
            }
            case op_local_set: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_set指令的立即数
                cur_frame->local_vars[index] = operand_stack[--sp];
                break;
            }
            case op_local_tee: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_tee指令的立即数
                AnyData temp = operand_stack[sp - 1];
                cur_frame->local_vars[index] = temp;
                break;
            }
            case op_global_get: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//global_get指令的立即数
                AnyData temp = global_vars[index];
                operand_stack[sp++] = temp;
                break;
            }
            case op_global_set: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//global_set指令的立即数
                global_vars[index] = operand_stack[--sp];
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
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(a + b);
                break;
            }
            case i32_sub: {
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b - a);
                break;
            }
            case i32_mul: {
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b * a);
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
                int32_t temp = read_leb_i32(&ip);
                pushS32(temp);
                break;
            }
            case f32_const: {
                ip++;
                float temp = READ_FLOAT(ip);
                pushF32(temp);
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
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b & a);
                break;
            }
            case i32_or: {
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b | a);
                break;
            }
            case i32_xor: {
                ip++;
                int32_t a = popS32();
                int32_t b = popS32();
                pushS32(b ^ a);
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
            case op_i32_turnc_s_f32: {
                ip++;
                float a = popF32();
                pushS32((int) a);
                break;
            }
            default:
                break;
        }
        //检查操作数栈是否溢出
        if (sp >= MAX_STACK_SIZE) {
            printf("warning! Operand stack overflow!\r\n");
            return;
        }
    }
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
DEEPFrame *init_func(uint32_t index, DEEPModule *module, DEEPFrame *pre_frame) {
    DEEPFrame *cur_frame = (DEEPFrame *) malloc(sizeof(DEEPFrame));
    cur_frame->perv_frm = pre_frame;
    cur_frame->ip = module->func_section[index]->code_begin;
    cur_frame->ip_end = cur_frame->ip + module->func_section[index]->code_size - 1;
    //TODO:局部变量的初始化

    //找到函数对应DEEPFunction
    DEEPFunction *func = module->func_section[index];

    //函数类型
    DEEPType *deepType = func->func_type;
    int param_num = deepType->param_count;
    int ret_num = deepType->ret_count;
    //    current_env->sp-=param_num;//操作数栈指针下移
    cur_frame->local_vars = (AnyData *) malloc(
            sizeof(AnyData) * param_num);
    int vars_temp = param_num;
    while (vars_temp > 0) {
        AnyData temp = operand_stack[--sp];
        cur_frame->local_vars[--vars_temp] = temp;
    }

    return cur_frame;
}

