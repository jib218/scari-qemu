# scari-qemu

This is a hacked qemu for research purposes. Currently it supports to inject
values into memory areas and to simulate stuckat memory cells. This is based
on [FIES](https://github.com/ahoeller/fies), but ported to qemu version 2.8
and with easier to use hmp commands. In general, this should work with all
architectures supported by qemu, however we only tested it with x86_64-softmmu.

## Added qemu hmp commands

Command | Description  
--|--
inject_value \<gvma\> \<hex value\> | It overwrites the bytes at the specified guest virtual memory address with the hex value (without leading 0x).  
inject_stuckat_value \<gvma\> \<hex value\> | Injects the specified value every time _tb_find_ or a function of the _softmmu_template.h_ file is called. It misses writes followed by a read inside a translation block. Generating additional instructions would be necessary for achieving that.
remove_stuckat \<gvma\> | Removes a stuckat address.

The added fault injection code resides in the _fies_ subdirectory.

## Added qemu options

Option | Description
--|--
-profiling g | Enables the profiler_log_generic function and appends the specified string to a file named _profiling-generic.txt_

## Useful existing hmp commands

Command | Description  
--|--
x \<gvma\> |  Inspects the memory content at the specified guest virtual memory address.
stop  | Stops qemu.
c  | Continues qemu.  

## How to build

It follows the same build process like an unchanged qemu:

~~~bash
# inside qemu source directory
./configure --target-list=x86_64-softmmu --enable-gtk
make -j 4
~~~

Clean directory:

~~~bash
make distclean && rm -rf *-softmmu
~~~

## How to fault inject an application running on linux.

Following command executes a linux kernel which starts an application named
_hello_. A qemu monitor is now available through stdio.

~~~bash
./qemu-system-x86_64 -kernel bzImage -initrd newinitrd.img -append "root=/dev/ram rdinit=/hello" -monitor stdio
~~~

The qemu emulation can be stopped:

~~~sh
(scari-qemu) stop
~~~

A memory content can be viewed with the _x_ command. Note that one needs the
guest virtual address.

~~~sh
(scari-qemu) x 0x7ffcc9422b6c
00007ffcc9422b6c: 0x0000002a
~~~

A new value can be inserted with _inject_value_.

~~~sh
(scari-qemu) inject_value 0x7ffcc9422b6c 18
Number of overwritten bytes: 1
Success
~~~

The emulation can be continued with the command _c_.

For permanently setting a memory area to a value the _inject_stuckat_value_
command can be used. This command misses situations where a read happens after a
write within a qemu translation block. Catching these situations would require
to generate additional instructions.

~~~sh
(scari-qemu) inject_stuckat_value 0x7ffcc9422b6c 0000002a
~~~

A stuckat value can be removed with the command _remove_stuckat_

~~~sh
(scari-qemu) remove_stuckat 0x7ffcc9422b6c
~~~

### How to create newinitrd.img

Create initrd, last number must match kernel version. e.g.:

~~~bash
mkinitramfs -o initrd.img-4.9.41
~~~
extract image, should create kernel dir
put application inside kernel dir (e.g. hello)

~~~bash
cd kernel
# move application inside this dir
find . | cpio --create --format="newc" > ../newinitrd
cd ..
gzip newinitrd
mv newinitrd.gz newinitrd.img
~~~

### Example test program hello.c

~~~c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  int x = 0x2a;
  int *y = malloc(3 *sizeof(int));
  y[0] = 0xabc;

  while(1) {
    printf("Address x: %p\n", &x);
    printf("Address y: %p\n", &y);
    printf("Value x: %x\n",x);
    printf("Value y: %x\n", y[0]);

    sleep(2);

    if(x == 0x18)
      printf("You hacked me!!!\n");
    else if(x == 0x2a)    
      x = 0xabcdabcd;
    else
      x = 0x2a;
  }

  free(y);
  return 0;
}
~~~

~~~bash
gcc -static -o hello hello.c
~~~
