TP2: Procesos de usuario
========================

env_alloc
---------

1. ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)

La generacion de identificadores para los procesos consta de una serie de operaciones logicas. Primero parte de almacenar la varible `generation`
que esta dada por la siguiente formula.

> generation = (e->env_id + (1 << ENVGENSHIFT)) & `~(NENV - 1)`

Tenemos un par de constante tales como `ENVGENSHIFT = 12` y `NENV = (1 << LOG2NENV)` Como estamos ante la primera creaciones de procesos, `e->env_id` tendra un valor de `0`. Con estos datos podemos reemplazar

> (0 + (1 << 12)) & `~((1 << 10) - 1)`

> 1000000000000 & `~(10000000000 - 1)`

> 1000000000000 & `~01111111111`

> 1000000000000 & 1110000000000

> 1000000000000

Posterior a este calculo se hace una validacion de que no sea negativo. En caso de serlo se aplica

> generation = 1 << ENVGENSHIFT

Como no es nuestro caso seguimos con la siguiente operacion. La variable e que es el siguiente struct env disponible (se setea apuntando a env_free_list). envs es el arreglo de envs.

> e->env_id = generation | (e - envs)

La "resta" de `(e - envs)`nos da por resultado el indice del e disponible.

```
Proceso 0 -> e - envs = 0
Proceso 1 -> e - envs = 1
Proceso 2 -> e - envs = 2
Proceso 3 -> e - envs = 3
Proceso 4 -> e - envs = 4
```

Y luego tenemos la operacion `generation | (e - envs)`

```
Proceso 0 -> 1000000000000 | 0 = 0x1000
Proceso 1 -> 1000000000000 | 1 = 0x1001
Proceso 2 -> 1000000000000 | 2 = 0x1002
Proceso 3 -> 1000000000000 | 3 = 0x1003
Proceso 4 -> 1000000000000 | 4 = 0x1004
```

2. Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?

Como vimos previamente, un elemento asociado a envs[630], tendrá un id de 0x1276. Si se destruye se libera el struct, por lo que el nuevo proceso tendra el mismo lugar 630.

Primera ejecución (630 = 01001110110b)

> generation = (01001110110 + 1000000000000) & `~00001111111111`

> 01001001110110 & 11110000000000

> 1000000000000

Entonces 
> 1000000000000 | 01001110110 = 1001001110110 = 0x1276

> e->env_id = 0x1276

Las siguientes ejecuciones sucede lo mismo.

e->env_id = 0x1276

env_init_percpu
---------------

La funcion env_init_percpu lo primero que hace es llama a lgdt. Si venos su definicion es 

```
static inline void
lgdt(void *p)
{
	asm volatile("lgdt (%0)" : : "r" (p));
}
```
*asm* es una sintaxis que ayuda a poder utilizar variables de C para la escritura y lectura de variables de assembler.

*volatile* desacitva ciertas optimizaciones a fin de que asm sea seguro de usar.

*lgdt* se llama Load Global Descriptor Table Register. Lo que hace es escribir el valor recibido almacenado en la direccion pasada por parametro (en este caso el puntero de gdt_pd) y escribe el global descriptor table register. La instruccion escribe 6 bytes. Estos bytes representan el tamaño y la direccion de la tabla respectivamente.


env_pop_tf
----------

1. En este caso env_pop_tf recibe un puntero al trapfame. Por la convecion de escritura de x86 sabemos que el el tope de la pila corresponde al parametro pasado. Antes de llamar a popal se tiene en la pila del stack al elemento pasado por parametro, que apunta a un struct PushRegs (primer atributo del struc Trapframe).

    popal saca de la pila los siguientes 8 registros, de esta manera nos movemos todo el struct de PushRegs.

    Para este punto *popl %%es* y *popl %%ds* sacan de la pila 8 bytes respectivamente, siendo esto los atributos tf_es y tf_ds con sus paddings.

    *addl $0x8,%%esp* desplaza el punte del stack pointer por 8 bytes salteando de esta forma tf_trapno
 y tf_err del struct del trapframe.

    Antes de ejecutar la instruccion *iret* en el tope de la pila tenemos al elemento tf_eip.
 En el 3er lugar del stack tenemos a tf_eflags.

