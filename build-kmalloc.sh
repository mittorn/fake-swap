gcc kmalloc.c sbrk.c -shared -DHAVE_MMAP=0 -o libmalloc.so -fPIC -Dkmalloc=malloc -Dkrealloc=realloc
