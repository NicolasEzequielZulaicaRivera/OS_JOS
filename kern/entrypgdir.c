#include <inc/mmu.h>
#include <inc/memlayout.h>

pte_t entry_pgtable[NPTENTRIES];

// The entry.S page directory maps the first 4MB of physical memory
// starting at virtual address KERNBASE (that is, it maps virtual
// addresses [KERNBASE, KERNBASE+4MB) to physical addresses [0, 4MB)).
// We choose 4MB because that's how much we can map with one page
// table and it's enough to get us through early boot.  We also map
// virtual addresses [0, 4MB) to physical addresses [0, 4MB); this
// region is critical for a few instructions in entry.S and then we
// never use it again.
//
// Page directories (and page tables), must start on a page boundary,
// hence the "__aligned__" attribute.  Also, because of restrictions
// related to linking and static initializers, we use "x + PTE_P"
// here, rather than the more standard "x | PTE_P".  Everywhere else
// you should use "|" to combine flags.
__attribute__((__aligned__(PGSIZE)))
pde_t entry_pgdir[NPDENTRIES] = {
	// Map VA's [0, 4MB) to PA's [0, 4MB)
	[0]
		= ((uintptr_t) 0x000000)  + PTE_PS + PTE_P,

	// Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
	[KERNBASE>>PDXSHIFT]
		= ((uintptr_t) 0x000000)  + PTE_PS + PTE_P + PTE_W,
};

