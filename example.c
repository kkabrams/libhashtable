#include <stdio.h>
#include "hashtable.h"

extern char **environ;

int main(int argc,char *argv[]) {
 struct hashtable ht;
 int i;
 char *name;
 char *tmp;
 char *value;
 inittable(&ht,1000);
 for(i=0;environ[i];i++) {
  name=strdup(environ[i]);
  if((tmp=strchr(name,'=') )){
   *tmp=0;
   tmp++;
  }
  value=strdup(tmp);
  ht_setkey(&ht,name,value);
  free(name);
 }
 printf("ht.size: %u\n",ht.size);
 printf("before: PATH='%s'\n",ht_getvalue(&ht,"PATH"));
 ht_delete(&ht,"PATH");
 printf("after:  PATH='%s'\n",ht_getvalue(&ht,"PATH"));
 //if you want to get a whole entry struct use ht_getnode();
 //getentry() just returns the first struct in the linked list in that bucket.
 return 0;
}
