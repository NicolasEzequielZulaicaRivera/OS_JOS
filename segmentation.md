# Segmentación

##  Simulación de traducciones


### Primera corrida

```
ARG seed 104030
ARG address space size 64
ARG phys mem size 256

Segment register information:

Segment 0 base  (grows positive) : 0x000000b4 (decimal 180)
Segment 0 limit                  : 23

Segment 1 base  (grows negative) : 0x00000060 (decimal 96)
Segment 1 limit                  : 16

Virtual Address Trace
VA  0: 0x00000004 (decimal:    4) --> PA or segmentation violation?
VA  1: 0x0000003a (decimal:   58) --> PA or segmentation violation?
```

Primero paso ambas direcciones a binario:
- **VA 0**: 0 00100 -> Bit más significativo en 0 -> Segmento 0
- **VA 1**: 1 11010 -> Bit más significativo en 1 -> Segmento 1

Ahora traduzco las direcciones:
- **VA 0** es válida porque la parte que representa el offset es menor al límite. Por lo tanto, la traduzco sumándole la base, por lo que queda VA 0 -> 0x000000b8 (decimal 184)
- **VA 1** es válida, dado que los bits que representan el offset, que en decimal serian el número 26, representan una dirección más alta que la del límite del segmento. Para traducir la dirección, primero cálculo cuanto le "falta" al 26 para llegar a 32 (tamaño del address space del segmento), que me da 6. Este 6 será lo que debo restar de la base, por lo que la dirección traducida me queda VA 1: 0x0000005a (decimal 90)

### Segunda corrida

```
ARG seed 105774
ARG address space size 64
ARG phys mem size 256

Segment register information:

Segment 0 base  (grows positive) : 0x000000bd (decimal 189)
Segment 0 limit                  : 26

Segment 1 base  (grows negative) : 0x00000067 (decimal 103)
Segment 1 limit                  : 21

Virtual Address Trace
VA  0: 0x00000027 (decimal:   39) --> PA or segmentation violation?
VA  1: 0x0000000d (decimal:   13) --> PA or segmentation violation?
```

Primero paso ambas direcciones a binario:
- **VA 0**: 1 00111 -> Bit más significativo en 1 -> Segmento 1
- **VA 1**: 0 01101 -> Bit más significativo en 0 -> Segmento 0

Ahora traduzco las direcciones:
- **VA 0** es inválida, dado que el offset es 7, que es más bajo que el límite del segmento 
- **VA 1** es válida, por ser el offset 13, lo que está dentro del segmento. Para traducirla simplemente le sumo la base, por lo que queda VA 1: 0x000000ca (decimal 202)



## Traducciones inversas

### Primera corrida

Los resultados que se buscan obtener son:
- VA0: 4  -> PA0: Segmentation Fault
- VA1: 58 -> PA1: 202

Para que la VA0, que sería del segmento 1 por tener el bit más significativo en 0, dé Segmentation Fault, entonces 4 tendría que ser mayor al límite. Para hacer los límites lo más chicos que se pueda se eligen 0 como base y 1 como límite.

La VA1 estará en el segmento 1, por tener el bit más significativo en 1. Restándole su offset (26) al tamaño del address space para ese segmento (32), queda que hay que moverse 6 posiciones desde la base. Por esta razón para que vaya a la PA que debería ir elegimos una base de 208. Además ponemos un límite de 6 para que sea lo más chico posible.

```
python segmentation.py -a 64 -p 256 -s 104030 -A 4,58 -b 0 -l 1 -c -B 208 -L 6
ARG seed 104030
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
  Segment 0 limit                  : 1

  Segment 1 base  (grows negative) : 0x000000d0 (decimal 208)
  Segment 1 limit                  : 6

Virtual Address Trace
  VA  0: 0x00000004 (decimal:    4) --> SEGMENTATION VIOLATION (SEG0)
  VA  1: 0x0000003a (decimal:   58) --> VALID in SEG1: 0x000000ca (decimal:  202)
```




