#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashtable.h"

//#define DEBUG

/* _A_
struct entry {//wtf is with my stupid choice of names?
 void *target;//value
 char *original;//key
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
 if(!key) return fprintf(stderr,"piss.\n"),0;
 return murmur3_32(key,strlen(key),0);
}

unsigned int ht_getkeycount(struct hashtable *ht) {
  struct entry *m;
  int key_count=0,i;
  for(i=0;i<ht->kl;i++) //who needs braces? not us. our teeth are straight. :|
    if(ht->bucket[ht->keys[i]])
      for(m=ht->bucket[ht->keys[i]]->ll;m;m=m->next)
        key_count++;
  return key_count;
}

//returns an array of pointers to the keys stored in the hashtable
//free the pointer returned? or maybe we should just pass by reference?
char **ht_getkeys(struct hashtable *ht,unsigned int *len) {
  int i,j;
  //need to count up how many entries are in the hash table?
  //should the hashtable keep track of that info by itself?
  char **a;
  struct entry *m;
  *len=ht_getkeycount(ht);
  a=malloc(sizeof(char *) * *len);
  j=0;
  for(i=0;i<ht->kl;i++) {//for each key...
    if(ht->bucket[ht->keys[i]]) {
      for(m=ht->bucket[ht->keys[i]]->ll;m;m=m->next) {
        a[j]=m->original;
        j++;
      }
    }
  }
  return a;
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
  if(m->original) {
   if(!strcmp(key,m->original)) {
    return m;
   }
  } else {
   fprintf(stderr,"shit. %s this element in the linked list doesn't have a name. wtf?\n",key);
  }
 }
 return 0;
}

//returns the table entry (a linked list) at the key.
struct entry *ht_getentry(struct hashtable *ht,char *key) {
 if(!ht) return NULL;
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

void ht_dump(struct hashtable *ht) {
  int i;
  struct entry *m;

  unsigned int key_count=0;
  char **keys=ht_getkeys(ht,&key_count);
  printf("amount of keys: %d\n",key_count);
  for(i=0;i<key_count;i++) {
    printf("key: %s hash: %d\n",keys[i],hash(keys[i]) % ht->size);
  }

  printf("kl: %d\n",ht->kl);
  for(i=0;i<ht->kl;i++) {
    printf("looking into hash: %d\n",ht->keys[i]);
    if(ht->bucket[ht->keys[i]]) {
      printf("bucket not empty!: %d\n",ht->keys[i]);
      printf("printing its linked list:\n");
      for(m=ht->bucket[ht->keys[i]]->ll;m;m=m->next) {
        printf("  key: %s hash: %08x\n",m->original,ht->keys[i]);
      }
      printf("end of linked list\n");
    } else {
      printf("empty bucket: %d\n",ht->keys[i]);
    }
  }
}

//delete the node in the linked list in the table entry that matches the key.
void ht_delete(struct hashtable *ht,char *key) {
 int i;
 unsigned int h=hash(key)%(ht->size);
 struct entry *tmp;

 struct hitem *bucket=ht->bucket[h];
 if(!bucket) { fprintf(stderr,"attempted to ht_delete key: %s which had a non-existing bucket.\n",key) ; return; }
 if(!bucket->ll) { fprintf(stderr,"attempted to ht_delete key: %s which has a bucket with a null ll\n",key) ; return; }
 for(tmp=bucket->ll;tmp;tmp=tmp->next) {
   if(!strcmp(tmp->original,key)) break;//we found it
 }
 if(!tmp) { fprintf(stderr,"attempted to ht_delete key: %s which isn't in the linked list for its bucket\n",key); return; }
 tmp->target=0;
 if(tmp->next == 0 && tmp->prev == 0) {//only dump the bucket if it has only one thing
   free(bucket->ll);//btw, bucket->ll == tmp
   free(bucket);
   ht->bucket[h % ht->size]=0;

   for(i=0;i<ht->kl;i++) {
     if(ht->keys[i] == h) break;
   }
   for(;i<ht->kl;i++) {
     ht->keys[i]=ht->keys[i+1];
   }
   ht->kl--;
 }
 else {
   if(tmp->next) tmp->next->prev=tmp->prev;
   if(tmp->prev) tmp->prev->next=tmp->next;
   else bucket->ll=tmp->next;//we deleted the start of the linked list, we need to tell our bucket about it.
   free(tmp);
 }
 if(ht->kl < 0) {
   abort();
 }
}
