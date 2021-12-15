/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	// LAB 3: Your code here.
	user_mem_assert(curenv, s, len, PTE_P);

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	// LAB 4: Your code here.
	struct Env *new_env;
	int creation_result = env_alloc(&new_env, curenv->env_id);

	if (creation_result < 0)
		return creation_result;  // errors are already handled in env_alloc

	new_env->env_status = ENV_NOT_RUNNABLE;

	new_env->env_tf = curenv->env_tf;
	new_env->env_tf.tf_regs.reg_eax = 0;  // return 0

	return new_env->env_id;
}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.

	// LAB 4: Your code here.
	if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE)
		return -E_INVAL;

	struct Env *env;

	if (envid2env(envid, &env, 1) < 0)
		return -E_BAD_ENV;

	env->env_status = status;
	return 0;
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	struct Env *env;
	envid2env(envid, &env, true);
	if (!env) {
		return -E_BAD_ENV;
	}

	env->env_pgfault_upcall = func;
	return 0;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_SYSCALL in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 4: Your code here.
	if ((uint32_t) va >= UTOP || (uint32_t) va % PGSIZE != 0)
		return -E_INVAL;

	if ((perm & ~(PTE_U | PTE_P | PTE_AVAIL | PTE_W)) ||
	    !(perm & (PTE_U | PTE_P)))
		return -E_INVAL;

	struct Env *env;
	if (envid2env(envid, &env, 1) < 0)
		return -E_BAD_ENV;

	struct PageInfo *page = page_alloc(ALLOC_ZERO);
	if (page == NULL)
		return -E_NO_MEM;

	if (page_insert(env->env_pgdir, page, va, perm) < 0) {
		page_free(page);
		return -E_NO_MEM;
	}

	return 0;
}

int
_sys_page_map(struct Env *src_env,
              void *srcva,
              struct Env *dst_env,
              void *dstva,
              int perm)
{
	if ((perm & ~(PTE_U | PTE_P | PTE_AVAIL | PTE_W | PTE_COW)) ||
	    !(perm & (PTE_P | PTE_U)))
		return -E_INVAL;


	if ((uint32_t) srcva >= UTOP || (uint32_t) srcva % PGSIZE != 0 ||
	    (uint32_t) dstva >= UTOP || (uint32_t) dstva % PGSIZE != 0)
		return -E_INVAL;

	pte_t *src_pte;
	struct PageInfo *src_page =
	        page_lookup(src_env->env_pgdir, srcva, &src_pte);
	if (src_page == NULL)
		return -E_INVAL;

	if ((perm & PTE_W) && ((*src_pte) & PTE_W) == 0)
		return -E_INVAL;

	if (page_insert(dst_env->env_pgdir, src_page, dstva, perm) < 0)
		return -E_NO_MEM;

	return 0;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva, envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.

	struct Env *src_env;
	if (envid2env(srcenvid, &src_env, 1) < 0)
		return -E_BAD_ENV;

	struct Env *dst_env;
	if (envid2env(dstenvid, &dst_env, 1) < 0)
		return -E_BAD_ENV;

	return _sys_page_map(src_env, srcva, dst_env, dstva, perm);
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().

	// LAB 4: Your code here.
	if ((uint32_t) va >= UTOP || (uint32_t) va % PGSIZE != 0)
		return -E_INVAL;

	struct Env *env;
	if (envid2env(envid, &env, 1) < 0)
		return -E_BAD_ENV;

	page_remove(env->env_pgdir, va);
	return 0;
}

int
insert_env_ipc_sender(struct Env *env, envid_t sender_id)
{
	struct Env *sender;
	if (envid2env(sender_id, &sender, 0) < 0)
		return -E_BAD_ENV;

	if (env->env_ipc_senders_head == env->env_id) {
		// First sender
		env->env_ipc_senders_head = sender_id;
		env->env_ipc_senders_tail = sender_id;
	} else {
		// Add to tail
		struct Env *tail_env;
		if (envid2env(env->env_ipc_senders_tail, &tail_env, 0) < 0)
			return -E_BAD_ENV;
		tail_env->env_ipc_senders_next = sender;
		env->env_ipc_senders_tail = sender_id;
	}
	return 0;
}

