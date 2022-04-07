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

# TP2: Procesos de usuario


## env_alloc


### ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)

` generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1); `

env_id será 0 porque asi fue incializado y las entradas del arreglo envs no habian sido usado antes.

Haciendo la cuenta, generation será **4096** en estos primeros enviroments

` e->env_id = generation | (e - envs); `

Cada llamada a esta función se hara con el enviroment siguiente del arreglo, empezando el primero.

Por esto e - envs dara como resultado:
- 0 en la primera llamada -> env id será **4096**
- 1 en la segunda llamada -> env id será **4097**
- 2 en la tercera llamada -> env id será **4098**
- 3 en la cuarta  llamada -> env id será **4099**
- 4 en la quinta  llamada -> env id será **4050**

> Para los primeros procesos, no se entrará al siguiente if
> ```
> if (generation <= 0)  // Don't create a negative env_id.
> 		generation = 1 << ENVGENSHIFT;
> ```
> ya que para que generation sea negativo, su primer bit tendría que ser 1, para lo cual el
> env_id tendría que ser muy grande.


### Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?

En esta caso e-envs será siempre 630 y lo que cambia es generation:
- 4096  en la primera llamada -> env id será **4726** (4096+630)
- 8192  en la segunda llamada -> env id será **8822**
- 12288 en la tercera llamada -> env id será **12918**
- 16384 en la cuarta  llamada -> env id será **17014**
- 20480 en la quinta  llamada -> env id será **21110**
- 24576 en la sexta   llamada -> env id será **25206**

*Nota: la primera llamada sería la primera vez que se crea enviroment en la posición 630, al crear todos los otros. Las siguientes 5 son aquellas en las cuales se crea y se destruye*

> Aquí tampoco se entrara al if por la misma razón de antes

## env_init_percpu

#### ¿Cuántos bytes escribe la función lgdt, y dónde?

La función `env_init_percpu` hace un llamado a `lgdt` donde escribe 6 entradas de 8 bytes cada una, un total de 48 bytes.
Esto se escribe en la GDT ( Global descriptor table ), la dirección de esta tabla es almacenada en el registro %gdtr del cpu.

#### ¿Qué representan esos bytes?

Cada entrada representa un descriptor de segmento que almacena:
- *Dirección Inicial (Base address)*
- *Limite (Bounds)*
- *Permisos (Access Rights)*
- *Información de uso (Usage Information)*


## env_pop_tf

### La función env_pop_tf() ya implementada es en JOS el último paso de un context switch a modo usuario. Antes de implementar env_run(), responder a las siguientes preguntas:

#### 1. Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:
- el tope de la pila justo antes popal: La dirección del trapframe que recibe por parámetro
- el tope de la pila justo antes iret: El instruction pointer del trapframe recibido por parámetro (tf->tf_eip)
- el tercer elemento de la pila justo antes de iret: Los flags del trapframe recibido por parámetro (tf->tf_eflags)


#### 2. En la documentación de iret en [IA32-2A] se dice:

> If the return is to another privilege level, the IRET instruction also pops the stack pointer and SS from the stack, before resuming program execution.

¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?

El privilegio actual (CPL) ocupa 2 bits que se almacenan al principio de los registros de segmento CS y SS.

La CPU cambia el CPL cuando el control del programa es transferido a un segmento de codigo con un diferente nivel de privilegio.

## gdb_hello

### Se pide mostrar una sesión de GDB con los siguientes pasos:

#### 1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.

```
(gdb) b env_pop_tf
Breakpoint 1 at 0xf0102eab: file kern/env.c, line 510.
(gdb) c
Continuing.
The target architecture is set to "i386".
=> 0xf0102eab <env_pop_tf>:     push   %ebp

Breakpoint 1, env_pop_tf (tf=0xf01b3000) at kern/env.c:510
510     {
(gdb) 
```

#### 2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.

