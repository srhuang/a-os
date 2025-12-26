#include "printf.h"
#include "syscall_usr.h"

void init(void)
{
    printf("User Process Init\n");
    ps();

    while(1);

}
