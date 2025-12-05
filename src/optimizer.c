#include "optimizer.h"

// 常量折叠优化
void opt_constant_folding(context_t ctx){

}

// 死代码消除
void opt_dead_code(context_t ctx){

}

// 尾调用优化
void opt_tail_call(context_t ctx){

}

void optimize(context_t ctx){
    opt_constant_folding(ctx);
    opt_tail_call(ctx);
    opt_dead_code(ctx);
}