```
(qemu) info registers
EAX=003bc000 EBX=00010094 ECX=f03bc000 EDX=00000229
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0102eab EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
SS =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
```

#### 3. De vuelta a GDB, imprimir el valor del argumento tf:

```
(gdb) p tf
$1 = (struct Trapframe *) 0xf01b3000
```

#### 4. Imprimir, con `x/Nx` tf tantos enteros como haya en el struct Trapframe donde `N = sizeof(Trapframe) / sizeof(int)`.

```
(gdb) print sizeof(struct Trapframe) / sizeof(int)
$2 = 17
(gdb) x/17x tf
0xf01b3000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01b3010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01b3020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01b3030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01b3040:     0x00000023
```

#### 5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:

```
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0102eab <+0>:     push   %ebp
   0xf0102eac <+1>:     mov    %esp,%ebp
   0xf0102eae <+3>:     sub    $0xc,%esp
   0xf0102eb1 <+6>:     mov    0x8(%ebp),%esp
   0xf0102eb4 <+9>:     popa   
   0xf0102eb5 <+10>:    pop    %es
   0xf0102eb6 <+11>:    pop    %ds
   0xf0102eb7 <+12>:    add    $0x8,%esp
   0xf0102eba <+15>:    iret   
   0xf0102ebb <+16>:    push   $0xf0105350
   0xf0102ec0 <+21>:    push   $0x208
   0xf0102ec5 <+26>:    push   $0xf01052ba
   0xf0102eca <+31>:    call   0xf01000a9 <_panic>
End of assembler dump.
(gdb) si 4
=> 0xf0102eb4 <env_pop_tf+9>:   popa   
0xf0102eb4      511             asm volatile("\tmovl %0,%%esp\n"
(gdb) 
```

#### 6. Comprobar, con `x/Nx $sp` que los contenidos son los mismos que tf (donde N es el tamaño de tf).

```
(gdb) x/17x $sp
0xf01b3000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01b3010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01b3020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01b3030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01b3040:     0x00000023
```

#### 7. Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.
  | Pos | Valor | Atributo | Descripcion |
  |:---:|:---:|:---:|:---:|
  |  1 | 0x0  | reg_edi  | registro de uso general |
  |  2 | 0x0  | reg_esi  | registro de uso general |
  |  3 | 0x0  | reg_ebp  | registro de uso general |
  |  4 | 0x0  | reg_oesp | registro de uso general |
  |  5 | 0x0  | reg_ebx  | registro de uso general |
  |  6 | 0x0  | reg_edx  | registro de uso general |
  |  7 | 0x0  | reg_ecx  | registro de uso general |
  |  8 | 0x0  | reg_eax  | registro de uso general |
  |  9 | 0x23 | tf_es    | segment selector for data segment |
  | 10 | 0x23 | tf_ds    | segment selector for data segment |
  | 11 | 0x0  | tf_trapno| trap number |
  | 12 | 0x0  | tf_err   | error code  |
  | 13 | 0x20 | tf_eip   | instruction pointer |
  | 14 | 0x1b | tf_cs    | code segment |
  | 15 | 0x0  | tf_eflags  | flags |
  | 16 |0xeebfe000| tf_esp | stack pointer |
  | 17 | 0x23 | tf_ss      | segment stack |

  - Configuración inicial hecha en env_alloc:
```
e->env_tf.tf_ds = GD_UD | 3;
e->env_tf.tf_es = GD_UD | 3;
e->env_tf.tf_ss = GD_UD | 3;
e->env_tf.tf_esp = USTACKTOP;
e->env_tf.tf_cs = GD_UT | 3;
// You will set e->env_tf.tf_eip later.
```
  - Configuración inicial hecha en load_icode:
```
// Set the environment's instruction pointer to the elf's entry point
e->env_tf.tf_eip = elf->e_entry;
```

#### 8. Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.

