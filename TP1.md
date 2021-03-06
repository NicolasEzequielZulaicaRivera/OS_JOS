# TP1: Memoria virtual en JOS

## boot_alloc_pos

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


## page_alloc

### ¿en qué se diferencia page2pa() de page2kva()?
**page2pa()**: Obtiene la dirección física de un puntero a un struct PageInfo

**page2kva()**: Obtiene la dirección virtual de un puntero a un struct PageInfo


map_region_large
----------------

### ¿cuánta memoria se ahorró de este modo? (en KiB)

De este modo cada vez que se use una large page se ahorra el espacio que ocupa una *page table*, que son 1024 entradas de 32 bits cada una, es decir 4KiB. 
Un ejemplo de esto fue eliminó la *entry_pgtable* del Kernel y se la remplazó por el uso de una página grande.

### ¿es una cantidad fija, o depende de la memoria física de la computadora?

Para la arquitectura x86 esto es válido, aunque para otras arquitecturas, por ejemplo de 64 bits podría ser distinto, dado que cambia la cantidad de bits necesarios para guardar una dirección.
