#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "hash_table.h"
#include "prime.h"
static ht_item HT_DELETED = {NULL, NULL};

static ht_item* new_ht_item(const char* k, const char* v){
    ht_item* n = malloc(sizeof(ht_item));
    n->key = strdup(k);
    n->value = strdup(v);
    return n;
}

static ht_hash_table* ht_new_sized(const int base_size){
    ht_hash_table* ht = malloc(sizeof(ht_hash_table));
    ht->base_size = base_size;
    ht->size = next_prime(base_size);
    ht->count = 0;
    ht->items = calloc(ht->size, sizeof(ht_item*));
    return ht;
}
//need to rename
ht_hash_table* ht_new(){
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}



static void del_ht_item(ht_item* i){
    free(i->key);
    free(i->value);
    free(i);
}

void ht_del_hash_table(ht_hash_table* ht){
    for(int i = 0; i < ht->size; i++){
        ht_item* item = ht->items[i];
        if(item && item != &HT_DELETED){
            del_ht_item(item);
        }
    }
    free(ht->items);
    free(ht);
}

//prime 大于字母表（ascii 128）大小的质数
static int ht_hash(const char* s, const int prime, const int m){
    int len = strlen(s);
    long hash = 0;
    for(int i =  0; i < len; i++){
        hash += (long)pow(prime, len - i - 1) * (int)s[i];
        hash %= m;
    }
    return (int)hash;
}

static int ht_get_hash(
    const char*s, const int num_buckets, const int attempt
){
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + attempt * (hash_b + 1)) % num_buckets;
}

void ht_insert(ht_hash_table *ht, const char *key, const char *value){
    const int load = ht->count * 100 / ht->size;
    if(load > 70){
        ht_resize(ht, ht->base_size * 2);
    }
    
    ht_item* item = new_ht_item(key, value);
    int index = ht_get_hash(key, ht->size, 0);
    ht_item* _item = ht->items[index];
    int i = 1;
    
    while(_item != NULL){
        if(_item != &HT_DELETED && strcmp(_item->key, key) == 0){
            del_ht_item(ht->items[index]);
            ht->items[index] = item;
            return;
        }
        index = ht_get_hash(key, ht->size, i);
        _item = ht->items[index];
        i++;
    }
    ht->items[index] = item;
    ht->count++;
}

char* ht_search(ht_hash_table *ht, const char *key) {
    int index = ht_get_hash(key, ht->size, 0);
    int i = 1;
    while (ht->items[index] != NULL) {
        if (ht->items[index] != &HT_DELETED && strcmp(ht->items[index]->key, key) == 0) {
            return ht->items[index]->value;
        }
        index = ht_get_hash(key, ht->size, i);
        i++;
    }
    return NULL;
}


void ht_delete(ht_hash_table *ht, const char *key){
    const int load = ht->count * 100 / ht->size;
    if(load < 10){
        ht_resize(ht, ht->base_size / 2);
    }
    
    int index = ht_get_hash(key, ht->size, 0);
    int i = 1;
    while(ht->items[index] != NULL){
        if(ht->items[index] != &HT_DELETED && strcmp(key, ht->items[index]->key) == 0){
            del_ht_item(ht->items[index]);
            ht->items[index] = &HT_DELETED;
            ht->count--;
            return;
        }
        index = ht_get_hash(key, ht->size, i);
        i++;
    }
}

static void ht_resize(ht_hash_table* ht, const int base_size){
    if(base_size < HT_INITIAL_BASE_SIZE){return;}

    ht_hash_table* ht_new = ht_new_sized(base_size);
    for(int i = 0; i < ht->size; i++){
        ht_item* item = ht->items[i];
        if(item != NULL && item != &HT_DELETED){
            ht_insert(ht_new, item->key, item->value);
        }
    }
    
    // 保存旧数据
    const int tmp_size = ht->size;
    ht->size = ht_new->size;
    ht->base_size = ht_new->base_size;
    
    const int tmp_count = ht->count;
    ht->count = ht_new->count;
    
    ht_item** tmp_items = ht->items;
    ht->items = ht_new->items;
    
    // 清理旧数据
    ht_new->items = tmp_items;
    ht_new->size = tmp_size;
    ht_new->count = tmp_count;
    ht_del_hash_table(ht_new);
}

int is_prime(const int x){
    if(x < 2){return -1;}
    if(x < 4){return 1;}
    if ((x % 2) == 0){return 0;}
    for(int i = 3; i * i <= x; i+=2){
        if(x % i == 0){
            return 0;
        }
    }
    return 1;
}