```
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01b3030
EIP=f0102eba EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
SS =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
```
Cambios:
   - Los registros se limpiaron 
   - DPL los segmentos de datos pasan de kernel (0) a usuario (3)

#### 9. Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.

  - imprimir el valor del contador de programa con p $pc o p $eip
  - cargar los símbolos de hello con el comando add-symbol-filee
  - volver a imprimir el valor del contador de programa
  - Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

```
=> 0x800020:    cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
(gdb) p $pc
$1 = (void (*)()) 0x800020
(gdb) add-symbol-file obj/user/hello 0x800020
add symbol table from file "obj/user/hello" at
        .text_addr = 0x800020
(y or n) y
Reading symbols from obj/user/hello...
(gdb) p $pc
$2 = (void (*)()) 0x800020 <_start>
(gdb) 
```

```
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
SS =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
```

  - Cambios:
    - Ahora se pueden ver los mismos valores que el tf

#### 10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.

No se encontro `int $0x30`

## kern_idt

### ¿Cómo decidir si usar TRAPHANDLER o TRAPHANDLER_NOEC? ¿Qué pasaría si se usara solamente la primera?

Para saber si usar TRAPHANDLER o TRAPHANDLER_NOEC es necesario concer que interrpuciones utilizan el error code y cuales no, lo que se puede sacar del manual de intel.

Si se usara solamente TRAPHANDLER, dado que no se hace un push para rellenar cuando se trate de instrucciones que no requieren error code, el struct TrapFrame, que es el mismo para ambos casos, quedaria con basura.

### ¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro SETGATE? ¿Por qué se elegiría un comportamiento u otro durante un syscall?

Istrap se encarga de determinar si se permiten las interrupciones anidadas o si no se permiten. 

Cuando no se permiten, si llega una interrupción mientras se está manejando otra se encola la nueva. Esto es lo que hace Jos por simplicidad.

Permitir las interrupciones anidadas presenta la ventaja de que es más flexible ya que permite priorizar cual ejecutar primero, como pasa en Linux.

### Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué interrupción trata de generar? ¿Qué interrupción se genera? Si son diferentes a la que invoca el programa… ¿cuál es el mecanismo por el que ocurrió esto, y por qué motivos? ¿Qué modificarían en JOS para cambiar este comportamiento?

Intenta un PAGE_FAULT, pero genera un GENERAL_PROTECTION_FAULT.

Esto ocurre porque cuando definen los privilegios de la interrupcion en trap_init, se establece que el nivel de privilegio necesario para ejecutar la iterrupción PAGE_FAULT es el de Kernel. Al intentar ejecutarla desde un programa de usuario, no se permite lanzar la interrupcion y se aborta el programa con un GENERAL_PROTECTION_FAULT.

Para que el usuario pueda lanzar un PAGE_FAULT directamente, se tendria que cambiar el nivel de privilegio necesario para esto a de 0 a 3.

## user_evilhello
```
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
    char *entry = (char *) 0xf010000c;
    char first = *entry;
    sys_cputs(&first, 1);
}
```

- ¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba?

  - **original:** llama a `sys_cputs` con la dirección **0xf010000c**
  
  - **arriba:** se trata de acceder al valor apuntado desreferenciando un puntero local y asignándoselo a una variable local, y luego se le pasa la posición de esa variable a `sys_cputs` 

