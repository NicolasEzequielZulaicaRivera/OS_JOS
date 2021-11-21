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

*Nota: la primera llamada seria la primera vez que se crea enviroment en la posicion 630, al crear todos los otros. las siguientes 5 son aquellas en las cuales se crea y se destruye*

> Aquí tampoco se entrara al if por la misma razón de antes

## env_init_percpu

#### ¿Cuántos bytes escribe la función lgdt, y dónde?

La funcion `env_init_percpu` hace un llamado a `lgdt` donde escribe 6 entradas de 8 bytes cada una, un total de 48 bytes.
Esto se escribe en la GDT ( Global descriptor table ), la direccion de esta tabla es almacenada en el registro %gdtr del cpu.

#### ¿Qué representan esos bytes?

Cada entrada representa un descriptor de segmento que almacena:
- *Direccion Inicial (Base address)*
- *Limite (Bounds)*
- *Permisos (Access Rights)*
- *Informacion de uso (Usage Information)*


## env_pop_tf

### La función env_pop_tf() ya implementada es en JOS el último paso de un context switch a modo usuario. Antes de implementar env_run(), responder a las siguientes preguntas:

#### 1. Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:
- el tope de la pila justo antes popal: La direccion del trapframe que recibe por parametro
- el tope de la pila justo antes iret: El instruction pointer del trapframe recibido por parametro (tf->tf_eip)
- el tercer elemento de la pila justo antes de iret: Los flags del trapframe recibido por parametro (tf->tf_eflags)


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

  - env_alloc:
```
e->env_tf.tf_ds = GD_UD | 3;
e->env_tf.tf_es = GD_UD | 3;
e->env_tf.tf_ss = GD_UD | 3;
e->env_tf.tf_esp = USTACKTOP;
e->env_tf.tf_cs = GD_UT | 3;
// You will set e->env_tf.tf_eip later.
```
  - load_icode:
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
  - Cambios
   - Los registros se limpiaron 
   - DPL del segmento de datos pasa de kernel (0) a usuario (3)

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
    - Mismos valores que el tf

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