// Entry 0 of the page table maps to physical page 0, entry 1 to
// physical page 1, etc.
#if 0
__attribute__((__aligned__(PGSIZE)))
pte_t entry_pgtable[NPTENTRIES] = {
	0x000000 | PTE_P | PTE_W,
	0x001000 | PTE_P | PTE_W,
	0x002000 | PTE_P | PTE_W,
	0x003000 | PTE_P | PTE_W,
	0x004000 | PTE_P | PTE_W,
	0x005000 | PTE_P | PTE_W,
	0x006000 | PTE_P | PTE_W,
	0x007000 | PTE_P | PTE_W,
	0x008000 | PTE_P | PTE_W,
	0x009000 | PTE_P | PTE_W,
	0x00a000 | PTE_P | PTE_W,
	0x00b000 | PTE_P | PTE_W,
	0x00c000 | PTE_P | PTE_W,
	0x00d000 | PTE_P | PTE_W,
	0x00e000 | PTE_P | PTE_W,
	0x00f000 | PTE_P | PTE_W,
	0x010000 | PTE_P | PTE_W,
	0x011000 | PTE_P | PTE_W,
	0x012000 | PTE_P | PTE_W,
	0x013000 | PTE_P | PTE_W,
	0x014000 | PTE_P | PTE_W,
	0x015000 | PTE_P | PTE_W,
	0x016000 | PTE_P | PTE_W,
	0x017000 | PTE_P | PTE_W,
	0x018000 | PTE_P | PTE_W,
	0x019000 | PTE_P | PTE_W,
	0x01a000 | PTE_P | PTE_W,
	0x01b000 | PTE_P | PTE_W,
	0x01c000 | PTE_P | PTE_W,
	0x01d000 | PTE_P | PTE_W,
	0x01e000 | PTE_P | PTE_W,
	0x01f000 | PTE_P | PTE_W,
	0x020000 | PTE_P | PTE_W,
	0x021000 | PTE_P | PTE_W,
	0x022000 | PTE_P | PTE_W,
	0x023000 | PTE_P | PTE_W,
	0x024000 | PTE_P | PTE_W,
	0x025000 | PTE_P | PTE_W,
	0x026000 | PTE_P | PTE_W,
	0x027000 | PTE_P | PTE_W,
	0x028000 | PTE_P | PTE_W,
	0x029000 | PTE_P | PTE_W,
	0x02a000 | PTE_P | PTE_W,
	0x02b000 | PTE_P | PTE_W,
	0x02c000 | PTE_P | PTE_W,
	0x02d000 | PTE_P | PTE_W,
	0x02e000 | PTE_P | PTE_W,
	0x02f000 | PTE_P | PTE_W,
	0x030000 | PTE_P | PTE_W,
	0x031000 | PTE_P | PTE_W,
	0x032000 | PTE_P | PTE_W,
	0x033000 | PTE_P | PTE_W,
	0x034000 | PTE_P | PTE_W,
	0x035000 | PTE_P | PTE_W,
	0x036000 | PTE_P | PTE_W,
	0x037000 | PTE_P | PTE_W,
	0x038000 | PTE_P | PTE_W,
	0x039000 | PTE_P | PTE_W,
	0x03a000 | PTE_P | PTE_W,
	0x03b000 | PTE_P | PTE_W,
	0x03c000 | PTE_P | PTE_W,
	0x03d000 | PTE_P | PTE_W,
	0x03e000 | PTE_P | PTE_W,
	0x03f000 | PTE_P | PTE_W,
	0x040000 | PTE_P | PTE_W,
	0x041000 | PTE_P | PTE_W,
	0x042000 | PTE_P | PTE_W,
	0x043000 | PTE_P | PTE_W,
	0x044000 | PTE_P | PTE_W,
	0x045000 | PTE_P | PTE_W,
	0x046000 | PTE_P | PTE_W,
	0x047000 | PTE_P | PTE_W,
	0x048000 | PTE_P | PTE_W,
	0x049000 | PTE_P | PTE_W,
	0x04a000 | PTE_P | PTE_W,
	0x04b000 | PTE_P | PTE_W,
	0x04c000 | PTE_P | PTE_W,
	0x04d000 | PTE_P | PTE_W,
	0x04e000 | PTE_P | PTE_W,
	0x04f000 | PTE_P | PTE_W,
	0x050000 | PTE_P | PTE_W,
	0x051000 | PTE_P | PTE_W,
	0x052000 | PTE_P | PTE_W,
	0x053000 | PTE_P | PTE_W,
	0x054000 | PTE_P | PTE_W,
	0x055000 | PTE_P | PTE_W,
	0x056000 | PTE_P | PTE_W,
	0x057000 | PTE_P | PTE_W,
	0x058000 | PTE_P | PTE_W,
	0x059000 | PTE_P | PTE_W,
	0x05a000 | PTE_P | PTE_W,
	0x05b000 | PTE_P | PTE_W,
	0x05c000 | PTE_P | PTE_W,
	0x05d000 | PTE_P | PTE_W,
	0x05e000 | PTE_P | PTE_W,
	0x05f000 | PTE_P | PTE_W,
	0x060000 | PTE_P | PTE_W,
	0x061000 | PTE_P | PTE_W,
	0x062000 | PTE_P | PTE_W,
	0x063000 | PTE_P | PTE_W,
	0x064000 | PTE_P | PTE_W,
	0x065000 | PTE_P | PTE_W,
	0x066000 | PTE_P | PTE_W,
	0x067000 | PTE_P | PTE_W,
	0x068000 | PTE_P | PTE_W,
	0x069000 | PTE_P | PTE_W,
	0x06a000 | PTE_P | PTE_W,
	0x06b000 | PTE_P | PTE_W,
	0x06c000 | PTE_P | PTE_W,
	0x06d000 | PTE_P | PTE_W,
	0x06e000 | PTE_P | PTE_W,
	0x06f000 | PTE_P | PTE_W,
	0x070000 | PTE_P | PTE_W,
	0x071000 | PTE_P | PTE_W,
	0x072000 | PTE_P | PTE_W,
	0x073000 | PTE_P | PTE_W,
	0x074000 | PTE_P | PTE_W,
	0x075000 | PTE_P | PTE_W,
	0x076000 | PTE_P | PTE_W,
	0x077000 | PTE_P | PTE_W,
	0x078000 | PTE_P | PTE_W,
	0x079000 | PTE_P | PTE_W,
	0x07a000 | PTE_P | PTE_W,
	0x07b000 | PTE_P | PTE_W,
	0x07c000 | PTE_P | PTE_W,
	0x07d000 | PTE_P | PTE_W,
	0x07e000 | PTE_P | PTE_W,
	0x07f000 | PTE_P | PTE_W,
	0x080000 | PTE_P | PTE_W,
	0x081000 | PTE_P | PTE_W,
	0x082000 | PTE_P | PTE_W,
	0x083000 | PTE_P | PTE_W,
	0x084000 | PTE_P | PTE_W,
	0x085000 | PTE_P | PTE_W,
	0x086000 | PTE_P | PTE_W,
	0x087000 | PTE_P | PTE_W,
	0x088000 | PTE_P | PTE_W,
	0x089000 | PTE_P | PTE_W,
	0x08a000 | PTE_P | PTE_W,
	0x08b000 | PTE_P | PTE_W,
	0x08c000 | PTE_P | PTE_W,
	0x08d000 | PTE_P | PTE_W,
	0x08e000 | PTE_P | PTE_W,
	0x08f000 | PTE_P | PTE_W,
	0x090000 | PTE_P | PTE_W,
	0x091000 | PTE_P | PTE_W,
	0x092000 | PTE_P | PTE_W,
	0x093000 | PTE_P | PTE_W,
	0x094000 | PTE_P | PTE_W,
	0x095000 | PTE_P | PTE_W,
	0x096000 | PTE_P | PTE_W,
	0x097000 | PTE_P | PTE_W,
	0x098000 | PTE_P | PTE_W,
	0x099000 | PTE_P | PTE_W,
	0x09a000 | PTE_P | PTE_W,
	0x09b000 | PTE_P | PTE_W,
	0x09c000 | PTE_P | PTE_W,
	0x09d000 | PTE_P | PTE_W,
	0x09e000 | PTE_P | PTE_W,
	0x09f000 | PTE_P | PTE_W,
	0x0a0000 | PTE_P | PTE_W,
	0x0a1000 | PTE_P | PTE_W,
	0x0a2000 | PTE_P | PTE_W,
	0x0a3000 | PTE_P | PTE_W,
	0x0a4000 | PTE_P | PTE_W,
	0x0a5000 | PTE_P | PTE_W,
	0x0a6000 | PTE_P | PTE_W,
	0x0a7000 | PTE_P | PTE_W,
	0x0a8000 | PTE_P | PTE_W,
	0x0a9000 | PTE_P | PTE_W,
	0x0aa000 | PTE_P | PTE_W,
	0x0ab000 | PTE_P | PTE_W,
	0x0ac000 | PTE_P | PTE_W,
	0x0ad000 | PTE_P | PTE_W,
	0x0ae000 | PTE_P | PTE_W,
	0x0af000 | PTE_P | PTE_W,
	0x0b0000 | PTE_P | PTE_W,
	0x0b1000 | PTE_P | PTE_W,
	0x0b2000 | PTE_P | PTE_W,
	0x0b3000 | PTE_P | PTE_W,
	0x0b4000 | PTE_P | PTE_W,
	0x0b5000 | PTE_P | PTE_W,
	0x0b6000 | PTE_P | PTE_W,
	0x0b7000 | PTE_P | PTE_W,
	0x0b8000 | PTE_P | PTE_W,
	0x0b9000 | PTE_P | PTE_W,
	0x0ba000 | PTE_P | PTE_W,
	0x0bb000 | PTE_P | PTE_W,
	0x0bc000 | PTE_P | PTE_W,
	0x0bd000 | PTE_P | PTE_W,
	0x0be000 | PTE_P | PTE_W,
	0x0bf000 | PTE_P | PTE_W,
	0x0c0000 | PTE_P | PTE_W,
	0x0c1000 | PTE_P | PTE_W,
	0x0c2000 | PTE_P | PTE_W,
	0x0c3000 | PTE_P | PTE_W,
	0x0c4000 | PTE_P | PTE_W,
	0x0c5000 | PTE_P | PTE_W,
	0x0c6000 | PTE_P | PTE_W,
	0x0c7000 | PTE_P | PTE_W,
	0x0c8000 | PTE_P | PTE_W,
	0x0c9000 | PTE_P | PTE_W,
	0x0ca000 | PTE_P | PTE_W,
	0x0cb000 | PTE_P | PTE_W,
	0x0cc000 | PTE_P | PTE_W,
	0x0cd000 | PTE_P | PTE_W,
	0x0ce000 | PTE_P | PTE_W,
	0x0cf000 | PTE_P | PTE_W,
	0x0d0000 | PTE_P | PTE_W,
	0x0d1000 | PTE_P | PTE_W,
	0x0d2000 | PTE_P | PTE_W,
	0x0d3000 | PTE_P | PTE_W,
	0x0d4000 | PTE_P | PTE_W,
	0x0d5000 | PTE_P | PTE_W,
	0x0d6000 | PTE_P | PTE_W,
	0x0d7000 | PTE_P | PTE_W,
	0x0d8000 | PTE_P | PTE_W,
	0x0d9000 | PTE_P | PTE_W,
	0x0da000 | PTE_P | PTE_W,
	0x0db000 | PTE_P | PTE_W,
	0x0dc000 | PTE_P | PTE_W,
	0x0dd000 | PTE_P | PTE_W,
	0x0de000 | PTE_P | PTE_W,
	0x0df000 | PTE_P | PTE_W,
	0x0e0000 | PTE_P | PTE_W,
	0x0e1000 | PTE_P | PTE_W,
	0x0e2000 | PTE_P | PTE_W,
	0x0e3000 | PTE_P | PTE_W,
	0x0e4000 | PTE_P | PTE_W,
	0x0e5000 | PTE_P | PTE_W,
	0x0e6000 | PTE_P | PTE_W,
	0x0e7000 | PTE_P | PTE_W,
	0x0e8000 | PTE_P | PTE_W,
	0x0e9000 | PTE_P | PTE_W,
	0x0ea000 | PTE_P | PTE_W,
	0x0eb000 | PTE_P | PTE_W,
	0x0ec000 | PTE_P | PTE_W,
	0x0ed000 | PTE_P | PTE_W,
	0x0ee000 | PTE_P | PTE_W,
	0x0ef000 | PTE_P | PTE_W,
	0x0f0000 | PTE_P | PTE_W,
	0x0f1000 | PTE_P | PTE_W,
	0x0f2000 | PTE_P | PTE_W,
	0x0f3000 | PTE_P | PTE_W,
	0x0f4000 | PTE_P | PTE_W,
	0x0f5000 | PTE_P | PTE_W,
	0x0f6000 | PTE_P | PTE_W,
	0x0f7000 | PTE_P | PTE_W,
	0x0f8000 | PTE_P | PTE_W,
	0x0f9000 | PTE_P | PTE_W,
	0x0fa000 | PTE_P | PTE_W,
	0x0fb000 | PTE_P | PTE_W,
	0x0fc000 | PTE_P | PTE_W,
	0x0fd000 | PTE_P | PTE_W,
	0x0fe000 | PTE_P | PTE_W,
	0x0ff000 | PTE_P | PTE_W,
	0x100000 | PTE_P | PTE_W,
	0x101000 | PTE_P | PTE_W,
	0x102000 | PTE_P | PTE_W,
	0x103000 | PTE_P | PTE_W,
	0x104000 | PTE_P | PTE_W,
	0x105000 | PTE_P | PTE_W,
	0x106000 | PTE_P | PTE_W,
	0x107000 | PTE_P | PTE_W,
	0x108000 | PTE_P | PTE_W,
	0x109000 | PTE_P | PTE_W,
	0x10a000 | PTE_P | PTE_W,
	0x10b000 | PTE_P | PTE_W,
	0x10c000 | PTE_P | PTE_W,
	0x10d000 | PTE_P | PTE_W,
	0x10e000 | PTE_P | PTE_W,
	0x10f000 | PTE_P | PTE_W,
	0x110000 | PTE_P | PTE_W,
	0x111000 | PTE_P | PTE_W,
	0x112000 | PTE_P | PTE_W,
	0x113000 | PTE_P | PTE_W,
	0x114000 | PTE_P | PTE_W,
	0x115000 | PTE_P | PTE_W,
	0x116000 | PTE_P | PTE_W,
	0x117000 | PTE_P | PTE_W,
	0x118000 | PTE_P | PTE_W,
	0x119000 | PTE_P | PTE_W,
	0x11a000 | PTE_P | PTE_W,
	0x11b000 | PTE_P | PTE_W,
	0x11c000 | PTE_P | PTE_W,
	0x11d000 | PTE_P | PTE_W,
	0x11e000 | PTE_P | PTE_W,
	0x11f000 | PTE_P | PTE_W,
	0x120000 | PTE_P | PTE_W,
	0x121000 | PTE_P | PTE_W,
	0x122000 | PTE_P | PTE_W,
	0x123000 | PTE_P | PTE_W,
	0x124000 | PTE_P | PTE_W,
	0x125000 | PTE_P | PTE_W,
	0x126000 | PTE_P | PTE_W,
	0x127000 | PTE_P | PTE_W,
	0x128000 | PTE_P | PTE_W,
	0x129000 | PTE_P | PTE_W,
	0x12a000 | PTE_P | PTE_W,
	0x12b000 | PTE_P | PTE_W,
	0x12c000 | PTE_P | PTE_W,
	0x12d000 | PTE_P | PTE_W,
	0x12e000 | PTE_P | PTE_W,
	0x12f000 | PTE_P | PTE_W,
	0x130000 | PTE_P | PTE_W,
	0x131000 | PTE_P | PTE_W,
	0x132000 | PTE_P | PTE_W,
	0x133000 | PTE_P | PTE_W,
	0x134000 | PTE_P | PTE_W,
	0x135000 | PTE_P | PTE_W,
	0x136000 | PTE_P | PTE_W,
	0x137000 | PTE_P | PTE_W,
	0x138000 | PTE_P | PTE_W,
	0x139000 | PTE_P | PTE_W,
	0x13a000 | PTE_P | PTE_W,
	0x13b000 | PTE_P | PTE_W,
	0x13c000 | PTE_P | PTE_W,
	0x13d000 | PTE_P | PTE_W,
	0x13e000 | PTE_P | PTE_W,
	0x13f000 | PTE_P | PTE_W,
	0x140000 | PTE_P | PTE_W,
	0x141000 | PTE_P | PTE_W,
	0x142000 | PTE_P | PTE_W,
	0x143000 | PTE_P | PTE_W,
	0x144000 | PTE_P | PTE_W,
	0x145000 | PTE_P | PTE_W,
	0x146000 | PTE_P | PTE_W,
	0x147000 | PTE_P | PTE_W,
	0x148000 | PTE_P | PTE_W,
	0x149000 | PTE_P | PTE_W,
	0x14a000 | PTE_P | PTE_W,
	0x14b000 | PTE_P | PTE_W,
	0x14c000 | PTE_P | PTE_W,
	0x14d000 | PTE_P | PTE_W,
	0x14e000 | PTE_P | PTE_W,
	0x14f000 | PTE_P | PTE_W,
	0x150000 | PTE_P | PTE_W,
	0x151000 | PTE_P | PTE_W,
	0x152000 | PTE_P | PTE_W,
	0x153000 | PTE_P | PTE_W,
	0x154000 | PTE_P | PTE_W,
	0x155000 | PTE_P | PTE_W,
	0x156000 | PTE_P | PTE_W,
	0x157000 | PTE_P | PTE_W,
	0x158000 | PTE_P | PTE_W,
	0x159000 | PTE_P | PTE_W,
	0x15a000 | PTE_P | PTE_W,
	0x15b000 | PTE_P | PTE_W,
	0x15c000 | PTE_P | PTE_W,
	0x15d000 | PTE_P | PTE_W,
	0x15e000 | PTE_P | PTE_W,
	0x15f000 | PTE_P | PTE_W,
	0x160000 | PTE_P | PTE_W,
	0x161000 | PTE_P | PTE_W,
	0x162000 | PTE_P | PTE_W,
	0x163000 | PTE_P | PTE_W,
	0x164000 | PTE_P | PTE_W,
	0x165000 | PTE_P | PTE_W,
	0x166000 | PTE_P | PTE_W,
	0x167000 | PTE_P | PTE_W,
	0x168000 | PTE_P | PTE_W,
	0x169000 | PTE_P | PTE_W,
	0x16a000 | PTE_P | PTE_W,
	0x16b000 | PTE_P | PTE_W,
	0x16c000 | PTE_P | PTE_W,
	0x16d000 | PTE_P | PTE_W,
	0x16e000 | PTE_P | PTE_W,
	0x16f000 | PTE_P | PTE_W,
	0x170000 | PTE_P | PTE_W,
	0x171000 | PTE_P | PTE_W,
	0x172000 | PTE_P | PTE_W,
	0x173000 | PTE_P | PTE_W,
	0x174000 | PTE_P | PTE_W,
	0x175000 | PTE_P | PTE_W,
	0x176000 | PTE_P | PTE_W,
	0x177000 | PTE_P | PTE_W,
	0x178000 | PTE_P | PTE_W,
	0x179000 | PTE_P | PTE_W,
	0x17a000 | PTE_P | PTE_W,
	0x17b000 | PTE_P | PTE_W,
	0x17c000 | PTE_P | PTE_W,
	0x17d000 | PTE_P | PTE_W,
	0x17e000 | PTE_P | PTE_W,
	0x17f000 | PTE_P | PTE_W,
	0x180000 | PTE_P | PTE_W,
	0x181000 | PTE_P | PTE_W,
	0x182000 | PTE_P | PTE_W,
	0x183000 | PTE_P | PTE_W,
	0x184000 | PTE_P | PTE_W,
	0x185000 | PTE_P | PTE_W,
	0x186000 | PTE_P | PTE_W,
	0x187000 | PTE_P | PTE_W,
	0x188000 | PTE_P | PTE_W,
	0x189000 | PTE_P | PTE_W,
	0x18a000 | PTE_P | PTE_W,
	0x18b000 | PTE_P | PTE_W,
	0x18c000 | PTE_P | PTE_W,
	0x18d000 | PTE_P | PTE_W,
	0x18e000 | PTE_P | PTE_W,
	0x18f000 | PTE_P | PTE_W,
	0x190000 | PTE_P | PTE_W,
	0x191000 | PTE_P | PTE_W,
	0x192000 | PTE_P | PTE_W,
	0x193000 | PTE_P | PTE_W,
	0x194000 | PTE_P | PTE_W,
	0x195000 | PTE_P | PTE_W,
	0x196000 | PTE_P | PTE_W,
	0x197000 | PTE_P | PTE_W,
	0x198000 | PTE_P | PTE_W,
	0x199000 | PTE_P | PTE_W,
	0x19a000 | PTE_P | PTE_W,
	0x19b000 | PTE_P | PTE_W,
	0x19c000 | PTE_P | PTE_W,
	0x19d000 | PTE_P | PTE_W,
	0x19e000 | PTE_P | PTE_W,
	0x19f000 | PTE_P | PTE_W,
	0x1a0000 | PTE_P | PTE_W,
	0x1a1000 | PTE_P | PTE_W,
	0x1a2000 | PTE_P | PTE_W,
	0x1a3000 | PTE_P | PTE_W,
	0x1a4000 | PTE_P | PTE_W,
	0x1a5000 | PTE_P | PTE_W,
	0x1a6000 | PTE_P | PTE_W,
	0x1a7000 | PTE_P | PTE_W,
	0x1a8000 | PTE_P | PTE_W,
	0x1a9000 | PTE_P | PTE_W,
	0x1aa000 | PTE_P | PTE_W,
	0x1ab000 | PTE_P | PTE_W,
	0x1ac000 | PTE_P | PTE_W,
	0x1ad000 | PTE_P | PTE_W,
	0x1ae000 | PTE_P | PTE_W,
	0x1af000 | PTE_P | PTE_W,
	0x1b0000 | PTE_P | PTE_W,
	0x1b1000 | PTE_P | PTE_W,
	0x1b2000 | PTE_P | PTE_W,
	0x1b3000 | PTE_P | PTE_W,
	0x1b4000 | PTE_P | PTE_W,
	0x1b5000 | PTE_P | PTE_W,
	0x1b6000 | PTE_P | PTE_W,
	0x1b7000 | PTE_P | PTE_W,
	0x1b8000 | PTE_P | PTE_W,
	0x1b9000 | PTE_P | PTE_W,
	0x1ba000 | PTE_P | PTE_W,
	0x1bb000 | PTE_P | PTE_W,
	0x1bc000 | PTE_P | PTE_W,
	0x1bd000 | PTE_P | PTE_W,
	0x1be000 | PTE_P | PTE_W,
	0x1bf000 | PTE_P | PTE_W,
	0x1c0000 | PTE_P | PTE_W,
	0x1c1000 | PTE_P | PTE_W,
	0x1c2000 | PTE_P | PTE_W,
	0x1c3000 | PTE_P | PTE_W,
	0x1c4000 | PTE_P | PTE_W,
	0x1c5000 | PTE_P | PTE_W,
	0x1c6000 | PTE_P | PTE_W,
	0x1c7000 | PTE_P | PTE_W,
	0x1c8000 | PTE_P | PTE_W,
	0x1c9000 | PTE_P | PTE_W,
	0x1ca000 | PTE_P | PTE_W,
	0x1cb000 | PTE_P | PTE_W,
	0x1cc000 | PTE_P | PTE_W,
	0x1cd000 | PTE_P | PTE_W,
	0x1ce000 | PTE_P | PTE_W,
	0x1cf000 | PTE_P | PTE_W,
	0x1d0000 | PTE_P | PTE_W,
	0x1d1000 | PTE_P | PTE_W,
	0x1d2000 | PTE_P | PTE_W,
	0x1d3000 | PTE_P | PTE_W,
	0x1d4000 | PTE_P | PTE_W,
	0x1d5000 | PTE_P | PTE_W,
	0x1d6000 | PTE_P | PTE_W,
	0x1d7000 | PTE_P | PTE_W,
	0x1d8000 | PTE_P | PTE_W,
	0x1d9000 | PTE_P | PTE_W,
	0x1da000 | PTE_P | PTE_W,
	0x1db000 | PTE_P | PTE_W,
	0x1dc000 | PTE_P | PTE_W,
	0x1dd000 | PTE_P | PTE_W,
	0x1de000 | PTE_P | PTE_W,
	0x1df000 | PTE_P | PTE_W,
	0x1e0000 | PTE_P | PTE_W,
	0x1e1000 | PTE_P | PTE_W,
	0x1e2000 | PTE_P | PTE_W,
	0x1e3000 | PTE_P | PTE_W,
	0x1e4000 | PTE_P | PTE_W,
	0x1e5000 | PTE_P | PTE_W,
	0x1e6000 | PTE_P | PTE_W,
	0x1e7000 | PTE_P | PTE_W,
	0x1e8000 | PTE_P | PTE_W,
	0x1e9000 | PTE_P | PTE_W,
	0x1ea000 | PTE_P | PTE_W,
	0x1eb000 | PTE_P | PTE_W,
	0x1ec000 | PTE_P | PTE_W,
	0x1ed000 | PTE_P | PTE_W,
	0x1ee000 | PTE_P | PTE_W,
	0x1ef000 | PTE_P | PTE_W,
	0x1f0000 | PTE_P | PTE_W,
	0x1f1000 | PTE_P | PTE_W,
	0x1f2000 | PTE_P | PTE_W,
	0x1f3000 | PTE_P | PTE_W,
	0x1f4000 | PTE_P | PTE_W,
	0x1f5000 | PTE_P | PTE_W,
	0x1f6000 | PTE_P | PTE_W,
	0x1f7000 | PTE_P | PTE_W,
	0x1f8000 | PTE_P | PTE_W,
	0x1f9000 | PTE_P | PTE_W,
	0x1fa000 | PTE_P | PTE_W,
	0x1fb000 | PTE_P | PTE_W,
	0x1fc000 | PTE_P | PTE_W,
	0x1fd000 | PTE_P | PTE_W,
	0x1fe000 | PTE_P | PTE_W,
	0x1ff000 | PTE_P | PTE_W,
	0x200000 | PTE_P | PTE_W,
	0x201000 | PTE_P | PTE_W,
	0x202000 | PTE_P | PTE_W,
	0x203000 | PTE_P | PTE_W,
	0x204000 | PTE_P | PTE_W,
	0x205000 | PTE_P | PTE_W,
	0x206000 | PTE_P | PTE_W,
	0x207000 | PTE_P | PTE_W,
	0x208000 | PTE_P | PTE_W,
	0x209000 | PTE_P | PTE_W,
	0x20a000 | PTE_P | PTE_W,
	0x20b000 | PTE_P | PTE_W,
	0x20c000 | PTE_P | PTE_W,
	0x20d000 | PTE_P | PTE_W,
	0x20e000 | PTE_P | PTE_W,
	0x20f000 | PTE_P | PTE_W,
	0x210000 | PTE_P | PTE_W,
	0x211000 | PTE_P | PTE_W,
	0x212000 | PTE_P | PTE_W,
	0x213000 | PTE_P | PTE_W,
	0x214000 | PTE_P | PTE_W,
	0x215000 | PTE_P | PTE_W,
	0x216000 | PTE_P | PTE_W,
	0x217000 | PTE_P | PTE_W,
	0x218000 | PTE_P | PTE_W,
	0x219000 | PTE_P | PTE_W,
	0x21a000 | PTE_P | PTE_W,
	0x21b000 | PTE_P | PTE_W,
	0x21c000 | PTE_P | PTE_W,
	0x21d000 | PTE_P | PTE_W,
	0x21e000 | PTE_P | PTE_W,
	0x21f000 | PTE_P | PTE_W,
	0x220000 | PTE_P | PTE_W,
	0x221000 | PTE_P | PTE_W,
	0x222000 | PTE_P | PTE_W,
	0x223000 | PTE_P | PTE_W,
	0x224000 | PTE_P | PTE_W,
	0x225000 | PTE_P | PTE_W,
	0x226000 | PTE_P | PTE_W,
	0x227000 | PTE_P | PTE_W,
	0x228000 | PTE_P | PTE_W,
	0x229000 | PTE_P | PTE_W,
	0x22a000 | PTE_P | PTE_W,
	0x22b000 | PTE_P | PTE_W,
	0x22c000 | PTE_P | PTE_W,
	0x22d000 | PTE_P | PTE_W,
	0x22e000 | PTE_P | PTE_W,
	0x22f000 | PTE_P | PTE_W,
	0x230000 | PTE_P | PTE_W,
	0x231000 | PTE_P | PTE_W,
	0x232000 | PTE_P | PTE_W,
	0x233000 | PTE_P | PTE_W,
	0x234000 | PTE_P | PTE_W,
	0x235000 | PTE_P | PTE_W,
	0x236000 | PTE_P | PTE_W,
	0x237000 | PTE_P | PTE_W,
	0x238000 | PTE_P | PTE_W,
	0x239000 | PTE_P | PTE_W,
	0x23a000 | PTE_P | PTE_W,
	0x23b000 | PTE_P | PTE_W,
	0x23c000 | PTE_P | PTE_W,
	0x23d000 | PTE_P | PTE_W,
	0x23e000 | PTE_P | PTE_W,
	0x23f000 | PTE_P | PTE_W,
	0x240000 | PTE_P | PTE_W,
	0x241000 | PTE_P | PTE_W,
	0x242000 | PTE_P | PTE_W,
	0x243000 | PTE_P | PTE_W,
	0x244000 | PTE_P | PTE_W,
	0x245000 | PTE_P | PTE_W,
	0x246000 | PTE_P | PTE_W,
	0x247000 | PTE_P | PTE_W,
	0x248000 | PTE_P | PTE_W,
	0x249000 | PTE_P | PTE_W,
	0x24a000 | PTE_P | PTE_W,
	0x24b000 | PTE_P | PTE_W,
	0x24c000 | PTE_P | PTE_W,
	0x24d000 | PTE_P | PTE_W,
	0x24e000 | PTE_P | PTE_W,
	0x24f000 | PTE_P | PTE_W,
	0x250000 | PTE_P | PTE_W,
	0x251000 | PTE_P | PTE_W,
	0x252000 | PTE_P | PTE_W,
	0x253000 | PTE_P | PTE_W,
	0x254000 | PTE_P | PTE_W,
	0x255000 | PTE_P | PTE_W,
	0x256000 | PTE_P | PTE_W,
	0x257000 | PTE_P | PTE_W,
	0x258000 | PTE_P | PTE_W,
	0x259000 | PTE_P | PTE_W,
	0x25a000 | PTE_P | PTE_W,
	0x25b000 | PTE_P | PTE_W,
	0x25c000 | PTE_P | PTE_W,
	0x25d000 | PTE_P | PTE_W,
	0x25e000 | PTE_P | PTE_W,
	0x25f000 | PTE_P | PTE_W,
	0x260000 | PTE_P | PTE_W,
	0x261000 | PTE_P | PTE_W,
	0x262000 | PTE_P | PTE_W,
	0x263000 | PTE_P | PTE_W,
	0x264000 | PTE_P | PTE_W,
	0x265000 | PTE_P | PTE_W,
	0x266000 | PTE_P | PTE_W,
	0x267000 | PTE_P | PTE_W,
	0x268000 | PTE_P | PTE_W,
	0x269000 | PTE_P | PTE_W,
	0x26a000 | PTE_P | PTE_W,
	0x26b000 | PTE_P | PTE_W,
	0x26c000 | PTE_P | PTE_W,
	0x26d000 | PTE_P | PTE_W,
	0x26e000 | PTE_P | PTE_W,
	0x26f000 | PTE_P | PTE_W,
	0x270000 | PTE_P | PTE_W,
	0x271000 | PTE_P | PTE_W,
	0x272000 | PTE_P | PTE_W,
	0x273000 | PTE_P | PTE_W,
	0x274000 | PTE_P | PTE_W,
	0x275000 | PTE_P | PTE_W,
	0x276000 | PTE_P | PTE_W,
	0x277000 | PTE_P | PTE_W,
	0x278000 | PTE_P | PTE_W,
	0x279000 | PTE_P | PTE_W,
	0x27a000 | PTE_P | PTE_W,
	0x27b000 | PTE_P | PTE_W,
	0x27c000 | PTE_P | PTE_W,
	0x27d000 | PTE_P | PTE_W,
	0x27e000 | PTE_P | PTE_W,
	0x27f000 | PTE_P | PTE_W,
	0x280000 | PTE_P | PTE_W,
	0x281000 | PTE_P | PTE_W,
	0x282000 | PTE_P | PTE_W,
	0x283000 | PTE_P | PTE_W,
	0x284000 | PTE_P | PTE_W,
	0x285000 | PTE_P | PTE_W,
	0x286000 | PTE_P | PTE_W,
	0x287000 | PTE_P | PTE_W,
	0x288000 | PTE_P | PTE_W,
	0x289000 | PTE_P | PTE_W,
	0x28a000 | PTE_P | PTE_W,
	0x28b000 | PTE_P | PTE_W,
	0x28c000 | PTE_P | PTE_W,
	0x28d000 | PTE_P | PTE_W,
	0x28e000 | PTE_P | PTE_W,
	0x28f000 | PTE_P | PTE_W,
	0x290000 | PTE_P | PTE_W,
	0x291000 | PTE_P | PTE_W,
	0x292000 | PTE_P | PTE_W,
	0x293000 | PTE_P | PTE_W,
	0x294000 | PTE_P | PTE_W,
	0x295000 | PTE_P | PTE_W,
	0x296000 | PTE_P | PTE_W,
	0x297000 | PTE_P | PTE_W,
	0x298000 | PTE_P | PTE_W,
	0x299000 | PTE_P | PTE_W,
	0x29a000 | PTE_P | PTE_W,
	0x29b000 | PTE_P | PTE_W,
	0x29c000 | PTE_P | PTE_W,
	0x29d000 | PTE_P | PTE_W,
	0x29e000 | PTE_P | PTE_W,
	0x29f000 | PTE_P | PTE_W,
	0x2a0000 | PTE_P | PTE_W,
	0x2a1000 | PTE_P | PTE_W,
	0x2a2000 | PTE_P | PTE_W,
	0x2a3000 | PTE_P | PTE_W,
	0x2a4000 | PTE_P | PTE_W,
	0x2a5000 | PTE_P | PTE_W,
	0x2a6000 | PTE_P | PTE_W,
	0x2a7000 | PTE_P | PTE_W,
	0x2a8000 | PTE_P | PTE_W,
	0x2a9000 | PTE_P | PTE_W,
	0x2aa000 | PTE_P | PTE_W,
	0x2ab000 | PTE_P | PTE_W,
	0x2ac000 | PTE_P | PTE_W,
	0x2ad000 | PTE_P | PTE_W,
	0x2ae000 | PTE_P | PTE_W,
	0x2af000 | PTE_P | PTE_W,
	0x2b0000 | PTE_P | PTE_W,
	0x2b1000 | PTE_P | PTE_W,
	0x2b2000 | PTE_P | PTE_W,
	0x2b3000 | PTE_P | PTE_W,
	0x2b4000 | PTE_P | PTE_W,
	0x2b5000 | PTE_P | PTE_W,
	0x2b6000 | PTE_P | PTE_W,
	0x2b7000 | PTE_P | PTE_W,
	0x2b8000 | PTE_P | PTE_W,
	0x2b9000 | PTE_P | PTE_W,
	0x2ba000 | PTE_P | PTE_W,
	0x2bb000 | PTE_P | PTE_W,
	0x2bc000 | PTE_P | PTE_W,
	0x2bd000 | PTE_P | PTE_W,
	0x2be000 | PTE_P | PTE_W,
	0x2bf000 | PTE_P | PTE_W,
	0x2c0000 | PTE_P | PTE_W,
	0x2c1000 | PTE_P | PTE_W,
	0x2c2000 | PTE_P | PTE_W,
	0x2c3000 | PTE_P | PTE_W,
	0x2c4000 | PTE_P | PTE_W,
	0x2c5000 | PTE_P | PTE_W,
	0x2c6000 | PTE_P | PTE_W,
	0x2c7000 | PTE_P | PTE_W,
	0x2c8000 | PTE_P | PTE_W,
	0x2c9000 | PTE_P | PTE_W,
	0x2ca000 | PTE_P | PTE_W,
	0x2cb000 | PTE_P | PTE_W,
	0x2cc000 | PTE_P | PTE_W,
	0x2cd000 | PTE_P | PTE_W,
	0x2ce000 | PTE_P | PTE_W,
	0x2cf000 | PTE_P | PTE_W,
	0x2d0000 | PTE_P | PTE_W,
	0x2d1000 | PTE_P | PTE_W,
	0x2d2000 | PTE_P | PTE_W,
	0x2d3000 | PTE_P | PTE_W,
	0x2d4000 | PTE_P | PTE_W,
	0x2d5000 | PTE_P | PTE_W,
	0x2d6000 | PTE_P | PTE_W,
	0x2d7000 | PTE_P | PTE_W,
	0x2d8000 | PTE_P | PTE_W,
	0x2d9000 | PTE_P | PTE_W,
	0x2da000 | PTE_P | PTE_W,
	0x2db000 | PTE_P | PTE_W,
	0x2dc000 | PTE_P | PTE_W,
	0x2dd000 | PTE_P | PTE_W,
	0x2de000 | PTE_P | PTE_W,
	0x2df000 | PTE_P | PTE_W,
	0x2e0000 | PTE_P | PTE_W,
	0x2e1000 | PTE_P | PTE_W,
	0x2e2000 | PTE_P | PTE_W,
	0x2e3000 | PTE_P | PTE_W,
	0x2e4000 | PTE_P | PTE_W,
	0x2e5000 | PTE_P | PTE_W,
	0x2e6000 | PTE_P | PTE_W,
	0x2e7000 | PTE_P | PTE_W,
	0x2e8000 | PTE_P | PTE_W,
	0x2e9000 | PTE_P | PTE_W,
	0x2ea000 | PTE_P | PTE_W,
	0x2eb000 | PTE_P | PTE_W,
	0x2ec000 | PTE_P | PTE_W,
	0x2ed000 | PTE_P | PTE_W,
	0x2ee000 | PTE_P | PTE_W,
	0x2ef000 | PTE_P | PTE_W,
	0x2f0000 | PTE_P | PTE_W,
	0x2f1000 | PTE_P | PTE_W,
	0x2f2000 | PTE_P | PTE_W,
	0x2f3000 | PTE_P | PTE_W,
	0x2f4000 | PTE_P | PTE_W,
	0x2f5000 | PTE_P | PTE_W,
	0x2f6000 | PTE_P | PTE_W,
	0x2f7000 | PTE_P | PTE_W,
	0x2f8000 | PTE_P | PTE_W,
	0x2f9000 | PTE_P | PTE_W,
	0x2fa000 | PTE_P | PTE_W,
	0x2fb000 | PTE_P | PTE_W,
	0x2fc000 | PTE_P | PTE_W,
	0x2fd000 | PTE_P | PTE_W,
	0x2fe000 | PTE_P | PTE_W,
	0x2ff000 | PTE_P | PTE_W,
	0x300000 | PTE_P | PTE_W,
	0x301000 | PTE_P | PTE_W,
	0x302000 | PTE_P | PTE_W,
	0x303000 | PTE_P | PTE_W,
	0x304000 | PTE_P | PTE_W,
	0x305000 | PTE_P | PTE_W,
	0x306000 | PTE_P | PTE_W,
	0x307000 | PTE_P | PTE_W,
	0x308000 | PTE_P | PTE_W,
	0x309000 | PTE_P | PTE_W,
	0x30a000 | PTE_P | PTE_W,
	0x30b000 | PTE_P | PTE_W,
	0x30c000 | PTE_P | PTE_W,
	0x30d000 | PTE_P | PTE_W,
	0x30e000 | PTE_P | PTE_W,
	0x30f000 | PTE_P | PTE_W,
	0x310000 | PTE_P | PTE_W,
	0x311000 | PTE_P | PTE_W,
	0x312000 | PTE_P | PTE_W,
	0x313000 | PTE_P | PTE_W,
	0x314000 | PTE_P | PTE_W,
	0x315000 | PTE_P | PTE_W,
	0x316000 | PTE_P | PTE_W,
	0x317000 | PTE_P | PTE_W,
	0x318000 | PTE_P | PTE_W,
	0x319000 | PTE_P | PTE_W,
	0x31a000 | PTE_P | PTE_W,
	0x31b000 | PTE_P | PTE_W,
	0x31c000 | PTE_P | PTE_W,
	0x31d000 | PTE_P | PTE_W,
	0x31e000 | PTE_P | PTE_W,
	0x31f000 | PTE_P | PTE_W,
	0x320000 | PTE_P | PTE_W,
	0x321000 | PTE_P | PTE_W,
	0x322000 | PTE_P | PTE_W,
	0x323000 | PTE_P | PTE_W,
	0x324000 | PTE_P | PTE_W,
	0x325000 | PTE_P | PTE_W,
	0x326000 | PTE_P | PTE_W,
	0x327000 | PTE_P | PTE_W,
	0x328000 | PTE_P | PTE_W,
	0x329000 | PTE_P | PTE_W,
	0x32a000 | PTE_P | PTE_W,
	0x32b000 | PTE_P | PTE_W,
	0x32c000 | PTE_P | PTE_W,
	0x32d000 | PTE_P | PTE_W,
	0x32e000 | PTE_P | PTE_W,
	0x32f000 | PTE_P | PTE_W,
	0x330000 | PTE_P | PTE_W,
	0x331000 | PTE_P | PTE_W,
	0x332000 | PTE_P | PTE_W,
	0x333000 | PTE_P | PTE_W,
	0x334000 | PTE_P | PTE_W,
	0x335000 | PTE_P | PTE_W,
	0x336000 | PTE_P | PTE_W,
	0x337000 | PTE_P | PTE_W,
	0x338000 | PTE_P | PTE_W,
	0x339000 | PTE_P | PTE_W,
	0x33a000 | PTE_P | PTE_W,
	0x33b000 | PTE_P | PTE_W,
	0x33c000 | PTE_P | PTE_W,
	0x33d000 | PTE_P | PTE_W,
	0x33e000 | PTE_P | PTE_W,
	0x33f000 | PTE_P | PTE_W,
	0x340000 | PTE_P | PTE_W,
	0x341000 | PTE_P | PTE_W,
	0x342000 | PTE_P | PTE_W,
	0x343000 | PTE_P | PTE_W,
	0x344000 | PTE_P | PTE_W,
	0x345000 | PTE_P | PTE_W,
	0x346000 | PTE_P | PTE_W,
	0x347000 | PTE_P | PTE_W,
	0x348000 | PTE_P | PTE_W,
	0x349000 | PTE_P | PTE_W,
	0x34a000 | PTE_P | PTE_W,
	0x34b000 | PTE_P | PTE_W,
	0x34c000 | PTE_P | PTE_W,
	0x34d000 | PTE_P | PTE_W,
	0x34e000 | PTE_P | PTE_W,
	0x34f000 | PTE_P | PTE_W,
	0x350000 | PTE_P | PTE_W,
	0x351000 | PTE_P | PTE_W,
	0x352000 | PTE_P | PTE_W,
	0x353000 | PTE_P | PTE_W,
	0x354000 | PTE_P | PTE_W,
	0x355000 | PTE_P | PTE_W,
	0x356000 | PTE_P | PTE_W,
	0x357000 | PTE_P | PTE_W,
	0x358000 | PTE_P | PTE_W,
	0x359000 | PTE_P | PTE_W,
	0x35a000 | PTE_P | PTE_W,
	0x35b000 | PTE_P | PTE_W,
	0x35c000 | PTE_P | PTE_W,
	0x35d000 | PTE_P | PTE_W,
	0x35e000 | PTE_P | PTE_W,
	0x35f000 | PTE_P | PTE_W,
	0x360000 | PTE_P | PTE_W,
	0x361000 | PTE_P | PTE_W,
	0x362000 | PTE_P | PTE_W,
	0x363000 | PTE_P | PTE_W,
	0x364000 | PTE_P | PTE_W,
	0x365000 | PTE_P | PTE_W,
	0x366000 | PTE_P | PTE_W,
	0x367000 | PTE_P | PTE_W,
	0x368000 | PTE_P | PTE_W,
	0x369000 | PTE_P | PTE_W,
	0x36a000 | PTE_P | PTE_W,
	0x36b000 | PTE_P | PTE_W,
	0x36c000 | PTE_P | PTE_W,
	0x36d000 | PTE_P | PTE_W,
	0x36e000 | PTE_P | PTE_W,
	0x36f000 | PTE_P | PTE_W,
	0x370000 | PTE_P | PTE_W,
	0x371000 | PTE_P | PTE_W,
	0x372000 | PTE_P | PTE_W,
	0x373000 | PTE_P | PTE_W,
	0x374000 | PTE_P | PTE_W,
	0x375000 | PTE_P | PTE_W,
	0x376000 | PTE_P | PTE_W,
	0x377000 | PTE_P | PTE_W,
	0x378000 | PTE_P | PTE_W,
	0x379000 | PTE_P | PTE_W,
	0x37a000 | PTE_P | PTE_W,
	0x37b000 | PTE_P | PTE_W,
	0x37c000 | PTE_P | PTE_W,
	0x37d000 | PTE_P | PTE_W,
	0x37e000 | PTE_P | PTE_W,
	0x37f000 | PTE_P | PTE_W,
	0x380000 | PTE_P | PTE_W,
	0x381000 | PTE_P | PTE_W,
	0x382000 | PTE_P | PTE_W,
	0x383000 | PTE_P | PTE_W,
	0x384000 | PTE_P | PTE_W,
	0x385000 | PTE_P | PTE_W,
	0x386000 | PTE_P | PTE_W,
	0x387000 | PTE_P | PTE_W,
	0x388000 | PTE_P | PTE_W,
	0x389000 | PTE_P | PTE_W,
	0x38a000 | PTE_P | PTE_W,
	0x38b000 | PTE_P | PTE_W,
	0x38c000 | PTE_P | PTE_W,
	0x38d000 | PTE_P | PTE_W,
	0x38e000 | PTE_P | PTE_W,
	0x38f000 | PTE_P | PTE_W,
	0x390000 | PTE_P | PTE_W,
	0x391000 | PTE_P | PTE_W,
	0x392000 | PTE_P | PTE_W,
	0x393000 | PTE_P | PTE_W,
	0x394000 | PTE_P | PTE_W,
	0x395000 | PTE_P | PTE_W,
	0x396000 | PTE_P | PTE_W,
	0x397000 | PTE_P | PTE_W,
	0x398000 | PTE_P | PTE_W,
	0x399000 | PTE_P | PTE_W,
	0x39a000 | PTE_P | PTE_W,
	0x39b000 | PTE_P | PTE_W,
	0x39c000 | PTE_P | PTE_W,
	0x39d000 | PTE_P | PTE_W,
	0x39e000 | PTE_P | PTE_W,
	0x39f000 | PTE_P | PTE_W,
	0x3a0000 | PTE_P | PTE_W,
	0x3a1000 | PTE_P | PTE_W,
	0x3a2000 | PTE_P | PTE_W,
	0x3a3000 | PTE_P | PTE_W,
	0x3a4000 | PTE_P | PTE_W,
	0x3a5000 | PTE_P | PTE_W,
	0x3a6000 | PTE_P | PTE_W,
	0x3a7000 | PTE_P | PTE_W,
	0x3a8000 | PTE_P | PTE_W,
	0x3a9000 | PTE_P | PTE_W,
	0x3aa000 | PTE_P | PTE_W,
	0x3ab000 | PTE_P | PTE_W,
	0x3ac000 | PTE_P | PTE_W,
	0x3ad000 | PTE_P | PTE_W,
	0x3ae000 | PTE_P | PTE_W,
	0x3af000 | PTE_P | PTE_W,
	0x3b0000 | PTE_P | PTE_W,
	0x3b1000 | PTE_P | PTE_W,
	0x3b2000 | PTE_P | PTE_W,
	0x3b3000 | PTE_P | PTE_W,
	0x3b4000 | PTE_P | PTE_W,
	0x3b5000 | PTE_P | PTE_W,
	0x3b6000 | PTE_P | PTE_W,
	0x3b7000 | PTE_P | PTE_W,
	0x3b8000 | PTE_P | PTE_W,
	0x3b9000 | PTE_P | PTE_W,
	0x3ba000 | PTE_P | PTE_W,
	0x3bb000 | PTE_P | PTE_W,
	0x3bc000 | PTE_P | PTE_W,
	0x3bd000 | PTE_P | PTE_W,
	0x3be000 | PTE_P | PTE_W,
	0x3bf000 | PTE_P | PTE_W,
	0x3c0000 | PTE_P | PTE_W,
	0x3c1000 | PTE_P | PTE_W,
	0x3c2000 | PTE_P | PTE_W,
	0x3c3000 | PTE_P | PTE_W,
	0x3c4000 | PTE_P | PTE_W,
	0x3c5000 | PTE_P | PTE_W,
	0x3c6000 | PTE_P | PTE_W,
	0x3c7000 | PTE_P | PTE_W,
	0x3c8000 | PTE_P | PTE_W,
	0x3c9000 | PTE_P | PTE_W,
	0x3ca000 | PTE_P | PTE_W,
	0x3cb000 | PTE_P | PTE_W,
	0x3cc000 | PTE_P | PTE_W,
	0x3cd000 | PTE_P | PTE_W,
	0x3ce000 | PTE_P | PTE_W,
	0x3cf000 | PTE_P | PTE_W,
	0x3d0000 | PTE_P | PTE_W,
	0x3d1000 | PTE_P | PTE_W,
	0x3d2000 | PTE_P | PTE_W,
	0x3d3000 | PTE_P | PTE_W,
	0x3d4000 | PTE_P | PTE_W,
	0x3d5000 | PTE_P | PTE_W,
	0x3d6000 | PTE_P | PTE_W,
	0x3d7000 | PTE_P | PTE_W,
	0x3d8000 | PTE_P | PTE_W,
	0x3d9000 | PTE_P | PTE_W,
	0x3da000 | PTE_P | PTE_W,
	0x3db000 | PTE_P | PTE_W,
	0x3dc000 | PTE_P | PTE_W,
	0x3dd000 | PTE_P | PTE_W,
	0x3de000 | PTE_P | PTE_W,
	0x3df000 | PTE_P | PTE_W,
	0x3e0000 | PTE_P | PTE_W,
	0x3e1000 | PTE_P | PTE_W,
	0x3e2000 | PTE_P | PTE_W,
	0x3e3000 | PTE_P | PTE_W,
	0x3e4000 | PTE_P | PTE_W,
	0x3e5000 | PTE_P | PTE_W,
	0x3e6000 | PTE_P | PTE_W,
	0x3e7000 | PTE_P | PTE_W,
	0x3e8000 | PTE_P | PTE_W,
	0x3e9000 | PTE_P | PTE_W,
	0x3ea000 | PTE_P | PTE_W,
	0x3eb000 | PTE_P | PTE_W,
	0x3ec000 | PTE_P | PTE_W,
	0x3ed000 | PTE_P | PTE_W,
	0x3ee000 | PTE_P | PTE_W,
	0x3ef000 | PTE_P | PTE_W,
	0x3f0000 | PTE_P | PTE_W,
	0x3f1000 | PTE_P | PTE_W,
	0x3f2000 | PTE_P | PTE_W,
	0x3f3000 | PTE_P | PTE_W,
	0x3f4000 | PTE_P | PTE_W,
	0x3f5000 | PTE_P | PTE_W,
	0x3f6000 | PTE_P | PTE_W,
	0x3f7000 | PTE_P | PTE_W,
	0x3f8000 | PTE_P | PTE_W,
	0x3f9000 | PTE_P | PTE_W,
	0x3fa000 | PTE_P | PTE_W,
	0x3fb000 | PTE_P | PTE_W,
	0x3fc000 | PTE_P | PTE_W,
	0x3fd000 | PTE_P | PTE_W,
	0x3fe000 | PTE_P | PTE_W,
	0x3ff000 | PTE_P | PTE_W,
};
#endif
