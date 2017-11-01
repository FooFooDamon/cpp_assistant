/* Hash Tables Implementation.
 *
 * This file implements in memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DICT_H
#define __DICT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DICT_OK 0
#define DICT_ERR 1


/* Unused arguments generate annoying warnings... */
#define DICT_NOTUSED(V) ((void) V)

typedef struct dict_entry {
    unsigned int keylen;
    unsigned int vallen;
    void *key;
    void *val;
    struct dict_entry *next;
} dict_entry;


/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
typedef struct dictht {
    dict_entry **table;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
} dictht;

typedef struct dict {
    void *privdata;
    dictht ht[2];
    int rehashidx;              /* rehashing not in progress if rehashidx == -1 */
    int iterators;              /* number of iterators currently running */
} dict;

/* If safe is set to 1 this is a safe iteartor, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
typedef struct dict_iterator {
    dict *d;
    int table, index, safe;
    dict_entry *entry, *next_entry;
} dict_iterator;

/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/

#define dict_get_entry_key(he) ((he)->key)
#define dict_get_entry_val(he) ((he)->val)
#define dict_slots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dict_size(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dict_is_rehashing(ht) ((ht)->rehashidx != -1)

/* API */
void *alloc_key(dict *d, int size);
void *alloc_val(dict *d, int size);

dict *dict_create(void *priv_data_ptr);
int dict_expand(dict *d, unsigned long size);
int dict_add(dict *d, void *key, int keylen, void *val, int vallen);
int dict_replace(dict *d, void *key, int keylen, void *val, int vallen);
int dict_delete(dict *d, const void *key, int keylen);
int dict_delete_no_free(dict *d, const void *key, int keylen);
void dict_release(dict *d);
dict_entry *dict_find(dict *d, const void *key, int keylen);
void *dict_fetch_value(dict *d, const void *key, int keylen);
int dict_resize(dict *d);
dict_iterator *dict_get_iterator(dict *d);
dict_iterator *dict_get_safe_iterator(dict *d);
dict_entry *dict_next(dict_iterator *iter);
void dict_release_iterator(dict_iterator *iter);
dict_entry *dict_get_random_key(dict *d);
void dict_print_stats(dict *d);
unsigned int dict_gen_hash_function(const unsigned char *buf, int len);
unsigned int dict_gen_case_hash_function(const unsigned char *buf, int len);
void dict_empty(dict *d);
void dict_enable_resize(void);
void dict_disable_resize(void);
int dict_rehash(dict *d, int n);
int dict_rehash_milliseconds(dict *d, int ms);

#ifdef __cplusplus
}
#endif

#endif                          /* __DICT_H */