2. La cpu guarda el nivel de privilegio actua en el CPL (current privilege level) en el registro CS. Su size es de dos bits. Para determinar si hay un cambio de privilegio ante la ejecucion de una nueva instruccion se valida con el DPL (descriptor privilege level). Este valor esta en el sd_dpl de la estructura Segdesc.


gdb_hello
---------

2.
```
EAX=003bc000 EBX=f01c1000 ECX=f03bc000 EDX=00000211
ESI=00010094 EDI=00000000 EBP=f0119fd8 ESP=f0119fbc
EIP=f0102f3a EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

3. Impresion del argumento tf
```
(gdb)  p tf
$1 = (struct Trapframe *) 0xf01c1000

```



4. vemos cuanto vale N

```
(gdb) print sizeof(struct Trapframe) / sizeof(int)
$2 = 17
```

Entonces con

```
(gdb)  x/17x tf
0xf01c1000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c1010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c1020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01c1030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01c1040:     0x00000023
```

5. Avanzamos hasta luego del `movl ...,%esp`

```
(gdb) disas
Dump of assembler code for function env_pop_tf:
   0xf0102f3a <+0>:     push   %ebp
   0xf0102f3b <+1>:     mov    %esp,%ebp
   0xf0102f3d <+3>:     sub    $0xc,%esp
   0xf0102f40 <+6>:     mov    0x8(%ebp),%esp
=> 0xf0102f43 <+9>:     popa   
   0xf0102f44 <+10>:    pop    %es
   0xf0102f45 <+11>:    pop    %ds
   0xf0102f46 <+12>:    add    $0x8,%esp
   0xf0102f49 <+15>:    iret   
   0xf0102f4a <+16>:    push   $0xf010593f
   0xf0102f4f <+21>:    push   $0x1ee
   0xf0102f54 <+26>:    push   $0xf01058fa
   0xf0102f59 <+31>:    call   0xf01000a9 <_panic>
End of assembler dump.
```

6. Reejecutamos 

```
(gdb) x/17x tf
0xf01c1000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c1010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c1020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01c1030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01c1040:     0x00000023
```


7. Describir los valores

Vemos por que esta compuesto trapframe.
 
```
struct PushRegs {
	/* registers as pushed by pusha */
	uint32_t reg_edi;
	uint32_t reg_esi;
	uint32_t reg_ebp;
	uint32_t reg_oesp;		/* Useless */
	uint32_t reg_ebx;
	uint32_t reg_edx;
	uint32_t reg_ecx;
	uint32_t reg_eax;
} __attribute__((packed));

struct Trapframe {
	struct PushRegs tf_regs;
	uint16_t tf_es;
	uint16_t tf_padding1;
	uint16_t tf_ds;
	uint16_t tf_padding2;
	uint32_t tf_trapno;
	/* below here defined by x86 hardware */
	uint32_t tf_err;
	uintptr_t tf_eip;
	uint16_t tf_cs;
	uint16_t tf_padding3;
	uint32_t tf_eflags;
	/* below here only when crossing rings, such as from user to kernel */
	uintptr_t tf_esp;
	uint16_t tf_ss;
	uint16_t tf_padding4;
} __attribute__((packed));

