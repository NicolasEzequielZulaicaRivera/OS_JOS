# Parte 1: caché de bloques

### Lecturas y escrituras

- Se recomienda leer la función diskaddr() en el archivo fs/bc.c. Responder:
    - ¿Qué es super->s_nblocks?
    > super->s_nblocks es el número de bloques del superbloque.
    - ¿Dónde y cómo se configura este bloque especial?
    > El bloque especial 0 es el superbloque.