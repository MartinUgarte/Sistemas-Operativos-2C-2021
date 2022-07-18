TP3: Multitarea con desalojo
========================

env_return
---------
Responder:

- al terminar un proceso su función `umain()` ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde
 que termina `umain()` hasta que el kernel dispone del proceso.
 
 Luego de llamar a `umain()`, en la clase `libmain.c` (metodo `libmain()`) se llama a `exit()` con el fin destruir el proceso. Dentro
 del `exit()` (clase lib/exit.c) efectivamente se llama a `sys_env_destroy` (syscall) que finalmente llama a `env_destroy()`
 
- ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?

La mencionada funcion `env_destroy()` hasta el tp2 solo destruia el proceso liberando los recursos (solo se podía correr uno a la vez
en un unico cpu) pero en el tp3 al tener multiples CPUs y estados se le agregaron unas responsabilidades mas a la función,
ahora se debe verificar si el proceso se esta ejecutando en otro cpu, en cuyo caso actualizarle el estado (DYING) y en
caso que el proceso sea el actual ejecutandose se lo destruye y se llama a `sched_yield` con el fin de que el se designe 
al siguiente proceso a ejecutarse.


sys_yield
---------
Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho
programa, y mostrar y explicar la salida de make qemu-nox.
 
Salida del código.
 ```
fernando-sinisi@ubuntu:~/Documentos/sisop/sisop_2021b_g28_illescas_sinisi$ make qemu-nox
+ cc kern/init.c
+ ld obj/kern/kernel
+ mk obj/kern/kernel.img
***
*** Use Ctrl-a x to exit qemu
***
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log -smp 1  -d guest_errors
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
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
No runnable environments in the system!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.

```
Se llama al programa 3 veces y dentro ejecutar un for, llamando en cada una de las 5 iteraciones a la syscall `sys_yield()`
Se puede ver que se crean 3 procesos (environment) con los ids:  `00001000`, `00001001` y `00001002`. 
Como `sys_yield()` termina llamando a `sched_yield()` que implementa un scheduling round robin terminamos observando que 
las 5 iteraciones del for de cada proceso se van intercalando ejecutandose una a la vez hasta que terminan.

envid2env
---------
Responder:

- ¿Qué ocurre en JOS si un proceso llama a sys_env_destroy(0)?.

Se estaría destruyendo el proceso actual, esto porque dentro de `sys_env_destroy` se llamará a la función `envid2env`
para obtener el proceso con dicho id (0), y esta justamente verifica si el `envid` (el id obtenido por parametro) es 0 
y en tal caso devuelve el proceso actual para finalmente llamar a `env_destroy()`.

dumbfork
---------
Tras leer con atención el código, se pide responder las siguientes preguntas:
- Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de
 solo-lectura en las páginas copiadas?
 
 No se preserva el flag en el hijo debido a que al copiar las paginas le asigna los permisos de lectura y escritura, esto
 ocurre en la función `duppage()` que es llamada en `dumbfork()` 

- Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo
 lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly 
 que indicase si la página es modificable o no. Ayuda: usar las variables globales uvpd y/o uvpt.
 
 ```
 envid_t dumbfork(void) {
     // ...
    for (addr = UTEXT; addr < end; addr += PGSIZE) {
        bool readonly;
        pde_t pde = uvpd[PDX(addr)];
        pte_t pte = uvpt[PGNUM(addr)];
        readonly = !(pde & PTE_P) && (pte & PTE_P) && (pte & PTE_W))
        duppage(envid, addr, readonly);
     }
     // ...
```
- Supongamos que se desea actualizar el código de duppage() para tener en cuenta el argumento readonly: si este es
 verdadero, la página copiada no debe ser modificable en el hijo. Es fácil hacerlo realizando una última llamada
  a sys_page_map() para eliminar el flag PTE_W en el hijo, cuando corresponda:
  
```
  void duppage(envid_t dstenv, void *addr, bool readonly) {
      // Código original (simplificado): tres llamadas al sistema.
      sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
      sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W);
  
      memmove(UTEMP, addr, PGSIZE);
      sys_page_unmap(0, UTEMP);
  
      // Código nuevo: una llamada al sistema adicional para solo-lectura.
      if (readonly) {
          sys_page_map(dstenv, addr, dstenv, addr, PTE_P | PTE_U);
      }
  }
```
Esta versión del código, no obstante, incrementa las llamadas al sistema que realiza duppage() de tres, a cuatro.
Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly, pero sin usar en ningún
caso más de tres llamadas al sistema. Ayuda: Leer con atención la documentación de sys_page_map() en kern/syscall.c,
 en particular donde avisa que se devuelve error: if (perm & PTE_W) is not zero, but srcva is read-only in srcenvid’s address space.
 
 Nuevo código:
```
  void duppage(envid_t dstenv, void *addr, bool readonly) {
      int perm = PTE_P | PTE_U | PTE_W;
      if(readonly){
        perm = PTE_U | PTE_P;
      }
      sys_page_alloc(dstenv, addr, perm);
      sys_page_map(dstenv, addr, 0, UTEMP, perm);
  
      memmove(UTEMP, addr, PGSIZE);
      sys_page_unmap(0, UTEMP);
  }
```  
 
