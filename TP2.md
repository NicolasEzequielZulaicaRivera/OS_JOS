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

*Nota: la primera llamada seria la primera vez que se crea enviroment en la posicion 630, al crear todos los otros.*
* las siguientes 5 son aquellas en las cuales se crea y se destruye *

> Aquí tampoco se entrara al if por la misma razón de antes

## env_init_percpu



## env_pop_tf



## gdb_hello


