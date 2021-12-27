# Parte 1: caché de bloques

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