int
remove_env_ipc_sender(struct Env *env, envid_t sender_id)
{
	if (env->env_ipc_senders_head == env->env_ipc_senders_tail) {
		if (env->env_ipc_senders_head == env->env_id)
			return 0;  // List is empty

		// List has one element
		// Set it as self to mark as empty
		env->env_ipc_senders_head = env->env_ipc_senders_tail =
		        env->env_id;
		return 0;
	}
	// List has more than one element
	struct Env *sender = NULL, *prev_sender = NULL;
	if (envid2env(sender_id, &sender, 0) < 0)
		return -E_BAD_ENV;
	sender = prev_sender->env_ipc_senders_next;
	while (sender) {
		if (sender->env_id == sender_id) {
			// Found the sender
			if (sender->env_id == env->env_ipc_senders_tail) {
				// Last sender
				env->env_ipc_senders_tail = prev_sender->env_id;
				prev_sender->env_ipc_senders_next = NULL;
			} else {
				// Not last sender
				prev_sender->env_ipc_senders_next =
				        sender->env_ipc_senders_next;
			}
			sender->env_ipc_senders_next = NULL;
			return 0;
		}
		sender = sender->env_ipc_senders_next;
	}
	return 0;
}

int
ipc_set_send_pgdata(struct Env *reciever, struct Env *sender, void *srcva, int perm)
{
	// If srcva < UTOP, then also send page currently mapped at 'srcva',
	// so that receiver gets a duplicate mapping of the same page.
	void *dstva = reciever->env_ipc_dstva;
	if ((uint32_t) srcva < UTOP && (uint32_t) dstva < UTOP) {
		if (_sys_page_map(sender, srcva, reciever, dstva, perm) < 0)
			return -E_NO_MEM;
		reciever->env_ipc_perm = perm;
	} else {
		reciever->env_ipc_perm = 0;
	}
	return 0;
}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
	struct Env *reciever;  // receiver environment

	//	-E_BAD_ENV if environment envid doesn't currently exist.
	if (envid2env(envid, &reciever, 0) < 0)
		return -E_BAD_ENV;

	//	-E_IPC_NOT_RECV if envid is not currently blocked
	if (!(reciever->env_ipc_recving))
		return -E_IPC_NOT_RECV;

	pte_t *pte;
	if (
	        //	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
	        ((srcva < (void *) UTOP) && ((uint32_t) srcva % PGSIZE != 0))
	        //	-E_INVAL if srcva < UTOP and perm is inappropriate
	        //		(see sys_page_alloc).
	        || ((srcva < (void *) UTOP) &&
	            (perm & ~(PTE_U | PTE_P | PTE_AVAIL | PTE_W)))
	        //	-E_INVAL if srcva < UTOP but srcva is not mapped in the
	        // caller's 		address space.
	        || ((srcva < (void *) UTOP) &&
	            (page_lookup(curenv->env_pgdir, srcva, &pte) == NULL))
	        //	-E_INVAL if (perm & PTE_W), but srcva is read-only in
	        // the 		current environment's address space.
	        || ((perm & PTE_W) && !(*pte & PTE_W)))
		return -E_INVAL;

	// Set message
	if (ipc_set_send_pgdata(reciever, curenv, srcva, perm) < 0)
		return -E_NO_MEM;
	reciever->env_ipc_value = value;
	reciever->env_ipc_from = curenv->env_id;

	// Set env fields
	reciever->env_ipc_recving = 0;
	reciever->env_status = ENV_RUNNABLE;
	reciever->env_tf.tf_regs.reg_eax = 0;

	return 0;
}

