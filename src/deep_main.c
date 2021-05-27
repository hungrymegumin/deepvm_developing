
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "deep_interp.h"
#include "deep_loader.h"
#include "deep_log.h"

#define WASM_FILE_SIZE 1024
#define MAX_STACK_SIZE 100
#define MAX_GLOBAL_COUNT 100

AnyData operand_stack[MAX_STACK_SIZE];
int32_t sp = 0;
uint8_t* memory;
AnyData global_vars[MAX_GLOBAL_COUNT];

int32_t main(int argv, char **args) {
    char *path;
    if(argv==1){
        error("no file input!");
    }else{
        path = args[1];
    }
    uint8_t *q = (uint8_t *) malloc(WASM_FILE_SIZE);
    uint8_t *p = q;
    if (p == NULL) {
        error("malloc fail.");
        return -1;
    }

    FILE *fp = fopen(path, "rb"); /* read wasm file with binary mode */
    if (fp == NULL) {
        error("file open fail.");
        return -1;
    }
    int32_t size = fread(p, 1, WASM_FILE_SIZE, fp);
    if (size == 0) {
        error ("fread faill.");
        return -1;
    }
    DEEPModule *module = deep_load(&p, size);
    if (module == NULL) {
        error("load fail.");
        return -1;
    }

    //初始化虚拟机环境
    memory = init_memory(1);
    int32_t main_index = find_main_index(module);
    DEEPFrame* cur_frame = init_func(main_index, module);
    exec_instructions(cur_frame, module, memory);
    AnyData A = pop();
    AnyData ans = operand_stack[sp];
    printf("%d\n", ans.value);
    fflush(stdout);

    /* release memory */
    fclose(fp);
    free(cur_frame);
    free(module);
    return 0;
}
