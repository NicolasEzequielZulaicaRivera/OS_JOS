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

int waity( int loops, int ops ) {
	// can be reasonably expected to help in ordering operations
	int z = 0;
	for ( int i = 0; i < loops; i++ ) {
		for ( int j = 0; j < ops; j++ ) {
			z++;
		}
		sys_yield();
	}
	return z;
}

void
main12()
{
	envid_t parent = thisenv->env_id;
	int NMESS = 3;
	int message = 0;

	envid_t child_1 = fork();
	if ( child_1 < 0 ) return;
	if( child_1 == 0 ){
		// Child 1
		cprintf("[ CH 1 ]\n");
		ipc_send(parent, 1, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_2 = fork();
	if ( child_2 < 0 ) return;
	if( child_2 == 0 ){
		// Child 2
		cprintf("[ CH 2 ]\n");
		ipc_send(parent, 2, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_3 = fork();
	if ( child_3 < 0 ) return;
	if( child_3 == 0 ){
		// Child 3
		cprintf("[ CH 3 ]\n");
		ipc_send(parent, 3, NULL, 0);
		return;
	}
	waity(10,100);

	cprintf("[ PARENT ]\n");
	for(int i = 0; i < NMESS; i++){
		message = ipc_recv(&parent, TEMP_ADDR, 0);
		cprintf("> %d \n", message);
	}

	return;
}

void
main13()
{
	// Test various senders
	// Firts gets deleted
	envid_t parent = thisenv->env_id;
	int NMESS = 3;
	int message = 0;

	envid_t child_1 = fork();
	if ( child_1 < 0 ) return;
	if( child_1 == 0 ){
		// Child 1
		cprintf("[ CH 1 ]\n");
		ipc_send(parent, 1, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_2 = fork();
	if ( child_2 < 0 ) return;
	if( child_2 == 0 ){
		// Child 2
		cprintf("[ CH 2 ]\n");
		ipc_send(parent, 2, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_3 = fork();
	if ( child_3 < 0 ) return;
	if( child_3 == 0 ){
		// Child 3
		cprintf("[ CH 3 ]\n");
		ipc_send(parent, 3, NULL, 0);
		return;
	}
	waity(10,100);

	cprintf("[ PARENT ]\n");

	// Delete child 1
	sys_env_destroy(child_1);

	for(int i = 0; i < NMESS; i++){
		message = ipc_recv(&parent, TEMP_ADDR, 0);
		cprintf("> %d \n", message);
	}

	return;
}

void
main14()
{
	// Test various senders
	// Middle gets deleted
	envid_t parent = thisenv->env_id;
	int NMESS = 3;
	int message = 0;

	envid_t child_1 = fork();
	if ( child_1 < 0 ) return;
	if( child_1 == 0 ){
		// Child 1
		cprintf("[ CH 1 ]\n");
		ipc_send(parent, 1, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_2 = fork();
	if ( child_2 < 0 ) return;
	if( child_2 == 0 ){
		// Child 2
		cprintf("[ CH 2 ]\n");
		ipc_send(parent, 2, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_3 = fork();
	if ( child_3 < 0 ) return;
	if( child_3 == 0 ){
		// Child 3
		cprintf("[ CH 3 ]\n");
		ipc_send(parent, 3, NULL, 0);
		return;
	}
	waity(10,100);

	cprintf("[ PARENT ]\n");

	// Delete child 2
	sys_env_destroy(child_2);

	for(int i = 0; i < NMESS; i++){
		message = ipc_recv(&parent, TEMP_ADDR, 0);
		cprintf("> %d \n", message);
	}

	return;
}

void
main15()
{
	// Test various senders
	// Last gets deleted
	envid_t parent = thisenv->env_id;
	int NMESS = 3;
	int message = 0;

	envid_t child_1 = fork();
	if ( child_1 < 0 ) return;
	if( child_1 == 0 ){
		// Child 1
		cprintf("[ CH 1 ]\n");
		ipc_send(parent, 1, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_2 = fork();
	if ( child_2 < 0 ) return;
	if( child_2 == 0 ){
		// Child 2
		cprintf("[ CH 2 ]\n");
		ipc_send(parent, 2, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_3 = fork();
	if ( child_3 < 0 ) return;
	if( child_3 == 0 ){
		// Child 3
		cprintf("[ CH 3 ]\n");
		ipc_send(parent, 3, NULL, 0);
		return;
	}
	waity(10,100);

	cprintf("[ PARENT ]\n");

	// Delete child 3
	sys_env_destroy(child_3);

	for(int i = 0; i < NMESS; i++){
		message = ipc_recv(&parent, TEMP_ADDR, 0);
		cprintf("> %d \n", message);
	}

	return;
}

void main16()
{
	// Test various senders
	// Middle gets deleted
	envid_t parent = thisenv->env_id;
	int NMESS = 3;
	int message = 0;

	envid_t child_1 = fork();
	if ( child_1 < 0 ) return;
	if( child_1 == 0 ){
		// Child 1
		cprintf("[ CH 1 ]\n");
		ipc_send(parent, 1, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_2 = fork();
	if ( child_2 < 0 ) return;
	if( child_2 == 0 ){
		// Child 2
		cprintf("[ CH 2 ]\n");
		ipc_send(parent, 2, NULL, 0);
		return;
	}
	waity(10,100);

	envid_t child_3 = fork();
	if ( child_3 < 0 ) return;
	if( child_3 == 0 ){
		// Child 3
		cprintf("[ CH 3 ]\n");
		ipc_send(parent, 3, NULL, 0);
		return;
	}
	waity(10,100);

	cprintf("[ PARENT ]\n");

	// Delete self
	sys_env_destroy(0);

	return;
}

void
umain(int argc, char **argv)
{
	main0();
}
