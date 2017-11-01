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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include <sys/time.h>
#include <ctype.h>

/*#include "fmacros.h"*/
#include "dict.h"

/* Using dictEnableResize() / dictDisableResize() we make possible to
 * enable/disable resizing of the hash table as needed. This is very important
 * for Redis, as we use copy-on-write and don't want to move too much memory
 * around when there is a child performing saving operations.
 *
 * Note that even when dict_can_resize is set to 0, not all resizes are
 * prevented: an hash table is still allowed to grow if the ratio between
 * the number of elements and the buckets > dict_force_resize_ratio. */
static int dict_can_resize = 1;
static unsigned int dict_force_resize_ratio = 5;

/****************memory pool*************************/

/* -------------------------- private prototypes ---------------------------- */

static int _dict_expand_if_needed(dict *ht);
static unsigned long _dict_next_power(unsigned long size);
static int _dict_key_index(dict * ht, const void *key, int keylen);
static int _dict_init(dict *ht, void *priv_data_ptr);

/* -------------------------- hash functions -------------------------------- */

/* Thomas Wang's 32 bit Mix Function */
unsigned int dict_int_hash_function(unsigned int key)
{
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

/* Identity hash function for integer keys */
unsigned int dict_identity_hash_function(unsigned int key)
{
    return key;
}

/* Generic hash function (a popular one from Bernstein).
 * I tested a few and this was the best. */
unsigned int dict_gen_hash_function(const unsigned char *buf, int len)
{
    unsigned int hash = 5381;

    while (len--)
        hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
    return hash;
}

/* And a case insensitive version */
unsigned int dict_gen_case_hash_function(const unsigned char *buf, int len)
{
    unsigned int hash = 5381;

    while (len--)
        hash = ((hash << 5) + hash) + (tolower(*buf++));        /* hash * 33 + c */
    return hash;
}

/* ----------------------------- API implementation ------------------------- */

/* Reset an hashtable already initialized with ht_init().
 * NOTE: This function should only called by ht_destroy(). */
static void _dict_reset(dictht *ht)
{
    ht->table = NULL;
    ht->size = 0;
    ht->sizemask = 0;
    ht->used = 0;
}

/* Create a new hash table */
dict *dict_create(void *priv_data_ptr)
{
    dict *d = (dict*)malloc(sizeof(*d));

    _dict_init(d, priv_data_ptr);
    return d;
}

/* Initialize the hash table */
int _dict_init(dict *d, void *priv_data_ptr)
{
    _dict_reset(&d->ht[0]);
    _dict_reset(&d->ht[1]);
    d->privdata = priv_data_ptr;
    d->rehashidx = -1;
    d->iterators = 0;

    return DICT_OK;
}

/* Resize the table to the minimal size that contains all the elements,
 * but with the invariant of a USER/BUCKETS ratio near to <= 1 */
int dict_resize(dict *d)
{
    int minimal;

    if (!dict_can_resize || dict_is_rehashing(d))
        return DICT_ERR;
    minimal = d->ht[0].used;
    if (minimal < DICT_HT_INITIAL_SIZE)
        minimal = DICT_HT_INITIAL_SIZE;
    return dict_expand(d, minimal);
}

/* Expand or create the hashtable */
int dict_expand(dict *d, unsigned long size)
{
    dictht n;                   /* the new hashtable */
    unsigned long realsize = _dict_next_power(size);

    /* the size is invalid if it is smaller than the number of
     * elements already inside the hashtable */
    if (dict_is_rehashing(d) || d->ht[0].used > size)
        return DICT_ERR;

    /* Allocate the new hashtable and initialize all pointers to NULL */
    n.size = realsize;
    n.sizemask = realsize - 1;
    n.table = (dict_entry**)calloc(1, realsize * sizeof(dict_entry *));
    n.used = 0;

    /* Is this the first initialization? If so it's not really a rehashing
     * we just set the first hash table so that it can accept keys. */
    if (d->ht[0].table == NULL)
    {
        d->ht[0] = n;
        return DICT_OK;
    }

    /* Prepare a second hash table for incremental rehashing */
    d->ht[1] = n;
    d->rehashidx = 0;
    return DICT_OK;
}

/* Performs N steps of incremental rehashing. Returns 1 if there are still
 * keys to move from the old to the new hash table, otherwise 0 is returned.
 * Note that a rehashing step consists in moving a bucket (that may have more
 * thank one key as we use chaining) from the old to the new hash table. */
int dict_rehash(dict *d, int n)
{
    if (!dict_is_rehashing(d))
        return 0;

    while (n--)
    {
        dict_entry *de, *nextde;

        /* Check if we already rehashed the whole table... */
        if (d->ht[0].used == 0)
        {
            free(d->ht[0].table);
            d->ht[0] = d->ht[1];
            _dict_reset(&d->ht[1]);
            d->rehashidx = -1;
            return 0;
        }

        /* Note that rehashidx can't overflow as we are sure there are more
         * elements because ht[0].used != 0 */
        while (d->ht[0].table[d->rehashidx] == NULL)
            d->rehashidx++;
        de = d->ht[0].table[d->rehashidx];
        /* Move all the keys in this bucket from the old to the new hash HT */
        while (de)
        {
            unsigned int h;

            nextde = de->next;
            /* Get the index in the new hash table */
            h = dict_gen_hash_function((const unsigned char*)de->key, de->keylen) & d->ht[1].sizemask;
            de->next = d->ht[1].table[h];
            d->ht[1].table[h] = de;
            d->ht[0].used--;
            d->ht[1].used++;
            de = nextde;
        }
        d->ht[0].table[d->rehashidx] = NULL;
        d->rehashidx++;
    }
    return 1;
}

long long time_in_milliseconds(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

/* Rehash for an amount of time between ms milliseconds and ms+1 milliseconds */
int dict_rehash_milliseconds(dict *d, int ms)
{
    long long start = time_in_milliseconds();
    int rehashes = 0;

    while (dict_rehash(d, 100))
    {
        rehashes += 100;
        if (time_in_milliseconds() - start > ms)
            break;
    }
    return rehashes;
}

/* This function performs just a step of rehashing, and only if there are
 * no safe iterators bound to our hash table. When we have iterators in the
 * middle of a rehashing we can't mess with the two hash tables otherwise
 * some element can be missed or duplicated.
 *
 * This function is called by common lookup or update operations in the
 * dictionary so that the hash table automatically migrates from H1 to H2
 * while it is actively used. */
static void _dict_rehash_step(dict *d)
{
    if (d->iterators == 0)
        dict_rehash(d, 1);
}

/* Add an element to the target hash table */
int dict_add(dict *d, void *key, int keylen, void *val, int vallen)
{
    int index;
    dict_entry *entry;
    dictht *ht;

    if (dict_is_rehashing(d))
        _dict_rehash_step(d);

    /* Get the index of the new element, or -1 if
     * the element already exists. */
    if ((index = _dict_key_index(d, key, keylen)) == -1)
    {
        return DICT_ERR;
    }

    /* Allocates the memory and stores key */
    ht = dict_is_rehashing(d) ? &d->ht[1] : &d->ht[0];
    entry = (dict_entry*)malloc(sizeof(*entry));

    entry->next = ht->table[index];
    ht->table[index] = entry;
    ht->used++;

    /* Set the hash entry fields. */
    entry->key = key;
    entry->keylen = keylen;
    entry->val = val;
    entry->vallen = vallen;

    return DICT_OK;
}

/* Add an element, discarding the old if the key already exists.
 * Return 1 if the key was added from scratch, 0 if there was already an
 * element with such key and dictReplace() just performed a value update
 * operation. */
int dict_replace(dict *d, void *key, int keylen, void *val, int vallen)
{
    dict_entry *entry, auxentry;

    /* Try to add the element. If the key
     * does not exists dictAdd will suceed. */
    if (dict_add(d, key, keylen, val, vallen) == DICT_OK)
        return 1;
    /* It already exists, get the entry */
    entry = dict_find(d, key, keylen);
    /* Free the old value and set the new one */
    /* Set the new value and free the old one. Note that it is important
     * to do that in this order, as the value may just be exactly the same
     * as the previous one. In this context, think to reference counting,
     * you want to increment (set), and then decrement (free), and not the
     * reverse. */
    auxentry = *entry;
    entry->val = val;
    entry->vallen = vallen;

    free(auxentry.val);

    return 0;
}

/* Search and remove an element */
static int dict_generic_delete(dict *d, const void *key, int keylen, int nofree)
{
    unsigned int h, idx;
    dict_entry *he, *prev_he;
    int table;

    if (d->ht[0].size == 0)
        return DICT_ERR;        /* d->ht[0].table is NULL */
    if (dict_is_rehashing(d))
        _dict_rehash_step(d);
    h = dict_gen_hash_function((const unsigned char*)key, keylen);

    unsigned int keylen_ = keylen;

    for (table = 0; table <= 1; table++)
    {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        prev_he = NULL;
        while (he)
        {
            //if (memcmp(key, he->key, keylen) == 0)
            if ((keylen_ == he->keylen) && (memcmp(key, he->key, keylen_) == 0))
            {
                /* Unlink the element from the list */
                if (prev_he)
                    prev_he->next = he->next;
                else
                    d->ht[table].table[idx] = he->next;
                if (!nofree)
                {
                    free(he->key);
                    free(he->val);
                }
                free(he);
                d->ht[table].used--;
                return DICT_OK;
            }
            prev_he = he;
            he = he->next;
        }
        if (!dict_is_rehashing(d))
            break;
    }
    return DICT_ERR;            /* not found */
}

int dict_delete(dict *ht, const void *key, int keylen)
{
    return dict_generic_delete(ht, key, keylen, 0);
}

int dict_delete_no_free(dict *ht, const void *key, int keylen)
{
    return dict_generic_delete(ht, key, keylen, 1);
}

/* Destroy an entire dictionary */
int _dict_clear(dict *d, dictht *ht)
{
    unsigned long i;

    /* Free all the elements */
    for (i = 0; i < ht->size && ht->used > 0; i++)
    {
        dict_entry *he, *next_he;

        if ((he = ht->table[i]) == NULL)
            continue;
        while (he)
        {
            next_he = he->next;

            free(he->key);
            free(he->val);
            free(he);

            ht->used--;
            he = next_he;
        }
    }
    /* Free the table and the allocated cache structure */
    free(ht->table);
    /* Re-initialize the table */
    _dict_reset(ht);

    return DICT_OK;             /* never fails */
}

/* Clear & Release the hash table */
void dict_release(dict *d)
{
    _dict_clear(d, &d->ht[0]);
    _dict_clear(d, &d->ht[1]);
    free(d);
}

dict_entry *dict_find(dict *d, const void *key, int keylen)
{
    dict_entry *he;
    unsigned int h, idx, table;

    if (d->ht[0].size == 0)
        return NULL;            /* We don't have a table at all */
    if (dict_is_rehashing(d))
        _dict_rehash_step(d);
    h = dict_gen_hash_function((const unsigned char*)key, keylen);

    unsigned int keylen_ = keylen;

    for (table = 0; table <= 1; table++)
    {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while (he)
        {
            if ((keylen_ == he->keylen) && (memcmp(key, he->key, keylen_) == 0))
            {
                return he;
            }
            he = he->next;
        }
        if (!dict_is_rehashing(d))
            return NULL;
    }
    return NULL;
}

void *dict_fetch_value(dict *d, const void *key, int keylen)
{
    dict_entry *he;

    he = dict_find(d, key, keylen);
    return he ? he->val : NULL;
}

dict_iterator *dict_get_iterator(dict *d)
{
    dict_iterator *iter = (dict_iterator*)malloc(sizeof(*iter));

    iter->d = d;
    iter->table = 0;
    iter->index = -1;
    iter->safe = 0;
    iter->entry = NULL;
    iter->next_entry = NULL;
    return iter;
}

dict_iterator *dict_get_safe_iterator(dict *d)
{
    dict_iterator *i = dict_get_iterator(d);

    i->safe = 1;
    return i;
}

dict_entry *dict_next(dict_iterator *iter)
{
    while (1)
    {
        if (iter->entry == NULL)
        {
            dictht *ht = &iter->d->ht[iter->table];
            if (iter->safe && iter->index == -1 && iter->table == 0)
                iter->d->iterators++;
            iter->index++;
            if (iter->index >= (signed) ht->size)
            {
                if (dict_is_rehashing(iter->d) && iter->table == 0)
                {
                    iter->table++;
                    iter->index = 0;
                    ht = &iter->d->ht[1];
                }
                else
                {
                    break;
                }
            }
            iter->entry = ht->table[iter->index];
        }
        else
        {
            iter->entry = iter->next_entry;
        }
        if (iter->entry)
        {
            /* We need to save the 'next' here, the iterator user
             * may delete the entry we are returning. */
            iter->next_entry = iter->entry->next;
            return iter->entry;
        }
    }
    return NULL;
}

void dict_release_iterator(dict_iterator *iter)
{
    if (iter->safe && !(iter->index == -1 && iter->table == 0))
        iter->d->iterators--;
    free(iter);
}

/* Return a random entry from the hash table. Useful to
 * implement randomized algorithms */
dict_entry *dict_get_randomKey(dict *d)
{
    dict_entry *he, *orighe;
    unsigned int h;
    int listlen, listele;

    if (dict_size(d) == 0)
        return NULL;
    if (dict_is_rehashing(d))
        _dict_rehash_step(d);
    if (dict_is_rehashing(d))
    {
        do
        {
            h = random() % (d->ht[0].size + d->ht[1].size);
            he = (h >= d->ht[0].size) ? d->ht[1].table[h - d->ht[0].size] :
                d->ht[0].table[h];
        }
        while (he == NULL);
    }
    else
    {
        do
        {
            h = random() & d->ht[0].sizemask;
            he = d->ht[0].table[h];
        }
        while (he == NULL);
    }

    /* Now we found a non empty bucket, but it is a linked
     * list and we need to get a random element from the list.
     * The only sane way to do so is counting the elements and
     * select a random index. */
    listlen = 0;
    orighe = he;
    while (he)
    {
        he = he->next;
        listlen++;
    }
    listele = random() % listlen;
    he = orighe;
    while (listele-- && he)
        he = he->next;
    return he;
}

/* ------------------------- private functions ------------------------------ */

/* Expand the hash table if needed */
static int _dict_expand_if_needed(dict *d)
{
    /* Incremental rehashing already in progress. Return. */
    if (dict_is_rehashing(d))
        return DICT_OK;

    /* If the hash table is empty expand it to the intial size. */
    if (d->ht[0].size == 0)
        return dict_expand(d, DICT_HT_INITIAL_SIZE);

    /* If we reached the 1:1 ratio, and we are allowed to resize the hash
     * table (global setting) or we should avoid it but the ratio between
     * elements/buckets is over the "safe" threshold, we resize doubling
     * the number of buckets. */
    if (d->ht[0].used >= d->ht[0].size &&
        (dict_can_resize ||
         d->ht[0].used / d->ht[0].size > dict_force_resize_ratio))
    {
        return dict_expand(d, ((d->ht[0].size > d->ht[0].used) ?
                              d->ht[0].size : d->ht[0].used) * 2);
    }
    return DICT_OK;
}

/* Our hash table capability is a power of two */
static unsigned long _dict_next_power(unsigned long size)
{
    unsigned long i = DICT_HT_INITIAL_SIZE;

    if (size >= LONG_MAX)
        return LONG_MAX;
    while (1)
    {
        if (i >= size)
            return i;
        i *= 2;
    }
}

/* Returns the index of a free slot that can be populated with
 * an hash entry for the given 'key'.
 * If the key already exists, -1 is returned.
 *
 * Note that if we are in the process of rehashing the hash table, the
 * index is always returned in the context of the second (new) hash table. */
static int _dict_key_index(dict *d, const void *key, int keylen)
{
    unsigned int h, idx, table;
    dict_entry *he;

    /* Expand the hashtable if needed */
    if (_dict_expand_if_needed(d) == DICT_ERR)
    {
        return -1;
    }
    /* Compute the key hash value */
    h = dict_gen_hash_function((const unsigned char*)key, keylen);

    for (table = 0; table <= 1; table++)
    {
        idx = h & d->ht[table].sizemask;
        /* Search if this slot does not already contain the given key */
        he = d->ht[table].table[idx];
        while (he)
        {
            if ((keylen == (int)he->keylen) && (memcmp(key, he->key, keylen) == 0))
            {
                return -1;
            }
            he = he->next;
        }
        if (!dict_is_rehashing(d))
            break;
    }

    return idx;
}

void dict_empty(dict *d)
{
    _dict_clear(d, &d->ht[0]);
    _dict_clear(d, &d->ht[1]);
    d->rehashidx = -1;
    d->iterators = 0;
}

#define DICT_STATS_VECTLEN 50
static void _dict_print_stats_ht(dictht *ht)
{
    unsigned long i, slots = 0, chainlen, maxchainlen = 0;
    unsigned long totchainlen = 0;
    unsigned long clvector[DICT_STATS_VECTLEN];

    if (ht->used == 0)
    {
        printf("No stats available for empty dictionaries\n");
        return;
    }

    for (i = 0; i < DICT_STATS_VECTLEN; i++)
        clvector[i] = 0;
    for (i = 0; i < ht->size; i++)
    {
        dict_entry *he;

        if (ht->table[i] == NULL)
        {
            clvector[0]++;
            continue;
        }
        slots++;
        /* For each hash entry on this slot... */
        chainlen = 0;
        he = ht->table[i];
        while (he)
        {
            chainlen++;
            he = he->next;
        }
        clvector[(chainlen <
                  DICT_STATS_VECTLEN) ? chainlen : (DICT_STATS_VECTLEN - 1)]++;
        if (chainlen > maxchainlen)
            maxchainlen = chainlen;
        totchainlen += chainlen;
    }
    printf("Hash table stats:\n");
    printf(" table size: %ld\n", ht->size);
    printf(" number of elements: %ld\n", ht->used);
    printf(" different slots: %ld\n", slots);
    printf(" max chain length: %ld\n", maxchainlen);
    printf(" avg chain length (counted): %.02f\n", (float) totchainlen / slots);
    printf(" avg chain length (computed): %.02f\n", (float) ht->used / slots);
    printf(" Chain length distribution:\n");
    for (i = 0; i < DICT_STATS_VECTLEN - 1; i++)
    {
        if (clvector[i] == 0)
            continue;
        printf("   %s%ld: %ld (%.02f%%)\n",
               (i == DICT_STATS_VECTLEN - 1) ? ">= " : "", i, clvector[i],
               ((float) clvector[i] / ht->size) * 100);
    }
}

void dict_print_stats(dict *d)
{
    _dict_print_stats_ht(&d->ht[0]);
    if (dict_is_rehashing(d))
    {
        printf("-- Rehashing into ht[1]:\n");
        _dict_print_stats_ht(&d->ht[1]);
    }
}

void dict_enable_resize(void)
{
    dict_can_resize = 1;
}

void dict_disable_resize(void)
{
    dict_can_resize = 0;
}

void *alloc_key(dict *d, int size)
{
    return malloc(size);
}

void *alloc_val(dict *d, int size)
{
    return malloc(size);
}

