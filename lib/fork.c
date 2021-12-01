// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	panic("duppage not implemented");
	return 0;
}

//
static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int r;

	// Check permissions
	if (!(perm & PTE_P) || !(perm & PTE_U))
		return;

	if (perm & PTE_W) {
		// DUPLICATE
		if (sys_page_alloc(dstenv, va, PTE_P | PTE_U | PTE_W) < 0)
			panic("dup_or_share: dup alloc error");

		if (sys_page_map(dstenv, va, 0, UTEMP, PTE_P | PTE_U | PTE_W) < 0)
			panic("dup_or_share: dup map error");
		memmove(UTEMP, va, PGSIZE);

		if (sys_page_unmap(0, UTEMP) < 0)
			panic("dup_or_share: dup unmap error");
	} else {
		// SHARE
		if (sys_page_map(dstenv, va, 0, UTEMP, perm) < 0)
			panic("dup_or_share: share map error");
	}
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	envid_t envid;
	uint8_t *addr;
	int r;
	extern unsigned char end[];

	// Allocate a new child environment.
	// The kernel will initialize it with a copy of our register state,
	// so that the child will appear to have called sys_exofork() too -
	// except that in the child, this "fake" call to sys_exofork()
	// will return 0 instead of the envid of the child.
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	// Eagerly copy our entire address space into the child.
	// This is NOT what you should do in your fork implementation.
	for (addr = (uint8_t *) UTEXT; addr < (uint8_t *) UTOP; addr += PGSIZE)
		if ((uvpd[PDX(addr)] & PTE_P) &&    // page table present
		    (uvpt[PGNUM(addr)] & PTE_P) &&  // page present
		    (uvpt[PGNUM(addr)] & PTE_U)     // page accessible by user
		)
			dup_or_share(envid, addr, uvpt[PGNUM(addr)]);

	// Also copy the stack we are currently running on.
	dup_or_share(envid, ROUNDDOWN(&addr, PGSIZE), PTE_P | PTE_U | PTE_W);

	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}