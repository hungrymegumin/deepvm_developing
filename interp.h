//
// Created by xj on 2021/3/30.
//

#ifndef C___PROJECTS_INTERP_H
#define C___PROJECTS_INTERP_H

#endif //C___PROJECTS_INTERP_H

//帧
struct DEEPInterpFrame {  //DEEP帧

    struct DEEPInterpFrame *prev_frame;//指向前一个帧

    struct DEEPFunction *function;//当前函数实例

    int *sp;  //操作数栈指针

} DEEPInterpFrame;

//操作数栈
typedef struct DeepStack{
    int capacity;
    int *sp;
} DEEPStack;


typedef struct DEEPExecEnv {

    struct DEEPInterpFrame* cur_frame;//当前函数帧

    int deep_stack_size;//操作数栈大小

    int *sp;//sp指针

} DEEPExecEnv;


DeepStack stack_cons(){
    //创建操作数栈
    DeepStack stack;
    stack.capacity=100;
    stack.sp=(int*)malloc(sizeof(int));
    return stack;
}