multicore_init
---------
En este ejercicio se pide responder las siguientes preguntas sobre cómo se habilita el soporte multi-core en JOS.
Para ello se recomienda leer con detenimiento, además del código, la Parte A de la consigna original al completo.
Preguntas:

- ¿Qué código copia, y a dónde, la siguiente línea de la función `boot_aps()`?
```
memmove(code, mpentry_start, mpentry_end - mpentry_start);
```
En esta línea se copia el código de inicio del AP (application processor) ubicado en el archivo `mpentry.S` 
a una ubicación de memoria direccionable en el modo real (`MPENTRY_PADDR = 0x7000`).

- ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el
 archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?
 
La variable global `mpentry_kstack` se usa para determinar que espacio de memoria usar para el stack del mpentry.S.
El mismo no se define directamente en el archivo mpentry.S porque sino seria fijo, es decir, el mismo para todas las cpu 
y esto lo queremos evitar.

- En el archivo kern/mpentry.S se puede leer:
```
 # We cannot use kern_pgdir yet because we are still
 # running at a low EIP.
 movl $(RELOC(entry_pgdir)), %eax
```
- ¿Qué valor tendrá el registro %eip cuando se ejecute esa línea? Responder con redondeo a 12 bits,
 justificando desde qué región de memoria se está ejecutando este código.

 Segun la informacion en la consigna original, esta linea corresponde a una funcion de un pplication Processor. La funcion boot_aps mapea el codigo de este entry point (comprendido por mpentry_start y mpentry_end) en la direccion 0x7000 (MPENTRY_PADDR). Para el momento de la ejecucion de instruccion %eip deberia tener el valor inicial (0x7000) mas el offset del numero de instruccion que corresponde desde mpentry_start (18 en este caso). Usando un redondeo de 12 bits vemos que 18 es un numero bajo redondea para abajo por lo que el valor del registro seguira siendo 0x7000.
 
 
ipc_recv
---------
Una vez implementada la función, resolver este ejercicio:
Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no?
```
 envid_t src = -1;
 int r = ipc_recv(&src, 0, NULL);
 
 if (r < 0)
   if (/* ??? */)
     puts("Hubo error.");
   else
     puts("Valor negativo correcto.")
```

Si la funcion fallara, al recibir un parametro from_env_store no nullo, ante un error de syscall lo que hara es setear en la direccion apuntadao dicho parametro, un valor de cero.

Por lo cual si se quisiera hacer una correcta distincion entre error de syscall o no se usaria el siguiente codigo.
```
 envid_t src = -1;
 int r = ipc_recv(&src, 0, NULL);
 
 if (r < 0)
   if (!src)
     puts("Hubo error.");
   else
     puts("Valor negativo correcto.")
```

sys_ipc_try_send
---------
Se pide ahora explicar cómo se podría implementar una función sys_ipc_send() que sea bloqueante, es decir, que si un proceso A la usa para enviar un mensaje a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y despertado una vez B llame a ipc_recv() (cuya firma no debe ser cambiada).

Una posible implementacion es la de agregar algun campo al struc Env que indique si el proceso quiere enviar un mensaje, analogo a env_ipc_recving. Este campo, env_ipc_sending debera ser validado en las funciones sys_ipc_send y ipc_recv.

Centrandonos en sys_ipc_send antes de enviar tenemos que ver si el proceso B esta recibiendo informacion.


Se podría agregar un campo a la estructura env que sea env_ipc_sending, el cual indica si el proceso quiere enviar un mensaje o no. Entonces, tanto sys_ipc_send como sys_ipc_recv deberán validar si el otro proceso quiere recibir/enviar mensajes (según corresponda). 

Por ejemplo, en el caso de sys_ipc_send, tendria una implementacion similar a sys_ipc_try_send, con la diferencia en la validacion del proceso recibiendo informacion.

Aqui reemplazariamos algo de la siguiente forma

```
if (e->env_ipc_recving == false)
		return -E_IPC_NOT_RECV;
```
por 

```
while ( (e->env_ipc_recving == false) {
		curenv->env_status = ENV_NOT_RUNNABLE;
		curenv->env_ipc_sending = true;
		sys_yield();
}
```

De esta forma el proceso A queda bloqueado hasta que el proceso B pase su env_ipc_recving a true.

En lo que respecta a la recepcion del mensaje en sys_ipc_recv, deberiamos recibir el envid (podria agregar una lista al struc Env de envids del cual escuchar mensajes). Se desencola el primer envid, validamos si si flag env_ipc_sending es true y en dicho caso recibimos el mensaje normalmente, modificando el status del proceso que envio el mensaje ENV_RUNNABLE y su flag de env_ipc_sending a false.
En caso que no sea asi, continua la implementacion actual de setearle al currenv su estado en ENV_NOT_RUNNABLE y env_ipc_recving en true.

Este encolado permite que más de un proceso le envíe mensajes a B y el orden en el que se despertarían los procesos depende del orden en el que B quiera ir recibiendo los mensajes.

En lo que respecta a los deadlocks, puede suceder el ejemplo clasico de deadlock circular. A le quiere enviar a B, B quiere enviarle a C y C quiere enviarle a A, pero ninguno esta queriendo recibir los mensajes de los otros procesos y queda en un deadlock. Todos quedan bloqueados enviando y ninguno recibe nada.