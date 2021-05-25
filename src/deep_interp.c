//
// Created by xj on 2021/3/30.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "deep_interp.h"
#include "deep_loader.h"
#include "deep_opcode.h"

int popS32(AnyData **sp) {
    --(*sp);
    return (**sp).value.m_intval;
}

float popF32(AnyData **sp) {
    --(*sp);
    return (**sp).value.m_floatval;
}

float popU32(AnyData **sp) {
    --(*sp);
    return (**sp).value.m_intval;
}

void pushS32(int x, AnyData **sp) {
    struct AnyData data;
    data.m_datatype = 1;
    data.value.m_intval = x;
    (**sp) = data;
    (*sp)++;
}

void pushF32(float x, AnyData **sp) {
    struct AnyData data;
    data.m_datatype = 2;
    data.value.m_floatval = x;
    (**sp) = data;
    (*sp)++;
}

void pushU32(int x, AnyData **sp) {
    struct AnyData data;
    data.m_datatype = 0;
    data.value.m_uintval = x;
    (**sp) = data;
    (*sp)++;
}

#define READ_VALUE(Type, p) \
    (p += sizeof(Type), *(Type*)(p - sizeof(Type)))
#define READ_UINT32(p)  READ_VALUE(uint32_t, p)
#define READ_BYTE(p) READ_VALUE(uint8_t, p)

#define STACK_CAPACITY 100


//创建操作数栈
DEEPStack *stack_cons(void) {
    DEEPStack *stack = (DEEPStack *) malloc(sizeof(DEEPStack));
    if (stack == NULL) {
        printf("Operand stack creation failed!\r\n");
        return NULL;
    }
    stack->capacity = STACK_CAPACITY;
    stack->sp = (AnyData *) malloc(sizeof(AnyData) * STACK_CAPACITY);
    if (stack->sp == NULL) {
        printf("Malloc area for stack error!\r\n");
    }
    stack->sp_end = stack->sp + stack->capacity * sizeof(AnyData);
    return stack;
}


//求幂
double mypow(double num, double n) {
    double value = 1;
    int i = 1;
    if (n == 0) {
        value = 1;
    } else {
        if (n > 0) {
            while (i++ <= n) {
                value *= num;
            }
        } else {
            while (i++ <= -n) {
                value *= num;
            }
            value = 1 / value;
        }
    }
    return value;
}

//二进制浮点数转十进制
float binaryToDigital(double t) {
    char binary[100];
    sprintf(binary, "%.15f", t);
    char ch;
    int integer = 0;
    float decimal = 0.0;
    int i = 0, integerNum = 0, decimalNum = -1;
    bool hasDecimal = false;

    // 计算整数和小数所占位数
    for (; i < 30; ++i) {
        ch = binary[i];
        if (ch == 0) {
            break;
        }
        if (!hasDecimal && ch != '.') {
            ++integerNum;
        } else {
            hasDecimal = true;
            ++decimalNum;
        }
    }

    // 计算整数部分
    i = integerNum;
    for (; i > 0; --i) {
        if (binary[i - 1] == '1') {
            integer += mypow(2, integerNum - i);
        }
    }

    // 计算小数部分
    if (hasDecimal) {
        i = integerNum + 1;
        for (; i <= integerNum + decimalNum; ++i) {
            if (binary[i] == '1') {
                decimal += (float) mypow(2, integerNum - i);
            }
        }
        return ((float) integer + decimal);
    } else {
        return (float) integer;
    }

}

//十进制转二进制
int convertBinary(int t) {
    int result = 0;//存储a的二进制结果。
    int p = 1;//p=1表示个位数
    do {
        int b = t % 2;//b是余数,第一个余数是二进制的个位。
        result = result + p * b;
        p = p * 10;//*10表示下个是10位数。
        t = t / 2;
    } while (t);
    return result;
}

