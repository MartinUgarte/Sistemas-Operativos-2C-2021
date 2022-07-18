TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------
1. Incluir: Un cálculo manual de la primera dirección de memoria que devolverá `boot_alloc()` tras el arranque. Se puede calcular a partir del binario compilado (obj/kern/kernel), usando los comandos `readelf` y/o `nm` y operaciones matemáticas.

2. Incluir: Una sesión de GDB en la que, poniendo un breakpoint en la función `boot_alloc()`, se muestre el valor de `end` y `nextfree` al comienzo y fin de esa primera llamada a `boot_alloc()`.

Para calcular la direccion que devolvera boot_alloc tras el arranque necesitamos saber primero el valor de `end` y esto lo logramos de la siguiente manera:
```bash
fernando-sinisi@ubuntu:~/Documentos/sisop/sisop_2021b_g28_illescas_sinisi$ readelf -s obj/kern/kernel | grep end
   110: f0117950     0 NOTYPE  GLOBAL DEFAULT    6 end
```
Luego este valor lo redondea según el  `PGSIZE = 4096`
```
end = 0xf0117950 = 4027677008
4027677008 % 4096 = 2384
4096 - 2384 = 1712
nextfree = 4027677008 + 1712 = 4027678720 = 0xf0118000
```
Verificando esto con una corrida de gdb, nos da el mismo resultado:

```
fernando-sinisi@ubuntu:~/Documentos/sisop/sisop_2021b_g28_illescas_sinisi$ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Leyendo símbolos desde obj/kern/kernel...hecho.
Remote debugging using 127.0.0.1:26000
aviso: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b boot_alloc
Punto de interrupción 1 at 0xf0100b30: file kern/pmap.c, line 89.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf0100b30 <boot_alloc>:     push   %ebp

Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:89
89      {
(gdb) p &end
$1 = (<data variable, no debug info> *) 0xf0117950
(gdb) p nextfree
$2 = 0x0
(gdb) watch &end
Watchpoint 2: &end
(gdb) watch nextfree
Hardware watchpoint 3: nextfree
(gdb) c
Continuando.
=> 0xf0100b62 <boot_alloc+50>:  jmp    0xf0100b3e <boot_alloc+14>

Hardware watchpoint 3: nextfree

Old value = 0x0
New value = 0xf0118000 ""
```



page_alloc
----------
1. Responder: ¿en qué se diferencia page2pa() de page2kva()?

La diferencia entre page2pa() y page2kva() es que la primera recibe una página y devuelve su dirección física y la
 segunda recien también una pagina pero devuelve su correspondiente dirección virtual.
 
 `page2pa()` recibe `PageInfo*` y retorna `physaddr_t` (2pa significa to physical address)
 `page2kva()` recibe `PageInfo*` y retorna `void*` (2kva significa to kernel virtual address)


map_region_large
----------------
1. Responder las siguientes dos preguntas, específicamente en el contexto de JOS:
   
   ¿cuánta memoria se ahorró de este modo? (en KiB)
   ¿es una cantidad fija, o depende de la memoria física de la computadora?
   
Para mapear la primera región de memoria fisica (de 4MiB) se crean un page directory y una page table, la cual hace
referencia a cada una de las paginas de 4KiB (1024 * 4KiB = 4MiB) pero al cambiar a paginas de 4MiB con contar con solo
el page directory ya alcanza porque este ya puede direccionar un pagina de 4MiB sin necesidad de crear una page table
por lo que se ahorra la creacion de la page table (`entry_pgtable`), es decir se ahorran 4KiB (1024 entradas * 4iB).
La cantidad de memoria a ahorrar es fija por large page usada, siempre 4KiB, pero en el caso de usar toda la memoria
fisica disponible (arquitectura de 32 bit -> 4GB) mediante large pages se ahorrarian 4KiB por cada tabla de pagina no
creada entonces la memoria ahorrada va a ir de 4KiB a 4MiB (4MiB = 4GiB / 4MiB * kKiB).

