## The BIOS

PC 的物理地址空间布局如下:

![physical_address_space](images/physical_address_space.JPG)

BIOS 位于物理地址空间 0x000f0000-0x000fffff, 在上电后或者任何系统重启后总是首先获得机器控制权.

## Exercise 2

> 使用 GDB si(Step Instruction) 命令跟踪 ROM BIOS 几条指令, 试图去猜测它可能在做什么. 参考  [6.828 reference materials page](https://pdos.csail.mit.edu/6.828/2017/reference.html) 上的一些资料, 不需要弄清楚所有的细节, 只需大致了解 BIOS 首先要做什么.

PC 启动后, 处理器进入实模式, 将 *CS* 设置为 0xf000,  IP 设置为 0xfff0, 从 *CS:IP* 段地址(0xffff0: 16 * 0xf000 + 0xfff0 = 0xffff0) 开始执行, BIOS 执行的第一条指令是 `jmp` 指令, 跳转到段地址`CS = 0xf000` 和`IP = 0xe05b`

![BIOS_first_instr](images/BIOS_first_instr.JPG)

0xffff0 是 BIOS 结束前的16个字节, BIOS 所做的第一件事就是 `jmp` 返回到 BIOS 中较早的位置(0xfe05b),  si 命令跟踪 BIOS 的几条指令.

![BIOS_si](images/BIOS_si.JPG)



BIOS 设置一个中断描述符表, 初始化各种设备. 在初始化 PCI 总线和 BIOS 所知道的所有的重要设备之后, 它将搜索一个可引导的设备, 如软盘、硬盘或 CD-ROM, 当 BIOS 找到一个可引导磁盘时, 从磁盘读取 *boot loader* 并将控制权交给它.

## Boot Loader

BIOS 找到可引导的软盘或硬盘时, 将 512字节的引导扇区(boot sector)加载到物理地址0x7c00~0x7dff 的内存中, 然后使用 `jmp` 指令将 *CS:IP* 设置为 0000:7c00, 将控制权传递给引导加载程序.

Boot loader 位于这512字节中, 由一个汇编语言源文件 `boot/boot.S` 和一个 C源文件 `boot/main.c` 组成, 完成两个主要功能:

- 引导加载程序将处理器从实模式切换到32位保护模式, 因为只有在这种模式下软件才能访问处理器物理地址空间中1MB以上的所有内存. 
- 引导加载程序通过 x86 特殊的 I/O 指令直接访问 IDE 磁盘设备寄存器, 从硬盘读取内核.

编译好的 Boot Loader 位于 obj/boot/, 相关代码如下:

![BootLoader](images/BootLoader.JPG)

## 实模式和保护模式

两种模式都是CPU的工作模式, 实模式(real mode) 是早期CPU运行的工作模式, 而保护模式(protected mode) 则是现代CPU运行的模式.

#### 实模式

实模式出现于早期 Intel 8086/8088 CPU时期, 20位地址总线, 地址空间1MB(0x0~0xFFFFF), 8个16位的通用寄存器, 4个16位的段寄存器. 
直接参与运算的数值都是16位, 为了支持20位 1MB寻址空间, Intel 采用分段方法, 使用两个16 位的值决定一个地址. 当某个指令访问内存地址时, 使用下面的这种格式来表示:

```
(段基址:段偏移量)
```

其中第一个字段是段基址, 它的值是由段寄存器提供的. 段寄存器有4种: %cs, %ds, %ss, %es. 具体这个指令采用哪个段寄存器是由这个指令的类型来决定的, 比如取指令就是采用 %cs 寄存器, 读取或写入数据就是 %ds 寄存器, 如果是对堆栈操作就是 %ss 寄存器. 不管什么指令, 都会有一个段寄存器提供一个16位的段基址.

第二字段是段内偏移量, 代表你要访问的这个内存地址距离这个段基址的偏移, 它的值就是由通用寄存器来提供. 采用如下公式得出 20 位的实际物理地址.

```
物理地址 = 段基址 << 4 + 偏移地址
```

如 lab1 中 BIOS 执行的第一条指令,  `CS = 0xf000` `IP = 0xfff0` , 实际物理地址是 16 * 0xf000 + 0xfff0 = 0xffff0, 即  BIOS 从 0x000ffff0 开始执行.

#### 保护模式

随着CPU的发展, 地址总线从20位发展到 32位, 可以访问的内存空间从1MB 变为 4GB, 寄存器的位数变为32位. 所以实模式下的内存地址计算方式就不再适合了, 引入了保护模式, 实现更大空间的, 更灵活的内存访问.

在介绍保护模式的工作原理之前, 先清楚以下几个容易混淆的地址概念: 
逻辑地址(logical address), 虚拟地址(virtual address), 线性地址(linear address), 物理地址(physical address).

在编写程序时, 程序运行在虚拟地址空间下. 也就是说, 在编写程序时指令中出现的地址并不一定是这个程序在内存中运行时真正要访问的内存地址. 这样做的目的就是为了能够让程序员在编程时不需要直接操作真实地址, 因为当它在真实运行时, 内存中各个程序的分布情况是不可能在你编写程序时就知道的, 所以这个程序的这条指令到底要访问哪个内存单元是由操作系统来确定的, 这就是一个从虚拟地址(virtual address)到真实主存中的物理地址(physical address)的转换.

那么逻辑地址(logical address)又是什么呢? 根据上面一段文字, 编写程序时看到的是虚拟地址, 但是并不是是直接把这个虚拟地址写到指令中的, 它是由逻辑地址推导得到的, 所以指令中真实出现的是逻辑地址. 一个逻辑地址是由两部分组成的: 一个段选择子(segment selector), 一个段内偏移量(offset), 通常被写作 segment:offset. 而且采用哪个段选择子通常也是在指令中隐含的, 程序员通常只需要指明段内偏移量, 然后分段管理机构(segmentation hardware) 将会把这个逻辑地址转换为线性地址(linear address). 如果该机器没有采用分页机制(paging hardware)的话, 此时 linear address 就是最后的主存物理地址; 如果机器中还有分页设备的话, 比如内存大小实际只有1G, 但是根据前面我们知道可访问的空间有4G, 此时还需要分页机构(paging hardware) 把这个线性地址转换为最终的真实物理地址. 

即地址转换过程: segment:offset --> 逻辑地址---> 分段管理 ---> 虚拟地址/线性地址 ---> 分页机制 ---> 物理地址, 参考下图(来自 XV6 Appendix B). 

　　![img](https://images2015.cnblogs.com/blog/809277/201601/809277-20160109142207371-383459687.png)

在 boot loader中, 并没有开启分页, 所以计算出来的线性地址就是真实要访问的内存物理地址.

那么在保护模式下, 是如何通过 segment:offset 最终得到物理地址的呢?

首先, 在计算机中存在两个表: GDT(全局段描述符表), LDT(本地段描述符表). 他们都是用来存放关于某个运行在内存中的程序的分段信息的. 比如某个程序的代码段是从哪里开始, 有多大; 数据段又是从哪里开始, 有多大. GDT 表是全局可见的, 也就是说每一个运行在内存中的程序都能看到这个表, 所以操作系统内核程序的段信息就存在这里面. LDT 表是每一个在内存中的程序都包含的, 里面指明了每一个程序的段信息, 这两个表的结构如下图所示:

　　![img](https://images2015.cnblogs.com/blog/809277/201601/809277-20160109143519184-1430449279.png)

从图中可以看到, 无论是GDT, 还是LDT, 每一个表项都包括三个字段:

Base: 32位, 代表这个程序的这个段的基地址.
Limit: 20位, 代表这个程序的这个段的大小.
Flags: 12位, 代表这个程序的这个段的访问权限.

当程序中给出逻辑地址 segment:offset时, 他并不是像实模式那样, 用 segment 的值作为段基址, 而是把这个segment 的值作为一个 selector, 代表这个段的段表项在 GDT/LDT 表的索引. 比如你当前要访问的地址是segment:offset = 0x01:0x0000ffff, 此时由于每个段表项的长度为8, 此时应该取出地址8处的段表项. 然后首先根据Flags 字段来判断是否可以访问这个段的内容, 这样做是为了能够实现进程间地址的保护. 如果能访问, 则把Base 字段的内容取出, 直接与offset 相加, 得到线性地址(linear address)了. 之后就是要根据是否有分页机构来进行地址转换了. 比如当前Base 字段的值是0x00f0000, 则最后线性地址的值为 0x00f0ffff.

如上所述就是保护模式下内存地址的计算方法.

综上, 保护模式比实模式的工作方式灵活许多:

1. 实模式下段基地址必须是16的整数倍，保护模式下段基地址可以是4GB空间内的任意一个地址.
2. 实模式下段的长度是65536B, 但是保护模式下段的长度也是可以达到4GB的.
3. 保护模式下可以对内存的访问多加一层保护，但是实模式没有.

## Exercise 3

> 在 0x7c00 设置一个断点, 引导扇区将在这里加载. 继续执行直到该断点, 跟踪 `boot/boot.S` 代码, 使用源代码和反汇编文件 `obj/boot/boot.asm` 保持跟踪的位置. 还可以使用 x/i 命令在引导加载程序中对指令序列进行反汇编, 并将原始的引导加载程序源代码与 `obj/boot/boot.asm` 中的反汇编和 GDB 进行比较.
>

在一个 terminal 中输入 make qemu-gdb, 启动 QEMU, 在另一个 terminal 中运行 make gdb. 

`b *0x7c00` 在 0x7C00 地址设置了一个断点, 使用 `c` 命令继续执行到断点, 此时 QEMU 终端上显示 "Booting from Hard Disk...", 使用 `x/10i` 命令对 0x7c00 处的后续10条指令进行反汇编, 使用  `si` 命令继续下一条指令.

![BootLoader_0x7c00](images/BootLoader_0x7c00.JPG)

对比反汇编文件  `obj/boot/boot.asm` 和源文件  `boot/boot.S` 

`obj/boot/boot.asm` 0x7c00 及后续10条指令

![BootLoader_0x7c00_1](images/BootLoader_0x7c00_1.JPG)

  `boot/boot.S` 0x7c00 及后续10条指令

![BootLoader_0x7c00_2](images/BootLoader_0x7c00_2.JPG)



> 跟踪进入 `boot/main.c bootmain()`, 然后进入 `readsect()`. 识别出 `readsect()` 中每个语句对应的汇编指令, 跟踪 `readsect()` 的其余部分并返回到 `bootmain()`, 并标识从磁盘读取内核剩余扇区的 `for` 循环的开始和结束, 找出循环结束时将运行什么代码, 在那里设置一个断点, 然后继续运行到断点, 然后逐步完成引导加载程序的其余部分.

## Loading the Kernel

读取硬盘上的 kernel image 到内存中并运行. 代码位于函数 `bootmain()` 中, 如 lab1 所说, 要弄清 `boot/main.c` 的输出则要知道 ELF 二进制文件是什么, kernel image 是以 ELF 形式组织的.

```
/**********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(boot.S and main.c) is the bootloader.  It should
 *    be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in boot.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 **********************************************************************/
```

硬盘上的第一个扇区(扇区 0) 存储的是 bootloader, 第二个扇区是 kernel image, 每个扇区大小 512 byptes, 加载到内存都需按照 512 bytes 对齐. 

`readseg((uint32_t) ELFHDR, SECTSIZE*8, 0);` 
	---> `offset = (offset / SECTSIZE) + 1;` 
	---> `readsect((uint8_t*) pa, offset);`

读取4K 数据, 包括ELF头部和程序头表到物理地址 0x10000, offset =1 表示从扇区1开始, `readsect()`是以扇区为读取.

	#define SECTSIZE	512
	#define ELFHDR		((struct Elf *) 0x10000) // scratch space
	
		// read 1st page off disk
		readseg((uint32_t) ELFHDR, SECTSIZE*8, 0);
	
		// is this a valid ELF?
		if (ELFHDR->e_magic != ELF_MAGIC)
			goto bad;
	
		// load each program segment (ignores ph flags)
		// 通过 e_phoff 找到第一个段, 以及段的个数 e_phnum
		ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
		eph = ph + ELFHDR->e_phnum;
		for (; ph < eph; ph++)
			// p_pa is the load address of this segment (as well
			// as the physical address)
			// p_offset 开始之后的段的 p_memsz 个内存数据读取到物理地址 p_pa
			readseg(ph->p_pa, ph->p_memsz, ph->p_offset);
			
		((void (*)(void)) (ELFHDR->e_entry))();
		
	===================================================================================
	
	// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
	void
	readseg(uint32_t pa, uint32_t count, uint32_t offset)
	{
		while (pa < end_pa) {
			// Since we haven't enabled paging yet and we're using
			// an identity segment mapping (see boot.S), we can
			// use physical addresses directly.  This won't be the
			// case once JOS enables the MMU.
			readsect((uint8_t*) pa, offset);
			pa += SECTSIZE;
			offset++;
	}
`ELFHDR`: ELF Header 结构, `Proghdr`: 是 program header 结构.

`e_phoff`: program header table(程序头表第一项)相对于 ELF 文件开始位置的偏移, 则 `ph` 为第一项程序头表的地址, 保存了第一个段(segment)的信息(地址, 大小等).

`e_phnum`: program header table 中表项个数, 即段的个数, 则 `eph` 为最后一个段的信息.

`ph->p_pa`: program header 中的 p_vaddr, 表示 segment 的第一个字节在内存中的物理地址.

`ph->p_memsz`: program header 中的 p_memsz, 表示 segment 的内存映像大小.

`ph->p_offset`: program header 中的 p_offset, ELF 文件开始位置到该 segment 第一个字节的偏移量.

`readseg()`: 从 `ph->p_offset` 读取 `ph->p_memsz` 个字节到 `ph->p_pa`.

```
struct Elf {
	uint32_t e_magic;	// must equal ELF_MAGIC
	uint8_t e_elf[12];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct Proghdr {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_va;
	uint32_t p_pa;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};
```

通过 ELF Header 和 Program Header 找到文件的第 i 个段地址的方法如下:

```undefined
第 i 段程序头表表项位置 = 文件起始位置 + 程序头表偏移e_phoff + i*程序头表项字节数 
第 i 段地址就是第i个程序头表表项的 p_offset 值
```

下面是根据 kernel ELF header 和 Program header 信息画出的内存 mapping. 

Program header table1 相对 ELF文件起始地址偏移 e_phoff(52 bytes), 对应 segment 1, 其包含 .text .rodata .stab .stabstr 等 section, 加载到内存物理地址 0x100000, 占用 p_memsz (0x0716c bytes) 空间.

![](images/Kernal_image_mapping.JPG)

### ELF & Program Header

关于 ELF Header 和 Program Header 的详细字段解释及更多可参考 ELF.md.

#### Boot Loader

入口地址是: Entry point address:               0x7c00

```
hongssun@hongssun-user:~/workspace/6.828/lab/obj/boot$ readelf -h boot.out
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Intel 80386
  Version:                           0x1
  Entry point address:               0x7c00
  Start of program headers:          52 (bytes into file)
  Start of section headers:          4868 (bytes into file)
  Flags:                             0x0
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         2
  Size of section headers:           40 (bytes)
  Number of section headers:         9
  Section header string table index: 6
hongssun@hongssun-user:~/workspace/6.828/lab/obj/boot$ readelf -l boot.out

Elf file type is EXEC (Executable file)
Entry point 0x7c00
There are 2 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x000074 0x00007c00 0x00007c00 0x0022c 0x0022c RWE 0x4
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10

 Section to Segment mapping:
  Segment Sections...
   00     .text .eh_frame 
   01     
```

#### Kernel

入口地址是: Entry point address:               0x10000c

```
hongssun@hongssun-user:~/workspace/6.828/lab/obj/kern$ git branch
* lab1
hongssun@hongssun-user:~/workspace/6.828/lab/obj/kern$ readelf -h kernel
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Intel 80386
  Version:                           0x1
  Entry point address:               0x10000c
  Start of program headers:          52 (bytes into file)
  Start of section headers:          78712 (bytes into file)
  Flags:                             0x0
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         3
  Size of section headers:           40 (bytes)
  Number of section headers:         11
  Section header string table index: 8
hongssun@hongssun-user:~/workspace/6.828/lab/obj/kern$ 
hongssun@hongssun-user:~/workspace/6.828/lab/obj/kern$ readelf -l kernel

Elf file type is EXEC (Executable file)
Entry point 0x10000c
There are 3 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0xf0100000 0x00100000 0x0716c 0x0716c R E 0x1000
  LOAD           0x009000 0xf0108000 0x00108000 0x0a300 0x0a944 RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10

 Section to Segment mapping:
  Segment Sections...
   00     .text .rodata .stab .stabstr 
   01     .data .bss 
   02     
```

## 几个问题的答案

- 处理器从何时开始执行 32 位代码, 是什么导致从16位模式切换到32位模式?

  代码如下:

  ```
  # Switch from real to protected mode, using a bootstrap GDT
  # and segment translation that makes virtual addresses 
  # identical to their physical addresses, so that the 
  # effective memory map does not change during the switch.
    lgdt    gdtdesc
    movl    %cr0, %eax
    orl     $CR0_PE_ON, %eax
    movl    %eax, %cr0
  ```

  CR0 寄存器的 bit0 是保护模式启动位, 把这一位值1代表保护模式启动.

  ```
    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp    $PROT_MODE_CSEG, $protcseg
  ```

  跳转指令, 把当前的运行模式切换成32位地址模式.

  详细分析过程参考: https://www.cnblogs.com/fatsheep9146/p/5115086.html

  

- 引导加载程序执行的最后一条指令是什么? 加载内核的第一条指令是什么?

  Boot Loader 的最后一条指令是跳转到 Kernel 的入口(ELF header 中的入口地址).

  ```
  // call the entry point from the ELF header
  // note: does not return!
  ((void (*)(void)) (ELFHDR->e_entry))();
  ```

   对应于`obj/boot/boot.asm` 中的汇编代码:

  ```
  	((void (*)(void)) (ELFHDR->e_entry))();
      7d61:	ff 15 18 00 01 00    	call   *0x10018
  ```

  即跳转到 0x10018 内存地址所存储的值处运行, 而该地址存储的内容是 0x10000C, 即 Kernel 的入口地址.

  注意:

  ```
  此时位于保护模式下, Kernel 入口地址 0x10000C 经过全局描述符表(GDT)生成线性地址, 又因还没有开启分页, 线性地址就等于物理地址, 所以 kernel 代码本身就加载在内存物理地址 0x10000C.
  ```

- 内核的第一条指令在哪?

  根据 kernel 的入口文件 kern/entry.S, 第一条指令如下, 由注释可知, 此时虚拟地址还没有设置, lootloader 跳转到 entry point 的物理地址即 0x10000C 执行.

  ```
  # '_start' specifies the ELF entry point.  Since we haven't set up
  # virtual memory when the bootloader enters this code, we need the
  # bootloader to jump to the *physical* address of the entry point.
  .globl		_start
  _start = RELOC(entry)
  ```

- 引导加载程序如何决定必须读取多少个扇区才能从磁盘获取整个内核? 它从哪里找到这些信息?

  从 ELF Header 获取, 由 e_phoff 知道第一个段的位置, 由 e_phnum 可以知道需要加载几个段.
  
  

## 参考

https://www.cnblogs.com/fatsheep9146/p/5116426.html

https://www.jianshu.com/p/af9d7eee635e

[PC Assembly Language](https://pdos.csail.mit.edu/6.828/2017/readings/pcasm-book.pdf) 
