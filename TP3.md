# Parte 0: Múltiples CPUs

# Parte 1: Planificador y múltiples procesos

## env_return

### al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

### ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?

## sys_yield

### Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.

```
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000.
Hello, I am environment 00001001.
Hello, I am environment 00001002.
Back in environment 00001000, iteration 0.
Back in environment 00001001, iteration 0.
Back in environment 00001002, iteration 0.
Back in environment 00001000, iteration 1.
Back in environment 00001001, iteration 1.
Back in environment 00001002, iteration 1.
Back in environment 00001000, iteration 2.
Back in environment 00001001, iteration 2.
Back in environment 00001002, iteration 2.
Back in environment 00001000, iteration 3.
Back in environment 00001001, iteration 3.
Back in environment 00001002, iteration 3.
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
```

# Parte 2: Creación dinámica de procesos

## envid2env

### ¿Qué ocurre en JOS si un proceso llama a sys_env_destroy(0)?.

## dumbfork

### Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?

### Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly que indicase si la página es modificable o no:

```
envid_t dumbfork(void) {
    // ...
    for (addr = UTEXT; addr < end; addr += PGSIZE) {
        bool readonly;
        //
        // TAREA: dar valor a la variable readonly
        //
        duppage(envid, addr, readonly);
    }
    // ...
```
Ayuda: usar las variables globales uvpd y/o uvpt.

### Supongamos que se desea actualizar el código de duppage() para tener en cuenta el argumento readonly: si este es verdadero, la página copiada no debe ser modificable en el hijo. Es fácil hacerlo realizando una última llamada a sys_page_map() para eliminar el flag PTE_W en el hijo, cuando corresponda:

```
void duppage(envid_t dstenv, void *addr, bool readonly) {
    // Código original (simplificado): tres llamadas al sistema.
    sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
    sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W);

    memmove(UTEMP, addr, PGSIZE);
    sys_page_unmap(0, UTEMP);

    // Código nuevo: una llamada al sistema adicional para solo-lectura.
    if (readonly) {
        sys_page_map(dstenv, addr, dstenv, addr, PTE_P | PTE_U);
    }
}
```

- Esta versión del código, no obstante, incrementa las llamadas al sistema que realiza duppage() de tres, a cuatro. Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly, pero sin usar en ningún caso más de tres llamadas al sistema.

Ayuda: Leer con atención la documentación de sys_page_map() en kern/syscall.c, en particular donde avisa que se devuelve error:
> if (perm & PTE_W) is not zero, but srcva is read-only in srcenvid’s address space.

# Parte 3: Ejecución en paralelo (multi-core)

## multicore_init

### ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?

`memmove(code, mpentry_start, mpentry_end - mpentry_start);`

### ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?

### En el archivo kern/mpentry.S se puede leer:

```
 # We cannot use kern_pgdir yet because we are still
 # running at a low EIP.
 movl $(RELOC(entry_pgdir)), %eax
```
- ¿Qué valor tendrá el registro %eip cuando se ejecute esa línea? Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.

# Parte 4: Comunicación entre procesos

## ipc_recv

- Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no?

```
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (/* ??? */)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```

Se puede distinguir ya que`src` sera el envid del proceso que envía el mensaje si es correcto o 0 si hay error.

# Parte 5: Manejo de page faults

# Parte 6: Copy-on-write fork

---

- [ ] Parte 1
    - [ ] env_return
    - [ ] sys_yield
- [ ] Parte 2
    - [ ] envid2env
    - [ ] dumbfork
- [ ] Parte 3
    - [ ] multicore_init
    - [ ] trap_init_percpu
- [ ] Parte 4
    - [ ] ipc_recv
    - [ ] sys_ipc_try_send
- [ ] Parte 5
    - [ ] page_fault_handler*
    - [ ] page_fault_handler_upcall*
- [ ] Parte 6