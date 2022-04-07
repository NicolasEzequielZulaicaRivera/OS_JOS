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
