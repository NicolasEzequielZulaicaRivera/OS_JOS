TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0xf0100000 0x00100000 0x07df2 0x07df2 R E 0x1000
  LOAD           0x009000 0xf0108000 0x00108000 0x0a300 0x0a980 RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10
```

`0x00108000` + `0x0a980` = `0xf0112980`

```
(gdb) b boot_alloc
Breakpoint 1 at 0xf0100987: file kern/pmap.c, line 98.
(gdb) c
Continuing.
The target architecture is set to "i386".
=> 0xf0100987 <boot_alloc>:     cmpl   $0x0,0xf0112564

Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:98
98              if (!nextfree) {
(gdb) fin
Run till exit from #0  boot_alloc (n=4096) at kern/pmap.c:98
=> 0xf0100a8b <mem_init+22>:    mov    %eax,0xf011255c
mem_init () at kern/pmap.c:136
136             kern_pgdir = (pde_t *) boot_alloc(PGSIZE);
Value returned is $1 = (void *) 0xf0113000
```


page_alloc
----------

...


map_region_large
----------------

...

