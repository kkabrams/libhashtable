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
unsigned int murmur3_32(const char *key, unsigned int len, unsigned int seed);
unsigned int hash(char *key);
unsigned int ht_getkeycount(struct hashtable *ht);
char **ht_getkeys(struct hashtable *ht,unsigned int *len);
void inittable(struct hashtable *ht,unsigned int tsize);
void ll_delete(struct entry *ll);
void ll_destroy(struct entry *ll);
void ht_destroy(struct hashtable *ht);
void ht_freevalues(struct hashtable *ht);
int ht_setkey(struct hashtable *ht,char *key,void *value);
struct entry *ll_getentry(struct entry *start,char *key);
struct entry *ht_getentry(struct hashtable *ht,char *key);
struct entry *ht_getnode(struct hashtable *ht,char *key);
void *ht_getvalue(struct hashtable *ht,char *key);
struct hitem *ht_getbucket(struct hashtable *ht,char *key);
void ht_dump(struct hashtable *ht);
void ht_delete(struct hashtable *ht,char *key);