//执行代码块指令
void exec_instructions(DEEPExecEnv *current_env, DEEPModule *module) {
    AnyData *sp = current_env->sp;
    uint8_t *ip = current_env->cur_frame->function->code_begin;
    uint8_t *ip_end = ip + current_env->cur_frame->function->code_size - 1;
    uint32_t *memory = current_env->memory;
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
            case i32_load: {
                ip++;
                uint32_t base = popU32(&sp);
                uint32_t align = read_leb_u32(&ip);
                ip++;
                uint32_t offset = read_leb_u32(&ip);
                uint32_t number = read_mem32(memory + base, offset);
                pushU32(number, &sp);
                break;
            }
            case i32_store: {
                ip++;
                uint32_t base = popU32(&sp);
                uint32_t align = read_leb_u32(&ip);
                ip++;
                uint32_t offset = read_leb_u32(&ip);
                uint32_t number = popU32(&sp);
                write_mem32(memory + base, number, offset);
                break;
            }
            case op_local_get: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_get指令的立即数
                *(sp++) = current_env->local_vars[index];
                break;
            }
            case op_local_set: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_set指令的立即数
                current_env->local_vars[index] = (*(--sp));
                break;
            }
            case op_local_tee: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//local_tee指令的立即数
                uint32_t num = (*(sp - 1)).value.m_intval;
                struct AnyData data;
                data.m_datatype = 0;
                data.value.m_intval = num;
                current_env->local_vars[index] = data;
                break;
            }
            case op_global_get: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//global_get指令的立即数
                uint32_t a = current_env->global_vars[index].value.m_intval;
                pushU32(a, &sp);
                break;
            }
            case op_global_set: {
                ip++;
                uint32_t index = read_leb_u32(&ip);//global_set指令的立即数
                current_env->global_vars[index] = (*(--sp));
                break;
            }

                //算术指令
            case i32_eqz: {
                ip++;
                uint32_t a = popU32(&sp);
                pushU32(a == 0 ? 1 : 0, &sp);
                break;
            }
            case i32_add: {
                ip++;
                int32_t a = popS32(&sp);
                int32_t b = popS32(&sp);
                if (b > 2147483647 - a) {
                    fprintf(stderr, "Add result overflow!\n");
                    return;
                }
                pushS32(a + b, &sp);
                break;
            }
            case i32_sub: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b - a, &sp);
                break;
            }
            case i32_mul: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b * a, &sp);
                break;
            }
            case i32_divs: {
                ip++;
                int32_t a = popS32(&sp);
                int32_t b = popS32(&sp);
                if (a == 0) {
                    fprintf(stderr, "Error! The divisor equals 0!\n");
                    return;
                }
                pushS32(b / a, &sp);
                break;
            }
            case i32_divu: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                if (a == 0) {
                    fprintf(stderr, "Error! The divisor equals 0!\n");
                    return;
                }
                pushU32(b / a, &sp);
                break;
            }
            case i32_const: {
                ip++;
                int32_t temp = read_leb_i32(&ip);
                pushS32(temp, &sp);
                break;
            }
            case f32_const: {
                ip++;
                uint32_t temp1 = READ_BYTE(ip);
                uint32_t temp2 = READ_BYTE(ip);
                uint32_t temp3 = READ_BYTE(ip);
                uint32_t temp4 = READ_BYTE(ip);
                int tag = temp4 >> 7;//符号标记位
                int index = ((temp4 - (tag << 7)) << 1) + (temp3 >> 7) - 127;//指数
                double t = convertBinary(temp3 - (temp3 >> 7 << 7)) / 1.0 / mypow(10, 7);
                t += convertBinary(temp2) / 1.0 / mypow(10, 15);
                t += convertBinary(temp1) / 1.0 / mypow(10, 23);
                float e = binaryToDigital((t + 1) * mypow(10, index));
                if (tag == 0) {
                    pushF32(e, &sp);
                } else {
                    pushF32(-e, &sp);
                }
                break;
            }
            case i32_rems: {
                ip++;
                int32_t a = popS32(&sp);
                int32_t b = popS32(&sp);
                pushS32(b % a, &sp);
                break;
            }
            case i32_and: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b & a, &sp);
                break;
            }
            case i32_or: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b | a, &sp);
                break;
            }
            case i32_xor: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b ^ a, &sp);
                break;
            }
            case i32_shl: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b << (a % 32), &sp);
                break;
            }
            case i32_shrs:
            case i32_shru: {
                ip++;
                uint32_t a = popU32(&sp);
                uint32_t b = popU32(&sp);
                pushU32(b >> (a % 32), &sp);
                break;
            }
            case f32_add: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(b + a, &sp);
                break;
            }
            case f32_sub: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(b - a, &sp);
                break;
            }
            case f32_mul: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(b * a, &sp);
                break;
            }
            case f32_div: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(b / a, &sp);
                break;
            }
            case f32_min: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(b < a ? b : a, &sp);
                break;
            }
            case f32_max: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(b > a ? b : a, &sp);
                break;
            }
            case f32_copysign: {
                ip++;
                float a = popF32(&sp);
                float b = popF32(&sp);
                pushF32(copysign(b, a), &sp);
                break;
            }

            case op_i32_turnc_s_f32: {
                ip++;
                float a = popF32(&sp);
                pushS32((int) a, &sp);
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

//调用普通函数
void call_function(DEEPExecEnv *current_env, DEEPModule *module,
                   int func_index) {

    //为func函数创建DEEPFunction
    DEEPFunction *func = module->func_section[func_index];

    //函数类型
    DEEPType *deepType = func->func_type;
    int param_num = deepType->param_count;
    int ret_num = deepType->ret_count;

//    current_env->sp-=param_num;//操作数栈指针下移
    current_env->local_vars = (AnyData *) malloc(
            sizeof(AnyData) * param_num);
    int vars_temp = param_num;
    while (vars_temp > 0) {
        AnyData temp = *(--current_env->sp);
        current_env->local_vars[(vars_temp--) - 1] = temp;
    }

    //局部变量组
    LocalVars **locals = func->localvars;

    //为func函数创建帧
    DEEPInterpFrame *frame = (DEEPInterpFrame *) malloc(
            sizeof(DEEPInterpFrame));
    if (frame == NULL) {
        printf("Malloc area for normal_frame error!\r\n");
    }
    //初始化
    frame->function = func;
    frame->prev_frame = current_env->cur_frame;

    //更新env中内容
    current_env->cur_frame = frame;

    //执行frame中函数
    //sp要下移，栈顶元素即为函数参数
    exec_instructions(current_env, module);

    //执行完毕退栈
    current_env->cur_frame = frame->prev_frame;
    //释放掉局部变量
    free(current_env->local_vars);
    free(frame);
    return;
}

//为main函数创建帧，执行main函数
int32_t
call_main(DEEPExecEnv *current_env, DEEPModule *module) {

    //create DEEPFunction for main
    //find the index of main
    int main_index = -1;
    int export_count = module->export_count;
    for (int i = 0; i < export_count; i++) {
        if (strcmp((module->export_section[i])->name, "main") ==
            0) {
            main_index = module->export_section[i]->index;
        }
    }
    if (main_index < 0) {
        printf("the main function index failed!\r\n");
        return -1;
    }

    //为main函数创建DEEPFunction
    DEEPFunction *main_func = module->func_section[main_index];//module.start_index记录了main函数索引

    //为main函数创建帧
    DEEPInterpFrame *main_frame = (DEEPInterpFrame *) malloc(
            sizeof(struct DEEPInterpFrame));
    if (main_frame == NULL) {
        printf("Malloc area for main_frame error!\r\n");
    }
    //初始化
    main_frame->function = main_func;

    //更新env中内容
    current_env->cur_frame = main_frame;

    //执行frame中函数
    //sp要下移，栈顶元素即为函数参数
    exec_instructions(current_env, module);
//    free(main_frame);

    //返回栈顶元素
    return (*(current_env->sp - 1)).value.m_intval;
}