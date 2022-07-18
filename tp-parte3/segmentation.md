# Parte 1

## Semilla 99139
## Corrida
ARG seed 99139
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000009c (decimal 156)
  Segment 0 limit                  : 21

  Segment 1 base  (grows negative) : 0x000000fb (decimal 251)
  Segment 1 limit                  : 25

Virtual Address Trace
  VA  0: 0x00000032 (decimal:   50) --> PA or segmentation violation?
  VA  1: 0x00000028 (decimal:   40) --> PA or segmentation violation?

For each virtual address, either write down the physical address it translates to
OR write down that it is an out-of-bounds address (a segmentation violation). For
this problem, you should assume a simple address space with two segments: the top
bit of the virtual address can thus be used to check whether the virtual address
is in segment 0 (topbit=0) or segment 1 (topbit=1). Note that the base/limit pairs
given to you grow in different directions, depending on the segment, i.e., segment 0
grows in the positive direction, whereas segment 1 in the negative. 

## Analisis
Tamaño de memoria virtual = 64
Tamaño de memoria fisica = 256
La memoria virtual como bien se menciona es de 64 bytes, tenemos entonces que las direcciones son de 6 bits. Tenemos 5 bits para 
offset dentro de un segmento. 

El segmento 0 tiene un limite de 21. Las direcciones virtual mas bajas [0, 1 , ... , 20] estan mapeadas en las direcciones fisicas
[156, 157, ... , 176]. El segmento 1 tiene un limite de 25. Las direcciones virtual mas altas [38 , 39 , ... , 63] estan mapeadas en las direcciones fisicas
[225, 225, ... , 250]. 

Tenemos dos direcciones virtuales para la seed 102071.
VA  0: 0x00000032 -> 110010 (binario) -> esta en el segmento 1
(110010)b = 50 -> complemento a dos = 14. Entonces 251 (base) - 14 (offset invertido) = 237

Tenemos un acesso a la PA = 235


VA  1: 0x00000028 -> 101000 (binario) -> esta en el segmento 1
(101000)b = 40 -> complemento a dos = 24. Entonces 251 (base) - 24 (offset invertido) = 227

Tenemos un acesso a la PA = 225

Vemos la salida del simulador con el flag -c y validamos que los valores mencionados antes estan correctos

## Corrida verificacion
ARG seed 99139
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000009c (decimal 156)
  Segment 0 limit                  : 21

  Segment 1 base  (grows negative) : 0x000000fb (decimal 251)
  Segment 1 limit                  : 25

Virtual Address Trace
  VA  0: 0x00000032 (decimal:   50) --> VALID in SEG1: 0x000000ed (decimal:  237)
  VA  1: 0x00000028 (decimal:   40) --> VALID in SEG1: 0x000000e3 (decimal:  227)

## Semilla 102071

## Corrida
ARG seed 102071
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000003e (decimal 62)
  Segment 0 limit                  : 19

  Segment 1 base  (grows negative) : 0x00000087 (decimal 135)
  Segment 1 limit                  : 30

Virtual Address Trace
  VA  0: 0x0000001b (decimal:   27) --> PA or segmentation violation?
  VA  1: 0x00000024 (decimal:   36) --> PA or segmentation violation?

For each virtual address, either write down the physical address it translates to
OR write down that it is an out-of-bounds address (a segmentation violation). For
this problem, you should assume a simple address space with two segments: the top
bit of the virtual address can thus be used to check whether the virtual address
is in segment 0 (topbit=0) or segment 1 (topbit=1). Note that the base/limit pairs
given to you grow in different directions, depending on the segment, i.e., segment 0
grows in the positive direction, whereas segment 1 in the negative. 

## Analisis

Tamaño de memoria virtual = 64
Tamaño de memoria fisica = 256
La memoria virtual como bien se menciona es de 64 bytes, tenemos entonces que las direcciones son de 6 bits. Tenemos 5 bits para 
offset dentro de un segmento. 

El segmento 0 tiene un limite de 19. Las direcciones virtual mas bajas [0, 1 , ... , 18] estan mapeadas en las direcciones fisicas
[62, 63, ... , 80]. El segmento 1 tiene un limite de 30. Las direcciones virtual mas altas [23 , 24 , ... , 63] estan mapeadas en las direcciones fisicas
[104, 105, ... , 134]. 

Tenemos dos direcciones virtuales para la seed 102071.
VA  0: 0x0000001b -> 011011 (binario) -> esta en el segmento 0
(11011)b = 27

Tenemos un segmentation fault. Para el segmento 0 el valor de offset maximo es 19.

VA  1: 0x00000024 -> 100100 (binario) -> esta en el segmento 1
(100100)b = 36 -> complemento a dos = 28. Entonces 135 (base) - 28 (offset invertido) = 107

Tenemos un acesso a la PA = 107

Vemos la salida del simulador con el flag -c y validamos que los valores mencionados antes estan correctos

## Corrida verificacion
ARG seed 102071
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000003e (decimal 62)
  Segment 0 limit                  : 19

  Segment 1 base  (grows negative) : 0x00000087 (decimal 135)
  Segment 1 limit                  : 30