// Try to receive a value from the sender list
//
// The send fails with a return value of -E_IPC_NOT_SEND if there
// is no element on the sender list.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_try_recv(void *dstva)
{
	if ((uint32_t) dstva < UTOP && (uint32_t) dstva % PGSIZE != 0)
		return -E_INVAL;

	if (curenv->env_ipc_senders_head == curenv->env_id)
		return -E_IPC_NOT_SEND;

	// Activate sender
	struct Env *sender;
	if (envid2env(curenv->env_ipc_senders_head, &sender, 0) < 0) {
		remove_env_ipc_sender(curenv, curenv->env_ipc_senders_head);
		return -E_BAD_ENV;
	}
	remove_env_ipc_sender(curenv, curenv->env_ipc_senders_head);

	sender->env_status = ENV_RUNNABLE;
	sender->env_tf.tf_regs.reg_eax = 0;

	// Load message
	// as an env cannot send and recv at the same time, we can use the same fields to store the sender's data
	curenv->env_ipc_dstva = dstva;
	if (ipc_set_send_pgdata(curenv,
	                        sender,
	                        sender->env_ipc_dstva,
	                        sender->env_ipc_perm) < 0)
		return -E_NO_MEM;
	curenv->env_ipc_value = sender->env_ipc_value;
	curenv->env_ipc_from = sender->env_id;

	return 0;
}


static int
sys_ipc_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	int r;
	if ((r = sys_ipc_try_send(envid, value, srcva, perm)) != -E_IPC_NOT_RECV)
		return r;

	// Add current environment to targets sender list
	struct Env *target;
	if (envid2env(envid, &target, 0) < 0)
		return -E_BAD_ENV;
	insert_env_ipc_sender(target, curenv->env_id);

	// Block current environment
	curenv->env_ipc_sending = 1;
	curenv->env_ipc_to = envid;
	curenv->env_status = ENV_NOT_RUNNABLE;

	// Store message
	// as an env cannot send and recv at the same time, we can use the same fields to store the sender's data
	curenv->env_ipc_dstva = srcva;
	curenv->env_ipc_perm = perm;
	curenv->env_ipc_value = value;

	return -E_UNSPECIFIED;  // return value should be set outside
}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	int r;
	if ((r = sys_ipc_try_recv(dstva)) != -E_IPC_NOT_SEND)
		return r;

	if ((uint32_t) dstva < UTOP && (uint32_t) dstva % PGSIZE != 0)
		return -E_INVAL;

	curenv->env_ipc_recving = 1;
	curenv->env_ipc_dstva = dstva;
	curenv->env_status = ENV_NOT_RUNNABLE;

	return -E_UNSPECIFIED;  // return value should be set outside
}

// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.

	switch (syscallno) {
	case SYS_cputs:
		sys_cputs((const char *) a1, a2);
		return 0;
	case SYS_cgetc:
		return sys_cgetc();
	case SYS_getenvid:
		return sys_getenvid();
	case SYS_env_destroy:
		return sys_env_destroy(a1);
	case SYS_yield:
		sys_yield();
		return 0;
	case SYS_exofork:
		return sys_exofork();
	case SYS_env_set_status:
		return sys_env_set_status(a1, a2);
	case SYS_page_alloc:
		return sys_page_alloc(a1, (void *) a2, a3);
	case SYS_page_map:
		return sys_page_map(a1, (void *) a2, a3, (void *) a4, a5);
	case SYS_page_unmap:
		return sys_page_unmap(a1, (void *) a2);
	case SYS_ipc_try_send:
		return sys_ipc_try_send(a1, a2, (void *) a3, a4);
	case SYS_ipc_try_recv:
		return sys_ipc_try_recv((void *) a1);
	case SYS_ipc_send:
		return sys_ipc_send(a1, a2, (void *) a3, a4);
	case SYS_ipc_recv:
		return sys_ipc_recv((void *) a1);
	case SYS_env_set_pgfault_upcall:
		return sys_env_set_pgfault_upcall(a1, (void *) a2);
	default:
		return -E_INVAL;
	}
}
