// Test Conversation between parent and child environment
// Contributed by Varun Agrawal at Stony Brook

#include <inc/lib.h>

const char *str1 = "hello child environment! how are you?";
const char *str2 = "hello parent environment! I'm good.";

#define TEMP_ADDR	((char*)0xa00000)
#define TEMP_ADDR_CHILD	((char*)0xb00000)
void
main0()
{
	envid_t who;

	if ((who = fork()) == 0) {
		// Child
		ipc_recv(&who, TEMP_ADDR_CHILD, 0);
		cprintf("%x got message from %x: %s\n",
			thisenv->env_id, who, TEMP_ADDR_CHILD);
		if (strncmp(TEMP_ADDR_CHILD, str1, strlen(str1)) == 0)
			cprintf("child received correct message\n");

		memcpy(TEMP_ADDR_CHILD, str2, strlen(str2) + 1);
		ipc_send(who, 0, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		return;
	}

	// Parent
	sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
	memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
	ipc_send(who, 0, TEMP_ADDR, PTE_P | PTE_W | PTE_U);

	ipc_recv(&who, TEMP_ADDR, 0);
	cprintf("%x got message from %x: %s\n",
		thisenv->env_id, who, TEMP_ADDR);
	if (strncmp(TEMP_ADDR, str2, strlen(str2)) == 0)
		cprintf("parent received correct message\n");
	return;
}

void
main1()
{
	// Original Style
	// Blocking Recv
	envid_t parent = thisenv->env_id;
	envid_t child = fork();
	int message = 0;

	if ( child < 0 ){
		cprintf("fork failed\n");
		return;
	}
	if( child == 0 ){
		// Child
		for(int i = 0; i < 10000; i++){}
		cprintf("child environment\n");
		message = 666;
		ipc_send(parent, message, NULL, 0);
		cprintf("child environment sent message: %d\n", message);
		return;
	}
	// Parent
	cprintf("parent environment\n");
	message = ipc_recv(&parent, NULL, NULL);
	cprintf("parent received message from child: %d\n", message);
	return;
}

void
main2()
{
	// New Style
	// Blocking Send
	envid_t parent = thisenv->env_id;
	envid_t child = fork();
	int message = 0;

	if ( child < 0 ){
		cprintf("fork failed\n");
		return;
	}
	if( child == 0 ){
		// Child
		cprintf("child environment\n");
		message = 666;
		ipc_send(parent, message, NULL, 0);
		cprintf("child environment sent message: %d\n", message);
		return;
	}
	// Parent
	for(int i = 0; i < 10000; i++){}
	cprintf("parent environment\n");
	message = ipc_recv(&parent, NULL, NULL);
	cprintf("parent received message from child: %d\n", message);
	return;
}

void
main3()
{
	// Original Style
	// Blocking Recv + Store
	envid_t parent = thisenv->env_id;
	envid_t child = fork();
	int message = 0;

	if ( child < 0 ){
		cprintf("fork failed\n");
		return;
	}
	if( child == 0 ){
		// Child
		for(int i = 0; i < 10000; i++){}
		cprintf("child environment\n");
		message = 666;
		sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
		ipc_send(parent, message, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		cprintf("child environment sent message: %d {%s}\n", message, TEMP_ADDR);
		return;
	}
	// Parent
	cprintf("parent environment\n");
	message = ipc_recv(&parent, NULL, NULL);
	cprintf("parent received message from child: %d\n", message);
	return;
}

void
umain(int argc, char **argv)
{
	main0();
}
