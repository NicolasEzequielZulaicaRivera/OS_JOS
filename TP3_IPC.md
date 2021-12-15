# IPC bloqueante

En este challenge se realizará una modificación al código existente de JOS para que ambas syscalls relacionadas con el mecanismo IPC sean bloqueantes.

La implementación debería ser consistente con las siguientes interfaces:

`lib/ipc.c`
    - `int32_t ipc_recv(envid_t *from_env_store, void *pg, int *perm_store);`
    - `void ipc_send(envid_t to_env, uint32_t val, void *pg, int perm);`

## Preguntas

- ¿Se pueden mantener ambas interfaces (tanto para user como para kernel), y solamente cambiar la implementación? ¿Por qué?



- Como se pregunta en la tarea: sys_ipc_try_send: ¿Cuáles son los posibles deadlocks? ¿Cuáles son esos escenarios, y cómo se pueden solventar?

- ¿Cuáles son los cambios que se necesitan hacer en el struct Env?

- ¿Es necesario alguna estructura de datos extra?

- Tener en cuenta las responsabilidades de cada syscall, es decir, ¿cómo dividirlas?

- ¿Qué implementación es más efeciente en cuanto al uso de los recursos del sistema? (por ejemplo, cantidad de context switch realizados)

- ¿Cuáles fueron las nuevas estructuras de datos utilizadas? ¿Por qué? ¿Qué otras opciones contemplaron?

- ¿Existe algún escenario o caso de uso en el cual se desee tener una implementación no bloqueante de alguna de las dos syscalls? Ejemplificar, y de ser posible implementar programas de usuario que lo muestren.