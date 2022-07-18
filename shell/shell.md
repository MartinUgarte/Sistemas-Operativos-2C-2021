# Lab: shell

### Búsqueda en $PATH
Responder:  ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la 
librería estándar de C (libc) exec(3)?

Respuesta: La diferencia es que la familia de funciones de la libreria estandar de C cumple la tarea de brindar una interfaz
mas "amigable" para el usuario, contando con distintas maneras de ejecutar la syscall execve(2).
Justamente para lograr esto la familia de wrappers consta de varias firmas de funciones, que permiten recibir distintos
parametros, como por ej: 

**execl, execlp, execle** (poseen un caracter 'l' en su nombre) que envian los argumentos como una lista de parametros terminada en NULL (char* arg1, ... ,char argN, NULL)

**execv, execvp, execvpe** (poseen un caracter 'v' en su nombre) que envian los argumentos como un vector de parametros terminado en NULL (char* const argv[])

**execle, execvpe** (poseen un caracter 'e' en su nombre) que permiten enviar un vector con las variables de entorno (char* const envp[])

**execlp, execvp, execvpe** (poseen un caracter 'p' en su nombre) que buscan el ejecutable en caso de que no se pase un path (no tiene el caracter '/').

Responder: ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Respuesta: Tal como se menciona en el manual (man 3 exec) cualquiera de las funciones de esta familia exec pueden fallar
por cualquiera de los errores especificados para execve (man 2 execve) retornando de igual manera -1 y seteando la variable
 errno.

### Comandos built-in

Pregunta: ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué?
 ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en
  los built-in como true y false)
  
Respuesta: El pwd se podría implementar sin ser un built-it debido a que cuando se hace fork el proceso hijo y el padre
tienen el mismo directorio de trabajo. En cambio el cd si lo hiciera el hijo, el padre seguiría estando en el directorio
 de trabajo actual y no cumpliria la funcion de actualizar el directorio de trabajo del padre. El motivo de hacerlo como 
 built-in es la eficiencia, el comando pwd es muy usado y lanzar un proceso cada vez que se quiere consultar el directorio
 de trabajo es costoso.

---

### Variables de entorno adicionales

Pregunta: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Respuesta: Es necestario agregar las variables de entorno temporarias o adicionales luego del fork para que solo las
contenga el proceso hijo que va a ejecutar el comando y no queden para siempre en el proceso padre (nuestra shell)

Pregunta: En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e),
 se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de
  entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de
   las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).
¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

Respuesta: No es el mismo comportamiento ya que en el primer caso el proceso que se va a ejecutar con exec tiene acceso
 a todas las variables del environ es decir las actuales y las que se agregaron y en caso de pasarle las explicitamente
 al exec las variables mediante un array, solo contara con ellas y no las demas del environ

Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.
Respuesta: Una implementacion podria ser obtener todas las variables del environ colocandolas en el mismo array que tiene las que
 quiero explicitamente que tenga el proceso y pasarselas al exec como parametro.

---

### Procesos en segundo plano

Responder: Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Respuesta: Los procesos de segundo plano se ejecutan como un proceso normal con la excepcion de que el proceso 
padre que lo lanza no se queda esperando que termine (lo implemente quitando el wait si era un comando BACK). 
Al no esperar que terminen los procesos hijos, estos al terminar quedan zombies, para evitar esto agregue al principio 
de la ejecucion de cualquier comando un wait no bloqueante adentro de un bucle para que busque todos los hijos que
 terminaron y si no encuentra ninguno o encuentra a todos los que terminaron sale del bucle y el padre sigue ejecutando.

---

### Flujo estándar

Responder: Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida 
de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?
 
Respuesta: El comando 2>&1 lo que hace es redireccionar lo que haya en la salida del file descriptor 2 (en gral la salida estandar de error
si es que no se la sustituyo) a lo que haya en el file descriptor 1, el caracter '&' lo que hace es que en lugar de que lo redireccione 
a un archivo llamado 1 lo lleve a lo que este asociado el file descriptor 1.
En el caso del ejemplo:
```
ls -C /home /noexiste >out.txt 2>&1
cat out.txt
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio
/home:
fernando-sinisi
```
Se listaron los elementos del path home y del path noexiste y se guardo el resultado en el archivo out.txt, como noexiste
 justamente no existe genera un error que va hacia salida estandar de error pero con la 2>&1 redireccionandola hacia la
  salida estandar que justamente es el archivo, el resultado final es que todo este en el archivo out.txt

Si se invierte el orden el resultado cambia debido a que la redireccion de la salida de error a salida estandar ocurren
 antes que la salida estandar sea el archivo por lo tanto el mensaje de error se imprime en pantalla y no en el archivo 

```
2>&1 ls -C /home /noexiste >out.txt
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio
cat out.txt
/home:
fernando-sinisi
```

### Tuberías simples (pipes)

Responder: Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? 
¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este
 comportamiento usando bash. Comparar con la implementación del este lab.

Respuesta: El exit code reportado es del proceso que termino, en el caso de pipe que son varias ejecuciones unidas
justamente por un pipe el exit code es el correspondiente al ultimo proceso en ejecutarse. Como ejemplo podemos ver:
```
gggg | echo hola | cat gllll | llll
cat: gllll: No existe el archivo o el directorio
llll: orden no encontrada
gggg: orden no encontrada
fernando-sinisi@ubuntu:~$ echo $?
127
fernando-sinisi@ubuntu:~$ echo 5 | slevg
slevg: orden no encontrada
fernando-sinisi@ubuntu:~$ echo $?
127
fernando-sinisi@ubuntu:~$ gggg | echo hola
hola
gggg: orden no encontrada
fernando-sinisi@ubuntu:~$ echo $?
0
```
Podemos notar que el unico caso que nos dio un status de 0 (terminacion correcta) es el caso donde el ultimo comando 
se ejecuto correctamente (la linea  "gggg | echo hola")

En el caso de nuestra terminal sh, la diferencia es que aunque el ultimo comando se ejecute bien si alguno anterior 
(conectado mediente un pipe) fallo el codigo es de error. Ej:
```
$ echo 5 | hola
Fallo execcv con comando: hola
$ echo $?
65280
$ echo 5
5
$ echo $?
0
$ hola | echo 5
5
$ echo $?
65280
```

---
### Pseudo-variables

Pregunta: Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su
 uso en bash (u otra terminal similar).
 
- $_ : Obtener el ultimo campo del ultimo comando ejecutado, pudiendo ser un comando o un parametro.
- $$ : Obtener el pid del proceso actual sobre el que se ejecuta.
- $! : Obtener el pid del ultimo proceso ejecutado en background.
- $0 : Obtener el nombre del script/shell que esta corriendo.
- $? : Obtener el exit code del ultimo comando ejecutado.
```
fernando-sinisi@ubuntu:~$ echo $0
/bin/bash
fernando-sinisi@ubuntu:~$ echo $$
19744
fernando-sinisi@ubuntu:~$ sleep 2 &
[1] 20708
fernando-sinisi@ubuntu:~$ echo $!
20708
[1]+  Hecho                   sleep 2
fernando-sinisi@ubuntu:~$ echo $!
20708
fernando-sinisi@ubuntu:~$ echo $?
0
fernando-sinisi@ubuntu:~$ echo 5
5
fernando-sinisi@ubuntu:~$ echo $_
5
```

---