```

Vemos que primero tenemos a PushRegs (que tiene un size de 32 bytes). Estos son los 8 registros de valores generales. Luego dentro del struct trapFram tememos a tf_es y tf_ds mas sus respectivos paddings.  Algo similar sucede con tf_cs y tf_ss. Entremedio tenemos los atributos que tienen un size de 4 bytes.

```
0xf01c1000:	reg_edi	            reg_esi	                reg_ebp	        reg_oesp
0xf01c1010:	reg_ebx	            reg_edx	                reg_ecx	        reg_eax
0xf01c1020:	(tf_es+tf_padding1)	(tf_ds + tf_padding2)	tf_trapno	    tf_err
0xf01c1030:	tf_eip	            (tf_cs+tf_padding3)	    tf_eflags	    tf_esp
0xf01c1040:	(tf_ss+tf_padding4)
```

Los valores que no son nulos:

```
tf_es = extra segment
tf_ds = data segment
tf_eip = Extended instruction pointer
tf_cs = code segment
tf_esp = stack pointer
tf_ss = stack segment
```

La inicilizacion de tf_ds, tf_es, tf_ss, tf_cs y tf_esp sucede en la funcion `env_alloc`. La de tf_eip sucede en `load_icode`.


8. Iret

```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c1030
EIP=f0102f49 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

Vemos que los valores de los registros generales fueron modificados por los valores que estaban en el trapframe impreso previamente. Hubo modificaciones en el registro esp por la ejecucion previa antes del iret `add    $0x8,%esp`
El privilege level se puede ver en el DLP de la 4ta linea (vale cero) indicando que estamos en una ejecucion de kernel ring

9. Imprimos el contador
```
(gdb) p $eip
$5 = (void (*)()) 0x800020
```

Cargamos los simbolos y luego 

```
(gdb) p $eip
$6 = (void (*)()) 0x800020 <_start>
``` 

Vemos los registros

```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```

El valor de los regitros de proposito general siguen igual al trapframe. El esp fue modificado (y tiene ahora el valor que estaba en el trapfram = eebfe000). Los privilegios pasaron a ser nivel 3.

10. syscall

```
(gdb) disas
Dump of assembler code for function syscall:
   0x008009d9 <+0>:     push   %ebp
   0x008009da <+1>:     mov    %esp,%ebp
   0x008009dc <+3>:     push   %edi
   0x008009dd <+4>:     push   %esi
   0x008009de <+5>:     push   %ebx
   0x008009df <+6>:     sub    $0x1c,%esp
   0x008009e2 <+9>:     mov    %eax,-0x20(%ebp)
   0x008009e5 <+12>:    mov    %edx,-0x1c(%ebp)
   0x008009e8 <+15>:    mov    %ecx,%edx
=> 0x008009ea <+17>:    mov    0x8(%ebp),%ecx
   0x008009ed <+20>:    mov    0xc(%ebp),%ebx
   0x008009f0 <+23>:    mov    0x10(%ebp),%edi
   0x008009f3 <+26>:    mov    0x14(%ebp),%esi
   0x008009f6 <+29>:    int    $0x30
   0x008009f8 <+31>:    cmpl   $0x0,-0x1c(%ebp)
   0x008009fc <+35>:    je     0x800a02 <syscall+41>
   0x008009fe <+37>:    test   %eax,%eax
   0x00800a00 <+39>:    jg     0x800a0a <syscall+49>
   0x00800a02 <+41>:    lea    -0xc(%ebp),%esp
   0x00800a05 <+44>:    pop    %ebx
   0x00800a06 <+45>:    pop    %esi
   0x00800a07 <+46>:    pop    %edi
   0x00800a08 <+47>:    pop    %ebp
   0x00800a09 <+48>:    ret    
   0x00800a0a <+49>:    mov    -0x20(%ebp),%edx
   0x00800a0d <+52>:    sub    $0xc,%esp
   0x00800a10 <+55>:    push   %eax
   0x00800a11 <+56>:    push   %edx
   0x00800a12 <+57>:    push   $0x800f6c
   0x00800a17 <+62>:    push   $0x23
   0x00800a19 <+64>:    push   $0x800f89
   0x00800a1e <+69>:    call   0x800ab3 <_panic>
```

vemos los registros
 

```
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=eebfde88 EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=eebfde18
EIP=008009ea EFL=00000096 [--S-AP-] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```

Ejecutamos int

