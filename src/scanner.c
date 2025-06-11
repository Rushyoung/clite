#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "token.h"


static token_t advance(scanner sc, context_t ctx){
    token_t tk = {};
    if(sc->cur >= sc->src + sc->size){
        tk.tk = 0; // end of input
        return tk;
    }
    sc->pre = sc->cur;
    sc->cur++;
    switch(*sc->pre){
        case '\n':
            sc->line++;
        case '\r':
        case ' ':
            break;
        case '#':
            while(*sc->cur && *sc->cur != '\n'){
                sc->cur++;
            }
            break;
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':{
            uint32_t hash = (int)(*sc->pre);
            while((*sc->cur >= 'a' && *sc->cur <= 'z') ||
                (*sc->cur >= 'A' && *sc->cur <= 'Z') ||
                (*sc->cur >= '0' && *sc->cur <= '9') ||
                *sc->cur == '_'){
                hash = hash * 147 + *sc->cur;
                sc->cur++;
            }
            hash = (hash << 6) + (sc->cur - sc->pre);
            for(size_t i = 0; i < ctx->sym_idx; i++){
                if(ctx->sym[i].hash == hash && 
                strncmp(ctx->sym[i].name, sc->pre, sc->cur - sc->pre) == 0){
                    return ctx->sym[i]; // return existing identifier token
                }
            }
            tk.name = sc->pre;
            tk.hash = hash;
            tk.tk   = TK_ID;
            tk.len  = sc->cur - sc->pre;
            ctx->sym[ctx->sym_idx] = tk;
            ctx->sym_idx++;
            return tk; // return identifier token
        }
        case '0' ... '9':{
            int val = *sc->pre - '0';
            if(val){
                while(*sc->cur >= '0' && *sc->cur <= '9'){
                    val = val * 10 + *sc->cur - '0';
                    sc->cur++;
                }
            } else if(*sc->cur == 'x' || *sc->cur == 'X'){
                sc->cur++;
                while((*sc->cur >= '0' && *sc->cur <= '9') ||
                    (*sc->cur >= 'a' && *sc->cur <= 'f') ||
                    (*sc->cur >= 'A' && *sc->cur <= 'F')){
                    val = val * 16 + (*sc->cur & 15) + (*sc->cur >= 'A' ? 9 : 0);
                    sc->cur++;
                }
            } else {
                while(*sc->cur >= '0' && *sc->cur <= '7'){
                    val = val * 8 + *sc->cur - '0';
                    sc->cur++;
                }
            }
            tk.tk = TK_NUM;
            tk.val = val;
            return tk; // return number token
        }
        case '/':
            if(*sc->cur == '/'){
                sc->cur++;
                while(*sc->cur && *sc->cur != '\n') sc->cur++;
            } else {
                tk.tk = TK_DIV;
                return tk;
            }
            break;
        case '\'':
        case '"':
            tk.name = sc->cur;
            while(*sc->cur && *sc->cur != *sc->pre){
                sc->cur++;
            }
            sc->cur++;              // Skip over the closing quote
            if(*sc->pre == '"'){
                tk.tk = TK_STR;     // string token
                tk.len = sc->cur - sc->pre - 2; // exclude the quotes
            } else {
                tk.val = *tk.name; // character token
                tk.tk = TK_NUM; // character token
            }
            return tk;
        case '=':
            if(*sc->cur == '='){
                sc->cur++;
                tk.tk = TK_EQ;
            } else {
                tk.tk = TK_ASSIGN;
            }
            return tk;
        case '+':
            if(*sc->cur == '+'){
                sc->cur++;
                tk.tk = TK_INC;
            } else {
                tk.tk = TK_ADD;
            }
            return tk;
        case '-':
            if(*sc->cur == '-'){
                sc->cur++;
                tk.tk = TK_DEC;
            } else {
                tk.tk = TK_SUB;
            }
            return tk;
        case '!':
            if(*sc->cur == '='){
                sc->cur++;
                tk.tk = TK_NE;
            } else {
                tk.tk = TK_NOT;
            }
            return tk;
        case '<':
            if(*sc->cur == '='){
                sc->cur++;
                tk.tk = TK_LE;
            } else if(*sc->cur == '<'){
                sc->cur++;
                tk.tk = TK_SHL;
            } else {
                tk.tk = TK_LT;
            }
            return tk;
        case '>':
            if(*sc->cur == '='){
                sc->cur++;
                tk.tk = TK_GE;
            } else if(*sc->cur == '>'){
                sc->cur++;
                tk.tk = TK_SHR;
            } else {
                tk.tk = TK_GT;
            }
            return tk;
        case '&':
            if(*sc->cur == '&'){
                sc->cur++;
                tk.tk = TK_LAN; // logical and
            } else {
                tk.tk = TK_AND; // bitwise and
            }
            return tk;
        case '|':
            if(*sc->cur == '|'){
                sc->cur++;
                tk.tk = TK_LOR; // logical or
            } else {
                tk.tk = TK_OR;  // bitwise or
            }
            return tk;
        case '^':
            tk.tk = TK_XOR; // bitwise xor
            return tk;
        case '*':
            tk.tk = TK_MUL; // multiply
            return tk;
        case '%':
            tk.tk = TK_MOD; // modulo
            return tk;
        case '?':
            tk.tk = TK_COND; // conditional operator
            return tk;
        case '[':
            tk.tk = TK_LE_BRCKT; // left BRCKT
            return tk;
        case ']':
            tk.tk = TK_RI_BRCKT; // right BRCKT
            return tk;
        case '{':
            tk.tk = TK_LE_BRACE; // left brace
            return tk;
        case '}':
            tk.tk = TK_RI_BRACE; // right brace
            return tk;
        case '(':
            tk.tk = TK_LE_PAREN; // left parenthesis
            return tk;
        case ')':
            tk.tk = TK_RI_PAREN; // right parenthesis
            return tk;
        case ';':
            tk.tk = TK_SEMICOLON; // semicolon
            return tk;
        case ',':
            tk.tk = TK_COMMA; // comma
            return tk;
        case ':':
            tk.tk = TK_COLON; // colon
            return tk;
        default:
            printf("Unknown character '%c'(%d) at line %zu\n", *sc->pre, *sc->pre, sc->line);
            exit(EXIT_FAILURE);
    }
    return advance(sc, ctx);
}

void next(scanner sc, context_t ctx){
    sc->prv = sc->loc;
    sc->loc = advance(sc, ctx);
}

token_t prst(scanner sc){
    return sc->loc;
}

token_t prev(scanner sc){
    return sc->prv;
}


scanner InitScanner(size_t size, char* src){
    scanner sc = malloc(sizeof(struct _scanner));
    sc->size = size;
    sc->line = 1;
    sc->src = sc->cur = src;
    sc->pre = NULL;
    sc->loc = (token_t){0};
    sc->prv = (token_t){0};
    return sc;
}