### Segunda corrida

Ahora se buscan los resultados:
- VA0: 39 -> PA0: 184
- VA1: 13 -> PA1: 90

La VA0 corresponde al segmento 1 y tiene un offset de 7. Reptiendo las cuentas hechas para la corrida anterior, llegamos a que es necesaria una base de 209 y un límite de 25 (32 - 7 = 25 y 184 + 25 = 209).

La VA1 pertenece al segmento 0 y su offset es 13. Para que la dirección física correspondiente sea la 90, la base tiene que ser un numero al que sumarle 13 de 90, es decir 77. Para hacer el límite lo más chico posible, se elige un límite de 14.

```
python segmentation.py -a 64 -p 256 -s 105774 -A 39,13 -B 209 -L 25 -b 77 -l 14 -c
ARG seed 105774
ARG address space size 64
ARG phys mem size 256

Segment register information:

Segment 0 base  (grows positive) : 0x0000004d (decimal 77)
Segment 0 limit                  : 14

Segment 1 base  (grows negative) : 0x000000d1 (decimal 209)
Segment 1 limit                  : 25

Virtual Address Trace
VA  0: 0x00000027 (decimal:   39) --> VALID in SEG1: 0x000000b8 (decimal:  184)
VA  1: 0x0000000d (decimal:   13) --> VALID in SEG0: 0x0000005a (decimal:   90)
```


## Límites de segmentación

Utilizando un espacio de direcciones virtuales de 5-bits; y un espacio de direcciones físicas de 7-bits. Responder:

### ¿Cuál es el tamaño (en número de direcciones) de cada espacio (físico y virtual)?

El espacio virtual tendrá un tamaño de 32 direcciones (2^5) y el fisíco tendrá uno de 128 (2^7).

### ¿Es posible configurar la simulación de segmentación para que dos direcciones virtuales se traduzcan en la misma dirección física? Explicar, y de ser posible brindar un ejemplo de corrida de la simulación.

Esto es posible, dado que el hecho de que una dirección fisíca pertenezca a un segmento de memoria no implica que no pueda pertencer también a otro.

```
python segmentation.py -a 32 -p 128 -c -b 0 -l 1 -A 0,31 -B 1
ARG seed 0
ARG address space size 32
ARG phys mem size 128

Segment register information:

Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
Segment 0 limit                  : 1

Segment 1 base  (grows negative) : 0x00000001 (decimal 1)
Segment 1 limit                  : 14

Virtual Address Trace
VA  0: 0x00000000 (decimal:    0) --> VALID in SEG0: 0x00000000 (decimal:    0)
VA  1: 0x0000001f (decimal:   31) --> VALID in SEG1: 0x00000000 (decimal:    0)
```

### ¿Es posible que (aproximadamente) el 90% del espacio de direcciones virtuales esté mapeado de forma válida? Explicar, y de ser posible, dar una configuración de segmentos que de tal resultado.

Si, de hecho es posible tener el 100% de las direcciones virtuales mapeadas de forma válida, por ejemplo con la configuración:
- Base 0: 0
- Base 1: 32
- Limite 0: 16
- Limite 1: 16 

Con esta configuración la traducción de cada dirección virtual es ella misma.

Hacer esto es posible dado que las cantidades direcciones y la cantidad de segmentos lo permiten. Si hubiera, por ejemplo 128 direcciones virtuales, 2 direcciones físicas y un segmento esto no se podría, ya que no habria forma de traducir el 90% de ellas de manera valida.

### ¿Es posible que (aproximadamente) el 90% del espacio de direcciones físicas esté mapeado de forma válida? Explicar, y de ser posible, dar una configuración de segmentos que de tal resultado.

No es posible, dado hay 128 físicas y 32 virtuales, por lo que por más que cada dirección virtual se traduzca a una dirección física distinta, solo se aprovechara el 25% de las direcciones físicas.