- ¿En qué cambia el comportamiento durante la ejecución? ¿Por qué? ¿Cuál es el mecanismo?
  
  - **original:**  falla `user_mem_check` en `sys_cputs`, que chequea que el usuario tenga acceso a la memoria que quiere imprimir , y se destrulle el env
    > ![image](https://user-images.githubusercontent.com/66538092/142731458-ec5ae44d-4dfd-4952-8529-6aa6be92ec73.png)

  - **arriba:** se lanza un page fault cuando se trata de acceder al valor de la posición ya que el usuario no tiene permiso de acceso
    > ![image](https://user-images.githubusercontent.com/66538092/142731462-105ef427-6f1e-4cea-ab05-0d641b7ae2e7.png)

- Listar las direcciones de memoria que se acceden en ambos casos, y en qué ring se realizan. ¿Es esto un problema? ¿Por qué?

  - **original:** se quiere acceder a `0xf010000c` desde el ring 0 mediante `sys_cputs`.
  Para evitar accesos indebidos la funcion comprueba los permisos y destruye el env malicioso.
  
  - **arriba:** se accede a `0xf010000c` desde el ring 3. es un problema porque el usuario no tiene permiso.

> **0xf010000c**: Punto de entrada del kernel, el usuario no tiene permiso de acceso.

# TP3

## Parte 0: Múltiples CPUs

## Parte 1: Planificador y múltiples procesos

### env_return

#### al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

Al terminar `umain` continuará con la siguiente instrucción de `libmain` (donde fue llamada `umain`) que llamara a la funcion `exit`, que llamara a `sys_env_destroy(0)` donde el kernel retoma la ejecución.

Luego, las secuencia de llamadas sera:
```
0xf01041e5 in trap (tf=0x0) at kern/trap.c:328
0xf010414b in trap_dispatch (tf=0xf02cd000) at kern/trap.c:242
0xf01223c0 in ?? ()
0xf0104b11 in syscall (syscallno=4027720640, a1=4026531744, a2=4027597131, a3=3, a4=0, a5=0) at kern/syscall.c:429
0xf010469f in sys_env_destroy (envid=30) at kern/syscall.c:63
env_destroy (e=0xf02cd000) at kern/env.c:508
```

#### ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?

**Viejo:**
```
void
env_destroy(struct Env *e)
{
	env_free(e);

	cprintf("Destroyed the only environment - nothing more to do!\n");
	while (1)
		monitor(NULL);
}
```

**Nuevo:**
```
void
env_destroy(struct Env *e)
{
	// If e is currently running on other CPUs, we change its state to
	// ENV_DYING. A zombie environment will be freed the next time
	// it traps to the kernel.
	if (e->env_status == ENV_RUNNING && curenv != e) {
		e->env_status = ENV_DYING;
		return;
	}

	env_free(e);

	if (curenv == e) {
		curenv = NULL;
		sched_yield();
	}
}
```

La diferencia es que en el TP anterior habia un solo proceso, por lo que cuando se termina el proceso, el kernel se queda 'colgado'

En cambio, en este TP tenemos multiples procesos y CPUS, por lo que cuando se termina un proceso, el kernel:
    - 1. verifica si el proceso que termino se esta corriendo en otro CPU, en ese caso, lo setea como `ENV_DYING` y retorna
    - 2. Libera el proceso
    - 3. Si el proceso es el actual, invoca `sched_yield()` para que el kernel se mueva a otro proceso.

### sys_yield

#### Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.

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

Se puede ver que cada vez que un proceso llama a `sys_yield` el scheduler pone a correr al proceso siguiente por lo que los prints se encuentran en intercalados.

## Parte 2: Creación dinámica de procesos

### envid2env

#### ¿Qué ocurre en JOS si un proceso llama a sys_env_destroy(0)?.

Lo que ocurre es que el kernel destruye el proceso que llamo a la funcion.

Un ejemplo se puede encontar en la funcion `exit` de lib/exit.c.

### dumbfork

#### Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?

Si, no se preservan los flags, dado que se fuerzan los permisos `PTE_P|PTE_U|PTE_W` al duplicar las paginas.

#### Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly que indicase si la página es modificable o no:

```
envid_t dumbfork(void) {
    // ...
    for (addr = UTEXT; addr < end; addr += PGSIZE) {
        bool readonly;
        readonly = !(
            (uvpd[PGNUM(va)] & PTE_W) && 
            (uvpt[PGNUM(va)] & PTE_W) 
        );
        duppage(envid, addr, readonly);
    }
    // ...
```
Ayuda: usar las variables globales uvpd y/o uvpt.

#### Supongamos que se desea actualizar el código de duppage() para tener en cuenta el argumento readonly: si este es verdadero, la página copiada no debe ser modificable en el hijo. Es fácil hacerlo realizando una última llamada a sys_page_map() para eliminar el flag PTE_W en el hijo, cuando corresponda:

```
void duppage(envid_t dstenv, void *addr, bool readonly) {
    // Código original (simplificado): tres llamadas al sistema.
    perm = PTE_P | PTE_U | PTE_W;
    if (readonly) perm &= ~PTE_W;

    sys_page_alloc(dstenv, addr, perm);
    sys_page_map(dstenv, addr, 0, UTEMP, perm);

    memmove(UTEMP, addr, PGSIZE);
    sys_page_unmap(0, UTEMP);
}
```

## Parte 3: Ejecución en paralelo (multi-core)

### multicore_init

#### ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?

`memmove(code, mpentry_start, mpentry_end - mpentry_start);`

Esa línea copia el código del archivo kern/mpentry.S a MPENTRY_PADDR. 
Dicho código se encarga de activar el modo protegido de 32 bits.
La dirección MPENTRY_PADDR cumple el requisito necesario para guardar instrucciones que se corran el modo real, que estar en los 2^16 bytes más bajos de la memoria física.


#### ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?

La variable mpentry_kstack es seteada por init.c  para indicarle a mpentry.S donde poner el stack pointer del CPU correspondiente.

Al haber muchos CPUs, cada uno tiene su stack del Kernel correspondiente, dado que distintos CPUs podrían querer acceder al Kernel a la vez.
Dado que kern/mpentry.S es el código que corre cada CPU al iniciar, si aquí se reservará espacio para el stack de manera similar a bootstack en el archivo kern/entry.S los stacks de los distintos CPUs se pisarían y sería imposible que cada uno tenga el suyo separado.


#### En el archivo kern/mpentry.S se puede leer:

```
 # We cannot use kern_pgdir yet because we are still
 # running at a low EIP.
 movl $(RELOC(entry_pgdir)), %eax
```
- ¿Qué valor tendrá el registro %eip cuando se ejecute esa línea? Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.

Redondeando a 12 bits, el valor del %eip al ejecutar esa línea será 0x7000. Esto ocurre porque el archivo kern/mpentry.S arranca corriendo en modo real, por lo que necesita estar en direcciones bajas.

## Parte 4: Comunicación entre procesos

### ipc_recv

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

### ipc_try_send

#### cómo se podría implementar una función sys_ipc_send() (con los mismos parámetros que sys_ipc_try_send()) que sea bloqueante



- qué cambios se necesitan en struct Env para la implementación (campos nuevos, y su tipo; campos cambiados, o eliminados, si los hay)

    - Un `bool` para indicar si el env está bloqueado enviando.
    - Un `envid_t` para indicar a que env envía. ( puede usarse tambien como flag para indicar si el env está bloqueado enviando)
    - Una lista/cola de envs esperando para enviar. ( puede implementarse si los envs almacenan una referencia al siguiente en la lista, se puede optimizar la insercion guardando una referencia al ultimo en la lista)

- qué asignaciones de campos se harían en sys_ipc_send()

Primero se probaria un try_send, si eso fallara porque el env no esta esperando, entoces:
    - se agreaga el env a la lista de espera del reciever
    - se setea el id del reciever en el sender
    - se asigna el sender como `ENV_NOT_RUNNABLE`

- qué código se añadiría en sys_ipc_recv()

Antes de setearse como recieving, chequea si hay algun proceso esperando para enviar.
Si lo hay, entonces lo despierta y lo saca de la lista de espera.

- ¿existe posibilidad de deadlock?

Si, para evitarlo, se puede:
    - Lanzar error si el env al que se quiere enviar esta bloqueado enviando. 
    - Recorrer a quien esperan enviar los sucesivos enviroments y fallar si se generara una dependencia circular.

- ¿funciona que varios procesos (A₁, A₂, …) puedan enviar a B, y quedar cada uno bloqueado mientras B no consuma su mensaje? ¿en qué orden despertarían?
    - Si, despertarian en orden de llegada.

- Otras cosideraciones
    - Habria que avisarle a la lista de senders si el reciever es eliminado. ( en `env_destroy` )
## Desafio: IPC bloqueante

En este challenge se realizará una modificación al código existente de JOS para que ambas syscalls relacionadas con el mecanismo IPC sean bloqueantes.

La implementación debería ser consistente con las siguientes interfaces:

- `lib/ipc.c`
    - `int32_t ipc_recv(envid_t *from_env_store, void *pg, int *perm_store);`
    - `void ipc_send(envid_t to_env, uint32_t val, void *pg, int perm);`

### Preguntas

#### ¿Se pueden mantener ambas interfaces (tanto para user como para kernel), y solamente cambiar la implementación? ¿Por qué?

Si, se pueden mantener ambas interfaces tanto para user como para kernel. \
Todos los parametros necesarios estan presentes y en modo kernel tenemos todas las herramientas necesarias para implementarlo. \

Algunos de los problemas que podrian surigir y sus soluciones son:
- Que hacer con el mensaje a enviar cuando el receptor no esta escuchando
> El emisor guarda el mensaje en los mismos campos, el receptor los cargara cuando escuche el mensaje. Hasta podemos reutilizar los mismos campos para lectura y escritura, ya que no reciven y envian al mismo tiempo
- Tiene un env permiso para modificar a otro ?
> Estando en modo kernel podemos modificar cualquier cosa

#### Como se pregunta en la tarea: sys_ipc_try_send: ¿Cuáles son los posibles deadlocks? ¿Cuáles son esos escenarios, y cómo se pueden solventar?

Habiendo agregado las funcionalidades de Enviar y Recibir mensajes puede haber 2 origenes para un deadlock:

- Un env se bloquea escuchando pero nadie puede/podra enviar
> Esto se puede dar si el resto de los envs esta bloqueado de modo que no se puede desbloquear sin accion de otro env

- Un env se bloquea enviando pero nadie puede/podra escuchar
> Esto se puede dar si el env receptor se bloquea (de modo que no se puede desbloquear) o es eliminado

Un caso particular de deadlock es cuando dos envs tratan de enviarse mensajes entre si (o forman una cadena) donde se bloquean de forma circular.

**Souciones:**
- Si todos los procesos estan bloqueados, forzar el desbloqueo de alguno/s ( podria desbloquearse el primero en el scheduler o se podria liberar un grupo, incluso todos ). En estos casos la funcion deberia retornar como fallida.
- Si un proceso es eliminado deberian fallar los intentos de escritura al mismo.
- Al enviar un mensaje a un proceso que esta bloqueado enviando
    - Retornar error directamente
    - Recorrer la cadena de envios para comprobar si se forma una cadena, en cuyo caso, se retorna error

#### ¿Cuáles son los cambios que se necesitan hacer en el struct Env?

```
// Structures needed to implement a blocking ipc_send
bool env_ipc_sending;		// Env is blocked sending
envid_t env_ipc_to;			// envid of the receiver
envid_t env_ipc_senders_head;	// Head of the list of senders
envid_t env_ipc_senders_tail;	// Tail of the list of senders
struct Env * env_ipc_senders_next;	// Next sender in the list
```

#### ¿Es necesario alguna estructura de datos extra?

Si y No,
Si porque se usa una nueva estructura, una cola de envs esperando para enviar un mensaje al env.
No porque esta cola esta embebida en el array de envs preexistente mediante los nuevos campos de los env 

#### Tener en cuenta las responsabilidades de cada syscall, es decir, ¿cómo dividirlas?

- `ipc_send`
> Si el receptor esta escuchando, se encarga de enviarle el mensaje y despertarlo \
> Si no, se encarga de guardar el mensaje, informarle al receptor que esta esperando y bloquearse hasta ser despertado

- `ipc_recv`
> Si hay algun emisor esperando, se encarga de aceptar su mensaje y despertarlo \
> Si no, se encarga de marcarse como que esta escuchando y bloquearse hasta ser despertado

#### ¿Qué implementación es más efeciente en cuanto al uso de los recursos del sistema? (por ejemplo, cantidad de context switch realizados)

La nueva es mas eficiente, en la vieja habia un loop que intentaba mandar mensajes hasta conseguirlo. \
Esto causa que el proceso emisor consuma recursos cuando no es necesario, si el proceso no envia durante la duracion de su quantum, se podria haber ahorrado no solo ese quantum si no que tambien el context switch, es mas aunque haya enviado podria haber malgastado tiempo dentro del loop.

#### ¿Cuáles fueron las nuevas estructuras de datos utilizadas? ¿Por qué? ¿Qué otras opciones contemplaron?

Usamos cola de envs esperando para enviar un mensaje.
Porque era simple y conserva el orden de llegada.
No contemplamos otras opciones, nos parece que la mejor solucion es FIFO y la cola esta hecha para eso.

#### ¿Existe algún escenario o caso de uso en el cual se desee tener una implementación no bloqueante de alguna de las dos syscalls? Ejemplificar, y de ser posible implementar programas de usuario que lo muestren.

##### `send` no bloqueante

Podriamos querer utilizarlo si nos podemos permitir perder mensajes o no nos podemos permitir quedarnos bloqueados.

Por Ejemplo:

Tenemos un juego, calcular el estado es rapido pero el renderizado es lento y no queremos que afecte el input de datos. \
nos podemos permitir perder un par de frames, por lo que el renderizado se realiza en un proceso aparte \
De este modo siempre se acepta el input y se mantiene un estado cercano al ultimo.

pseudocodigo:

`main env`
```
render(state){
    // Derivates task to renderer env
    trySend( renderer, state )
}
start(){
    state = initialState
    loop {
        input = getInput()
        state = getNextState( state, input )
        render( state )
    }
}
```
`renderer env`
```
render(state){
    // Actualy renders on screen
}
renderLoop(){
    loop {
        state = recv()
        render(state)
    }
}
```

##### `recv` no bloqueante

Podriamos querer utilizarlo si queremos recibir mensajes y hacer calculos en un mismo env, \
donde nos conviene hacer calculos en vez estar bloqueado esperando mensajes, \
y luego de realizar cierta cantidad de calculos podemos leer todos los mensajes encolados.

Por Ejemplo:


pseudocodigo:

`main env`
```
main(){
    
    queue : []
    state = initialState
    
    loop{

        while( payload = tryRecv() ){
            queue.push( payload )
        }
        
        if( queue.isEmpty ) data = recv()
        else data = queue.pop()
        
        result = processData( data )
        
        state = updateState( state, result )
        
        log( state )
        
    }

}
```
# TP4

## Parte 1: caché de bloques

### Lecturas y escrituras

- Se recomienda leer la función diskaddr() en el archivo fs/bc.c. Responder:
    - ¿Qué es super->s_nblocks?
    >  Representa la cantidad de bloques en total en el disco
    - ¿Dónde y cómo se configura este bloque especial?
    > Se configura en la funcion opendisk del archivo  fs/fsformat.c
    > ```
    > super->s_magic = FS_MAGIC; // Asinga el numero magico
    > super->s_nblocks = nblocks; // Asigna la cantidad de bloques
    > super->s_root.f_type = FTYPE_DIR; // Asigna el tipo del nodo del directorio raiz 
    > strcpy(super->s_root.f_name, "/"); // Asigna el nombre del nodo del directorio raiz
    > ```
