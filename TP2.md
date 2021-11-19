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

