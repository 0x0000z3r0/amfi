#import <dlfcn.h>
#import <mach-o/dyld.h>
#import <mach-o/nlist.h>
#import <stdio.h>
#import <string.h>

int main(int argc, char *argv[])
{
        dlopen("./lib.dylib", RTLD_LAZY);
        return 0;
}

// clang dl.c -o dl
