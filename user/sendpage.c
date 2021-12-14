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

// Child Sends, Parent receives

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
		sys_yield();
		cprintf("child environment\n");
		message = 666;
		ipc_send(parent, message, NULL, 0);
		cprintf("child environment sent message: %d\n", message);
		return;
	}
	// Parent
	cprintf("parent environment\n");
	message = ipc_recv(&child, NULL, NULL);
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
	sys_yield();
	cprintf("parent environment\n");
	message = ipc_recv(&child, NULL, NULL);
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
		sys_yield();
		cprintf("child environment\n");
		message = 666;
		sys_page_alloc(thisenv->env_id, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR_CHILD, str2, strlen(str2) + 1);
		ipc_send(parent, message, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		cprintf("child environment sent message: %d {%s}\n", message, TEMP_ADDR_CHILD);
		return;
	}
	// Parent
	cprintf("parent environment\n");
	message = ipc_recv(&child, TEMP_ADDR, 0);
	cprintf("parent received message from child: %d {%s} \n", message, TEMP_ADDR);
	return;
}

void
main4()
{
	// New Style
	// Blocking Send + Store
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
		sys_page_alloc(thisenv->env_id, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR_CHILD, str2, strlen(str2) + 1);
		ipc_send(parent, message, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		cprintf("child environment sent message: %d {%s}\n", message, TEMP_ADDR_CHILD);
		return;
	}
	// Parent
	sys_yield();
	cprintf("parent environment\n");
	message = ipc_recv(&child, TEMP_ADDR, 0);
	cprintf("parent received message from child: %d {%s} \n", message, TEMP_ADDR);
	return;
}

void
main5()
{
	// Uncontrolled
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
		sys_page_alloc(thisenv->env_id, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR_CHILD, str2, strlen(str2) + 1);
		ipc_send(parent, message, TEMP_ADDR_CHILD, PTE_P | PTE_W | PTE_U);
		cprintf("child environment sent message: %d {%s}\n", message, TEMP_ADDR_CHILD);
		return;
	}
	// Parent
	cprintf("parent environment\n");
	message = ipc_recv(&child, TEMP_ADDR, 0);
	cprintf("parent received message from child: %d {%s} \n", message, TEMP_ADDR);
	return;
}

// Parent Sends, Child receives

void
main6()
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
	if( child != 0 ){
		// Parent
		sys_yield();
		cprintf("parent environment\n");
		message = 666;
		ipc_send(child, message, NULL, 0);
		cprintf("parent environment sent message: %d\n", message);
		return;
	}
	// Child
	cprintf("child environment\n");
	message = ipc_recv(&parent, NULL, NULL);
	cprintf("child received message from parent: %d\n", message);
	return;
}

void
main7()
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
	if( child != 0 ){
		// Parent
		cprintf("parent environment\n");
		message = 666;
		ipc_send(child, message, NULL, 0);
		cprintf("parent environment sent message: %d\n", message);
		return;
	}
	// Child
	sys_yield();
	cprintf("child environment\n");
	message = ipc_recv(&parent, NULL, NULL);
	cprintf("child received message from parent: %d\n", message);
	return;
}

void
main8()
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
	if( child != 0 ){
		// Parent
		sys_yield();
		cprintf("parent environment\n");
		message = 666;

		sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
		ipc_send(child, message, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		cprintf("child environment sent message: %d {%s}\n", message, TEMP_ADDR);

		return;
	}
	// Child
	cprintf("child environment\n");
	message = ipc_recv(&parent, TEMP_ADDR_CHILD, 0);
	cprintf("parent received message from child: %d {%s} \n", message, TEMP_ADDR_CHILD);
	return;
}

void
main9()
{
	// New Style
	// Blocking Send + Store
	envid_t parent = thisenv->env_id;
	envid_t child = fork();
	int message = 0;

	if ( child < 0 ){
		cprintf("fork failed\n");
		return;
	}
	if( child != 0 ){
		// Parent
		cprintf("parent environment\n");
		message = 666;
		sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
		cprintf("parent environment sending message: %d {%s}\n", message, TEMP_ADDR);
		ipc_send(child, message, TEMP_ADDR, PTE_P | PTE_W | PTE_U);

		return;
	}
	// Child
	sys_yield();
	cprintf("child environment\n");
	message = ipc_recv(&parent, TEMP_ADDR_CHILD, 0);
	cprintf("child received message from parent: %d {%s} \n", message, TEMP_ADDR_CHILD);
	return;
}

void
main10()
{
	// Uncontrolled
	envid_t parent = thisenv->env_id;
	envid_t child = fork();
	int message = 0;

	if ( child < 0 ){
		cprintf("fork failed\n");
		return;
	}
	if( child != 0 ){
		// Parent
		cprintf("parent environment\n");
		message = 666;
		sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
		ipc_send(child, message, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
		cprintf("child environment sent message: %d {%s}\n", message, TEMP_ADDR);

		return;
	}
	// Child
	cprintf("child environment\n");
	message = ipc_recv(&parent, TEMP_ADDR_CHILD, 0);
	cprintf("parent received message from child: %d {%s} \n", message, TEMP_ADDR_CHILD);
	return;
}

void
main11()
{
	envid_t parent = thisenv->env_id;
	envid_t child = fork();
	int message = 0;
	int NMESS = 10;

	if ( child < 0 ){
		cprintf("fork failed\n");
		return;
	}
	if( child != 0 ){
		// Parent
		for(int i = 0; i < NMESS; i++){
			sys_page_alloc(thisenv->env_id, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
			memcpy(TEMP_ADDR, str1, strlen(str1) + 1);
			cprintf("## PARNT >>> %d {%s}\n", i, TEMP_ADDR);
			ipc_send(child, i, TEMP_ADDR, PTE_P | PTE_W | PTE_U);
			cprintf("## PARNT --- \n");
		}

		return;
	}
	// Child
	for(int i = 0; i < NMESS; i++){
		cprintf("** CHILD --- \n");
		message = ipc_recv(&parent, TEMP_ADDR_CHILD, 0);
		cprintf("** CHILD <<< %d {%s} \n", message, TEMP_ADDR_CHILD);
	}
	return;
}


void
umain(int argc, char **argv)
{
	main0();
}
