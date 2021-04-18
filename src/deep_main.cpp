
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "deep_interp.h"
#include "deep_loader.h"

int main() {
    const char* path = "program.wasm";
	uint8_t* p = (uint8_t *) malloc(1024);
    if (p == NULL) {
        printf("malloc fail.\r\n");
        return -1;
    }
	int file = open(path, O_RDONLY);
    if(file == -1) {
        printf("file open fail.\r\n");
        return -1;
    }
	int size = (int)read(file, p, 1024);
	DEEPModule* module = deep_load(p, size);
    if (module == NULL) {
        printf("load fail.\r\n");
        return -1;
    }
    //创建操作数栈
    DEEPStack *stack = stack_cons();
    //先声明环境并初始化
    DEEPExecEnv deep_env;
    DEEPExecEnv* current_env = &deep_env;
    current_env->sp_end = stack->sp_end;
    current_env->sp = stack->sp;
    int ans = call_main(current_env,module);
    printf("%d",ans);
    return 0;
}
