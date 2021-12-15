# IPC bloqueante

En este challenge se realizará una modificación al código existente de JOS para que ambas syscalls relacionadas con el mecanismo IPC sean bloqueantes.

La implementación debería ser consistente con las siguientes interfaces:

- `lib/ipc.c`
    - `int32_t ipc_recv(envid_t *from_env_store, void *pg, int *perm_store);`
    - `void ipc_send(envid_t to_env, uint32_t val, void *pg, int perm);`

## Preguntas

### ¿Se pueden mantener ambas interfaces (tanto para user como para kernel), y solamente cambiar la implementación? ¿Por qué?

Si, se pueden mantener ambas interfaces tanto para user como para kernel. \
Todos los parametros necesarios estan presentes y en modo kernel tenemos todas las herramientas necesarias para implementarlo. \

Algunos de los problemas que podrian surigir y sus soluciones son:
- Que hacer con el mensaje a enviar cuando el receptor no esta escuchando
> El emisor guarda el mensaje en los mismos campos, el receptor los cargara cuando escuche el mensaje. Hasta podemos reutilizar los mismos campos para lectura y escritura, ya que no reciven y envian al mismo tiempo
- Tiene un env permiso para modificar a otro ?
> Estando en modo kernel podemos modificar cualquier cosa

### Como se pregunta en la tarea: sys_ipc_try_send: ¿Cuáles son los posibles deadlocks? ¿Cuáles son esos escenarios, y cómo se pueden solventar?

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

### ¿Cuáles son los cambios que se necesitan hacer en el struct Env?

```
// Structures needed to implement a blocking ipc_send
bool env_ipc_sending;		// Env is blocked sending
envid_t env_ipc_to;			// envid of the receiver
envid_t env_ipc_senders_head;	// Head of the list of senders
envid_t env_ipc_senders_tail;	// Tail of the list of senders
struct Env * env_ipc_senders_next;	// Next sender in the list
```

### ¿Es necesario alguna estructura de datos extra?

Si y No,
Si porque se usa una nueva estructura, una cola de envs esperando para enviar un mensaje al env.
No porque esta cola esta embebida en el array de envs preexistente mediante los nuevos campos de los env 

### Tener en cuenta las responsabilidades de cada syscall, es decir, ¿cómo dividirlas?

- `ipc_send`
> Si el receptor esta escuchando, se encarga de enviarle el mensaje y despertarlo \
> Si no, se encarga de guardar el mensaje, informarle al receptor que esta esperando y bloquearse hasta ser despertado

- `ipc_recv`
> Si hay algun emisor esperando, se encarga de aceptar su mensaje y despertarlo \
> Si no, se encarga de marcarse como que esta escuchando y bloquearse hasta ser despertado

### ¿Qué implementación es más efeciente en cuanto al uso de los recursos del sistema? (por ejemplo, cantidad de context switch realizados)

La nueva es mas eficiente, en la vieja habia un loop que intentaba mandar mensajes hasta conseguirlo. \
Esto causa que el proceso emisor consuma recursos cuando no es necesario, si el proceso no envia durante la duracion de su quantum, se podria haber ahorrado no solo ese quantum si no que tambien el context switch, es mas aunque haya enviado podria haber malgastado tiempo dentro del loop.

### ¿Cuáles fueron las nuevas estructuras de datos utilizadas? ¿Por qué? ¿Qué otras opciones contemplaron?

Usamos cola de envs esperando para enviar un mensaje.
Porque era simple y conserva el orden de llegada.
No contemplamos otras opciones, nos parece que la mejor solucion es FIFO y la cola esta hecha para eso.

### ¿Existe algún escenario o caso de uso en el cual se desee tener una implementación no bloqueante de alguna de las dos syscalls? Ejemplificar, y de ser posible implementar programas de usuario que lo muestren.

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
