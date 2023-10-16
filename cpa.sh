gcc -o VInit VInit.c -I /usr/local/include/fuse3;
gcc -o MFS MFS.c $(pkg-config --cflags --libs fuse3)
