#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashtable.h"

//#define DEBUG

/* _A_
struct entry {
 char *original;
 void *target;
 struct entry *prev;
 struct entry *next;
};

struct hitem {
 struct entry *ll;
};

struct hashtable {
 unsigned int kl;
 unsigned int size;
 struct hitem **bucket;
 unsigned int *keys;
};
 _B_ */

/*stolen from wikipedia!*/
unsigned int murmur3_32(const char *key, unsigned int len, unsigned int seed) {
        static const uint32_t c1 = 0xcc9e2d51;
        static const uint32_t c2 = 0x1b873593;
        static const uint32_t r1 = 15;
        static const uint32_t r2 = 13;
        static const uint32_t m = 5;
        static const uint32_t n = 0xe6546b64;

        uint32_t hash = seed;

        const int nblocks = len / 4;
        const uint32_t *blocks = (const uint32_t *) key;
        const uint8_t *tail;
        uint32_t k1;
        int i;
        for (i = 0; i < nblocks; i++) {
                uint32_t k = blocks[i];
                k *= c1;
                k = (k << r1) | (k >> (32 - r1));
                k *= c2;

                hash ^= k;
                hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
        }

        tail = (const uint8_t *) (key + nblocks * 4);
        k1 = 0;

        switch (len & 3) {
        case 3:
                k1 ^= tail[2] << 16;
        case 2:
                k1 ^= tail[1] << 8;
        case 1:
                k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
        }

        hash ^= len;
        hash ^= (hash >> 16);
        hash *= 0x85ebca6b;
        hash ^= (hash >> 13);
        hash *= 0xc2b2ae35;
        hash ^= (hash >> 16);

        return hash;
}

unsigned int hash(char *key) {
// return 0;//use this like a linked list.*key*strlen(key);
 return murmur3_32(key,strlen(key),0);
}

void inittable(struct hashtable *ht,unsigned int tsize) {
 int i;
 ht->bucket=malloc(sizeof(char *)*tsize);
 ht->kl=0;
 ht->keys=malloc(sizeof(int *)*tsize);
 ht->size=tsize;
 if(!ht) {
  fprintf(stderr,"malloc error 6 in hash table.\n");//dafuq?
  return;
 }
 for(i=0;i<tsize;i++) {
  ht->bucket[i]=0;
 }
}

void ll_delete(struct entry *ll) {
 if(!ll) return;
 if(ll->prev) ll->prev->next=ll->next;
 if(ll->next) ll->next->prev=ll->prev;
 free(ll);//all these nodes are malloc()d.
}

void ll_destroy(struct entry *ll) {
 if(ll->next) ll_destroy(ll->next);
 free(ll->original);
 free(ll);
}

void ht_destroy(struct hashtable *ht) {
 int i=0;
 for(i=0;i<ht->kl;i++) {
  ll_destroy(ht->bucket[ht->keys[i]]->ll);
 }
 free(ht->bucket);
}

void ll_freevalues(struct entry *ll) {//only use if you malloc your table.
 if(ll->next) ll_destroy(ll->next);
 free(ll->target);
}

void ht_freevalues(struct hashtable *ht) {
 int i;
 for(i=0;i<ht->kl;i++) {
  ll_freevalues(ht->bucket[ht->keys[i]]->ll);
 }
}

int ht_setkey(struct hashtable *ht,char *key,void *value) {
 unsigned int h;
 struct entry *tmp;
 int i;
 if(!key) key="(null)";
 h=hash(key)%(ht->size);
#ifdef DEBUG
 printf("setkey: %s\nhash: %u\n",key,h);
#endif
 for(i=0;i<ht->kl;i++) {
  if(ht->keys[i]==h) break;
 }
 ht->keys[i]=h;
 ht->kl=(ht->kl)>i+1?ht->kl:i+1;
 if(!ht->bucket[h]) { //empty bucket!
  //add this to the list of used buckets so we can easily
  //use that list later for stuff.
  if(!(ht->bucket[h]=malloc(sizeof(struct hitem)))) return 1; 
  ht->bucket[h]->ll=0;
  //we now have a valid hashtable entry and a NULL ll in it.
  //don't bother with the new ll entry yet...
 }
 if((tmp=ll_getentry(ht->bucket[h]->ll,key)) != NULL) {
  tmp->target=value;
  return 0;
 }
 if(ht->bucket[h]->ll == NULL) {
  if(!(ht->bucket[h]->ll=malloc(sizeof(struct entry)))) return 3;
  ht->bucket[h]->ll->next=0;
  ht->bucket[h]->ll->prev=0;
  if(!(ht->bucket[h]->ll->original=strdup(key))) return 4;
  ht->bucket[h]->ll->target=value;
 } else {
  //go to the end and add another entry to the ll.
  for(tmp=ht->bucket[h]->ll;tmp->next;tmp=tmp->next);
  if(!(tmp->next=malloc(sizeof(struct entry)))) return 6;
  tmp->next->prev=tmp;
  tmp=tmp->next;
  if(!(tmp->original=strdup(key))) return 7;
  tmp->target=value;
  tmp->next=0;
 }
 return 0;
}

struct entry *ll_getentry(struct entry *start,char *key) {
 struct entry *m;
 if(!key) return 0;
 if(!start) return 0;
 for(m=start;m;m=m->next) {
  if(!strcmp(key,m->original)) {
   return m;
  }
 }
 return 0;
}

//returns the table entry (a linked list) at the key.
struct entry *ht_getentry(struct hashtable *ht,char *key) {
 unsigned int h=hash(key)%(ht->size);
 if(!ht->bucket[h]) return NULL;
 return ht->bucket[h]->ll;
}

//returns the node in the linked list in the table entry that matches the key.
struct entry *ht_getnode(struct hashtable *ht,char *key) {
 return ll_getentry(ht_getentry(ht,key),key);
}

//you'll probably want to use me.
void *ht_getvalue(struct hashtable *ht,char *key) {
 struct entry *tmp=ll_getentry(ht_getentry(ht,key),key);
#ifdef DEBUG
 printf("getvalue: %s\nhash: %u\n",key,hash(key)%(ht->size));
#endif
 return tmp?tmp->target:0;
}

struct hitem *ht_getbucket(struct hashtable *ht,char *key) {
 return ht->bucket[hash(key)%ht->size];
}

//delete the node in the linked list in the table entry that matches the key.
void ht_delete(struct hashtable *ht,char *key) {
 int i;
 unsigned int h=hash(key)%(ht->size);

//working on this stuff:
 struct hitem *bucket=ht_getbucket(ht,key);
 struct entry *first=ht_getentry(ht,key);
 struct entry *tmp=ll_getentry(first,key);

 if(!(tmp->next || tmp->prev)) bucket->ll=0;
 else ll_delete(tmp);

 for(i=0;i<ht->kl;i++) {
  if(ht->keys[i]==h) break;
 }
 for(;i<ht->kl;i++) {
  ht->keys[i]=ht->keys[i+1];
 }
 ht->kl--;
}