Virtual Address Trace
  VA  0: 0x0000001b (decimal:   27) --> SEGMENTATION VIOLATION (SEG0)
  VA  1: 0x00000024 (decimal:   36) --> VALID in SEG1: 0x0000006b (decimal:  107)

# Parte 2

Para esta seccion queremos que las simulaciones 
con la semilla 99139 tengan la salida 
  VA  0: 0x00000032 (decimal:   50) --> SEGMENTATION VIOLATION 
  VA  1: 0x00000028 (decimal:   40) --> VALID in SEG1: 0x0000006b (decimal:  107) (ignoramos el SEG1)


con la semilla 102071 tengan la salida
  VA  0: 0x0000001b (decimal:   27) --> VALID in SEG1: 0x000000ed (decimal:  237) (ignoramos el SEG1)
  VA  1: 0x00000024 (decimal:   36) --> VALID in SEG1: 0x000000e3 (decimal:  227) (ignoramos el SEG1)

## Semilla 99139

Ambas direcciones virtuales apuntan al segmento 1. Viendo la VA 0 necesitamos un segmentation fault. Para esto requerimos que la direccion 50
este fuera de rango. Esto nos establece un limite de 13 bytes para el segmento 1 (estamos con 6 bit, entonces el complemtento a 2 de 50 es 14) este fuera de . Dicho de otra forma necesitamos que las direcciones virtuales del segmento 1 sean de la direccion 51 a la direccion 63. Viendo la VA 1 tenemos un acceso en
la direccion virtual 40 para el mismo segmento. En base a lo concluido para la VA 0 no podemos satisfacer que VA 1 tenga un acceso exito. La misma pide que
40 este dentro del rango de direcciones alta pero como el limite llega hasta la direccion 51, no es posible obtener parametros de simulacion
para obtener dicho objetivo.  La simulacion es imposible.

## Semilla 102071

Queremos que una direccion VA 0 27 que apunta a la direccion PA 237 (segmento 0), una direccion VA 1 36 que apunta a la direccion PA 227.

Para la VA 0 27 no tenemos muchas complicaciones. Con un limite de 27 y un base de 210 para el segmento 0, cumplimos dicho objetivo.
Para la VA 1 36 queremos que, por lo menos, 36 sea el tope del stack, es decir que las direcciones VA [36, 37, ... , 63] sean el segmento 1
Esto nos indica que el limite del segmento 1 sea de 28 direcciones. Y una base igual de 255 (255 - 28 = 227!). Aca tenemos un problema, se nos superponen los segmentos fisicos. Por la VA 0 necesitamos que SEG 0 = [210, 237] y por la VA 1 necesitamos que SEG 1 = [227, 255] tenemos una superposicion entre [227, 237]. Ambos segmentos estan tomando limites minimos y aun asi tienen superposicion. La simulacion es imposible.

./segmentation.py -a 64 -p 256 -s 102071 -A 27,36 -b 210 -l 27 -B 255 -L 28 -c

Error: segments overlap in physical memory

# Parte 3

Si tenemos un espacio de dirrecciones virtuales de 5 bits podemos tener en total un 2^5 = 32 direcciones virtuales
Con un espacio de direcciones fisicas de 7 bits tenemos mapeadas 2^7 = 128 direcciones 

Que existan dos direcciones virtuales que mapeen a una misma direccion fisica implica que que haya una superposicion
de los bloques de memoria fisicos. Sin embargo en la corrida tenemos un error en la ejecucion. Teoricamente tenemos una 
direccion fisica mapeada por dos direcciones virtuales. 

./segmentation.py -a 64 -p 256 -s 102071 -b 210 -l 27 -B 255 -L 28 -c
Error: segments overlap in physical memory

Se puede tener la memoria mapeada de forma valida ya que tenemos un espacio fisico de 128 direcciones y dos segementos de 16 bytes cada uno
Como la herramienta de simulacion nos da errores de solpamaineto daremos un ejemplo que no haya superposicion. Por ejemplo tomando el caso 
de maximo tamaño de segmentos. Por ejemplo aca tenemos direcciones fisicas que mapean a VA [10, 26] y [103, 119]. Con esta configuracion es posible 
que al menos el 90% de las direcciones virtuales sean validas

./segmentation.py -a 32 -p 128 -s 102071 -A 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 -b 10 -l 16 -B 120 -L 16 -c
./segmentation.py -a 32 -p 128 -s 102071 -A 16,17,18,19,20,21,22,23,24,25,25,26,27,28,29,30,31 -b 10 -l 16 -B 120 -L 16 -c

No es posible que el 90% de la direccion fisicas mapeen a direciones virtuales validas. Esto se debe a que con un espacio de dirreciones virtuales
de 32 bytes y con un espacio de direcciones fisicas de 128 bytes, tenemos que solo el 25% del espacio de direcciones fisicas es mapeable por VA.
Si queremos que mas PA mapeen a VA debemos aumentar el espacio virtual.

