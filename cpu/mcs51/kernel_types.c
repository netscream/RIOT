#include "cpu.h"
#include "kernel_types.h"

int pid_is_valid(kernel_pid_t pid)
{
	return ((1 <= pid) && (pid <= (MAXTHREADS - 1)));
}
