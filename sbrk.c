#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#define _GNU_SOURCE
#include <dlfcn.h>
static struct sbrk_state_s
{
void *top;
int fd;
size_t size;
size_t prealloc;
pid_t pid;
} s;


static void initialize()
{
	char fname[64];
	char buf[64];
	char *prefix;

	pid_t pid = getpid();
	int fd;
	void *begin = NULL;
	int flags = MAP_SHARED;

	if( pid == s.pid )
		return;
	if( s.pid )
	{
		//printf("error: fork not supported\n");
	}
	prefix = getenv("SWAP_PREFIX");
	if( !prefix )
		prefix = "/tmp";
	snprintf(fname, 64, "%s/pages_%d", prefix, (int)pid);
	fd = open( fname, O_CREAT|O_RDWR, 0600 );
	char *prealloc = getenv("SWAP_SIZE");
	if( prealloc ) s.prealloc = atoi(prealloc);
	else s.prealloc = 128*1024*1024;
	if( s.pid )
	{

		write(1, buf, snprintf(buf, 32, "a\n") );

		mprotect( s.top - s.size, s.prealloc, PROT_READ|PROT_WRITE);
		write( fd, s.top - s.size, s.size );
		munmap( s.top - s.size, s.prealloc );
		close( s.fd );
		flags |= MAP_FIXED;

		write(1, buf, snprintf(buf, 32, "b\n") );
		//printf("error: fork not supported\n");
	}

	s.pid = pid, s.fd = fd;
	ftruncate(fd, s.prealloc);
	s.top = mmap(s.top - s.size, s.prealloc, PROT_READ|PROT_WRITE, flags, fd, 0);

		write(1, buf, snprintf(buf, 32, "c\n") );
	unlink(fname); // delete on exit

}
void *sbrk(intptr_t size)
{
	char buf[64];
	initialize();

	if( size == 0 )
		return s.top;
	else if( size > 0 )
	{
		void *res;

		write(1, buf, snprintf(buf, 32, "allocating %d\n", size) );
		res = s.top;
		s.size += size;
		s.top = res + size;
		if( s.size + size > s.prealloc )
			res = (void*)-1;
		else
		{
			s.size += size;
			s.top = res + size;
		}

		return res;
	}
	else
	{
		int res = s.top;

		if( -size > s.size )
			res = (void*)-1;
		else
		{
			s.top += size;
			s.size += size;
			write(1, buf, snprintf(buf, 32, "freed %d\n", -size) );
		}

		return res;
	}
}


pid_t fork(void)
{
	static pid_t (*fork_real)(void);
	pid_t result;
	
	if(!fork_real)
		fork_real = dlsym(RTLD_NEXT, "fork");
	result = fork_real();
	if( result )
		usleep(500000);
	else
	{
	//	pid_t pid = s.pid;
	//	kill(pid, SIGTSTP);
		initialize();
	//	kill(pid, SIGCONT);
	}
	return result;
}