```
(gdb) si 1
=> 0xf010382c <trap_48+2>:      push   $0x30
0xf010382c in trap_48 () at kern/trapentry.S:67
67      TRAPHANDLER_NOEC(trap_48, T_SYSCALL)
```
```
EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=efffffe8
EIP=f010382c EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

Vemos que cambia el EIP, el ESP y el CPL a 0 dejandonos en un ambiente de kernel. la interrupcion nos devolvia el contexto de kernel mode.

kern_idt
---------

1. El uso de TRAPHANDLER y TRAPHANDLER_NOEC depende si la interrupcion devuleve un codigo de error. Si es asi se desea saber que fue lo sucedido por lo cual se procede con TRAPHANDLER. Caso contrario se usa TRAPHANDLER_NOEC que envia un 0 como codigo de error. Si se usara siempre TRAPHANDLER se enviaria informacion "desfasada" al falta el campo de error, por lo que se tiene que rellenar de alguna forma ese campo faltante.



2. El parametro istrap es un flag indica estamos ante una trap gate(vale 1) o si tratamos con un interrupt gate(vale 0). La diferencia de este flag es el la modificacion del flag IF (interrupt-enable flag).
Para un interrup gate se resetea el flag IF, impiendo cualquier interferencia de otrsa interrupciones con la interrupción actual. 
para un trap gate, no se resetea el IF, permineitndo el manejo de otras excepciones junto con la actual.


3. Cuando corremos make run-softint-nox tenemos lo siguiente

[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
TRAP frame at 0xf01c1000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdfd0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000d General Protection
  err  0x00000072
  eip  0x00800036
  cs   0x----001b
  flag 0x00000082
  esp  0xeebfdfd0
  ss   0x----0023
[00001000] free env 00001000
Destroyed the only environment - nothing more to do!

En la arquitectura x86 la ejecucion de `int` genera una excepcion (page fault mas especificamente, 0x0Eh). Como se ve en el trap frame de arriba existe ya una excepcion que esta siendo manejada (General Protection, 0x0Dh o 13). Esta exception la tenemo debido a que la interrupcion original (page fault) es lanzada desde modo usuario y en realidad solo puede ser lanzada con nivel de privilegio 0. Asi se proteje de que un programa con privilegios de usuario no arroje cualquier excepcion, pudiendo asi alterar el estado del sistema.

Se podria modifica el seteo del permiso modificadno el DPL descriptor privilege level correspondiente a PageFault

> SETGATE(idt[T_PGFLT], 0, GD_KT, trap_handler, 3);


user_evilhello
---------
Corrida original

[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
f�rIncoming TRAP frame at 0xefffffbc
[00001000] exiting gracefully
[00001000] free env 00001000


Corrida modificados
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
[00001000] user fault va f010000c ip 00800039
TRAP frame at 0xf01c1000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdfd0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000e Page Fault
  cr2  0xf010000c
  err  0x00000005 [user, read, protection]
  eip  0x00800039
  cs   0x----001b
  flag 0x00000082
  esp  0xeebfdfb0
  ss   0x----0023
[00001000] free env 00001000

1. La modificacion del programa acorde a la consigna tiene como finalidad hacer lo mismo (imprimir el primer byte del kernel entry point), sin embargo, en la modificacion se guarda el valor que esta almacenado en la direccion 0xf010000c guardando la direccion y desreferenciandola.

2. En la version original la direccion del entry point se lo pasa como input a la syscal que enmascara la funcion sys_cputs. Dentor de esta ejecucion la direccion sera ejecutada bajo modo kernel y no habra problemas. En la version modificada al desreferenciar la direccion se tiene un acceso a un bloque de memoria no permitido y de ahi, para proteger al sistema, se lanza la excepcion de Page Fault.

3. Tanto en la version original como la modificada se quiere acceder a la direccion (0xf010000c). En la original se pasa el valor de la direccion como un input accediendo a la misma con un ring 0. En la version modificada la desreferencia se ejecuta aun en un programa con un ring 3 (modo usuario).

    Es un problema el que no haya una validacion de accesos de memoria para ejecuciones dentro del ring 0. Implementando correctamente user_mem_check se evitara este error.
