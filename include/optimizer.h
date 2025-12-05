#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include "def.h"
#include "opcode.h"

#include <stdint.h>

#define PEEPHOLE_SIZE 10

struct _peephole_t{
    uint64_t* btcode_cur;
    uint64_t* btcode_end;
    uint64_t* window[PEEPHOLE_SIZE];
    int       window_cur;
};

typedef struct _peephole_t peephole_t;

peephole_t InitPeephole(context_t ctx);
OpCode     PeepholeOpGet(peephole_t peep, int at);
uint64_t   PeepholeOpSet(peephole_t peep, int at, OpCode op);
uint64_t   PeepholeInsSet(peephole_t peep, int at, uint64_t ins);
void       PeepholeAdvance(peephole_t* peep);

void optimize(context_t ctx);

#endif//__OPTIMIZER_H__
