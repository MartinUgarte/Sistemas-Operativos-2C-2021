TP4: Sistema de archivos e intérprete de comandos
========================


Lecturas y escrituras
---------------------
Responder
- ¿Qué es super->s_nblocks?

`super->s_nblocks` corresponde a la cantidad de bloques que tiene el disco 

- ¿Dónde y cómo se configura este bloque especial?

Se configura en la funcion `opendisk` definida dentro del archivo `fs/fsformat.c`. Se toma el valor de la variables
nblocks que se recibe por paramentro dentro del main de dicho archivo.