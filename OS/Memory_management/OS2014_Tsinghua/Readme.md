# 内存管理

## 参考资料

操作系统原理(清华)

视频: [清华 操作系统原理_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1uW411f72n?p=19)

课件: 
[OS2014 - OscourseWiki (tsinghua.edu.cn)](http://os.cs.tsinghua.edu.cn/oscourse/OS2014#Course_Introduction) 
[OS2018spring - OscourseWiki (tsinghua.edu.cn)](http://os.cs.tsinghua.edu.cn/oscourse/OS2018spring) 
[OS2020spring - OscourseWiki (tsinghua.edu.cn)](http://os.cs.tsinghua.edu.cn/oscourse/OS2020spring) [Releases · dramforever/os-lectures-build · GitHub](https://github.com/dramforever/os-lectures-build/releases) 
PS. 2014 年的课程章节与视频比较匹配, 2018 课件内容与视频更匹配, 2020 年课件最新, 包含一些 RISC-V 的内容.

其他课件参考:

https://github.com/computer-system-education/os-syllabi/tree/master/material

https://github.com/LearningOS

学习笔记: 
[GitHub - OXygenPanda/3.OperatingSystem_in_depth: 视频学习 - 深入理解操作系统](https://github.com/OXygenPanda/3.OperatingSystem_in_depth)

[GitHub - kirklin/PrivateNotes: 个人学习笔记，包含了计算机科学笔记，前端笔记，后端笔记](https://github.com/kirklin/PrivateNotes)

[物理内存管理：非连续内存分配 - Kirk Lin的个人学习笔记](https://kirklin.github.io/PrivateNotes/计算机科学/计算机操作系统/物理内存管理：非连续内存分配/)

[【操作系统】 Operation System 第四章：非连续式内存分配_iwanderu的博客-CSDN博客](https://blog.csdn.net/iwanderu/article/details/103946219)

## 计算机体系结构

计算机基本硬件结构：CPU, 内存, 外设.  程序执行在 CPU, 执行时的程序代码和数据保存在内存, 键盘鼠标等输入输出设备作为外设. 

![](images/Computer_HW_Arch_0.JPG)



其中, CPU 由运算器(ALU), 控制器(CU), 寄存器(Register), 缓存(Cache), 存储管理单元(MMU) 等组成.

内存也可以扩展为广义上的存储器, 包括主存(DRAM), Flash, 硬盘等其他存储设备.

![](images/Computer_HW_Arch_1.JPG)

## 存储器分层体系

现代计算机系统将存储器以金字塔的形式分层管理(Memory Hierarchy). 实际就是 CPU执行的指令和数据所处的不同层次的位置. 如下图所示. 

![](images/Memory_Hierarchy_0.JPG)

自上而下包括: 寄存器, L1/L2/L3 Cache, 主存(物理内存), 本地磁盘(硬盘), 远程存储.  从高到低存储设备的容量越来越大, 硬件成本越来越低, 但是访问速度越来越慢. 另外还有一种非易失性存储器, 如 flash 和 SSD, 其访问速度介于 主存和存盘之间, 相对于传统的磁盘, 它更快速, 更低能耗.

下图是不同层次存储器的访问时间, 以及 CPU 的速率和存储器访问时间的差异变化.

![](images/Memory_Hierarchy_1.JPG)

![](images/Memory_Hierarchy_2.JPG)

## 关于内存的其他主题

关于内存的一些其他主题: 局部性, 内存与缓存一致性, 虚拟内存等.

[深入理解计算机系统合集（周更中）_哔哩哔哩 (゜-゜)つロ 干杯~-bilibili](https://www.bilibili.com/video/BV17K4y1N7Q2?p=27)

[GitHub - yangminz/bcst_csapp: A C language repo to implement CSAPP](https://github.com/yangminz/bcst_csapp/)

## 内存管理目标

现代计算机系统的内存面临几个问题. 一是 CPU 的速率和存储器的访问速率差距越来越大, 这就引入了缓存; 二是运行时的数据都放在内存上, 现代计算机运行的程序越来越多, 对内存容量的需求越来越大, 这就引入了硬盘; 三是内存一旦掉电后数据不能保存, 所以代码和数据需要永久保存在硬盘. 

这些都需要操作系统的内存管理工作, 其理想是既能快速地访问存储器, 存储器的容量也能最大.

4个目标:

* 抽象: 程序在内存中运行时不需要考虑底层细节. 比如物理内存在什么地方, 外设在什么地方, 只需访问一段地址空间就可以了. 这个地址空间称为: 逻辑地址空间.
* 保护: 内存中可能运行多个不同的应用进程, 相互之间有可能会破坏对方地址空间上的数据, 需要一种机制来保护/隔离进程的数据: 独立地址空间.
*  共享: 进程之间也可能需要互相交互, 需要共享空间使得进程间可以安全、可靠、有效地进行数据传递. 需要一种进程间的数据传递机制: 访问相同内存.
* 虚拟化: 大量的应用程序会导致主存容量不够, 如何让正在运行的进程获得他需要的内存空间. 这就需要把正在使用的数据放在内存中, 暂时不适用的数据临时地放到硬盘中, 通过这种方式实现虚拟的, 更大的可用内存. 这个过程对应用程序不可见, 满足进程的数据空间需求.

![](images/Memory_Management_Target.JPG)

## 内存管理方法

要实现上述目标, 方法有:

* 程序重定位(program relocation)
* 分段 (segmentation)
* 分页 (paging) , 目前多数系统如 Linux采用按需分页虚拟内存

同时操作系统实现内存管理也高度依赖硬件支持:

* 与计算机存储架构紧耦合
* MMU(内存管理单元): 处理 CPU 的内存访问请求 

## 地址空间

### 定义

* 物理地址空间 --- 硬件支持的地址空间, 比如物理内存的地址空间.
* 逻辑地址空间 --- 一个运行的程序所拥有的内存范围.

如下图所示, 物理地址空间是0 ~ **MAX**sys, 逻辑地址空间是 0~ **MAX**prog. 那么程序中的某个指令的地址是如何生成的呢?

![](images/Address_Space.JPG)

### 逻辑地址生成

如下图所示: 逻辑地址的生成由编译器和加载器完成, 其过程为: 编译, 汇编, 链接, 加载(程序重定位).

![](images/Address_Logical.JPG)

* 编译(Compilation): c 程序中的函数、变量都是逻辑地址的体现, 他以一种人更容易理解的方式存在. 经过编译之后生成 .s 程序, 用符号代表函数和变量
* 汇编(Assembly): s 汇编程序更接近机器语言, 经过汇编器转换成机器语言 .o 程序. 变量和函数的符号被转换成地址, 是从0开始的相对地址.
* 链接(Linking): .o 程序之间会互相依赖互相访问, 链接器将多个 .o 链接成一个可执行程序, 对所有.o 程序里的地址作了全局分布, 生成全局地址. 但是这个地址依然是在程序中的相对地址.
* 加载(Loading): 加载器将程序加载到内存中运行. 通过程序重定位生成内存中运行的地址, 其相对于程序中的地址有个偏移量. Linux 系统的程序加载通过 exec 函数族实现.

具体过程可参考: [C编译器、链接器、加载器详解 - LiuYanYGZ - 博客园 (cnblogs.com)](https://www.cnblogs.com/LiuYanYGZ/p/5574601.html)

逻辑地址的生成时机.

* 编译时, 如果起始地址已知, 也就是说知道程序放在内存中的什么位置, 那么编译时就可以生成逻辑地址.

* 加载时, 大多数情况下起始地址是未知的, 这就需要编译器生成可重定位的代码, 加载时重定位, 生成绝对地址. ELF 格式的目标文件中都包含可重定位信息, 比如 .text 的重定位信息存储在.rel.text 中. 链接器在处理目标文件时对相应的 .text 重定位. ELF 格式的可执行文件中也有一个重定位表, 加载时改成绝对地址就可以运行了.
* 执行时, 执行到这条指令之前一直使用相对地址, 当执行到这一条指令的时候, 才去获取它确切访问的地址. 这种情况出现在使用虚拟存储的系统里, 优点是程序执行过程中就可以将代码移动, 而前面两种生成时机都不可以. 但是需要地址转化硬件支持, 相对较麻烦.

### 逻辑地址处理

应用程序的逻辑地址是如何映射到物理地址.

* CPU
  * 运算器 ALU 需要在逻辑地址的内存内容
  * 内存管理单元 MMU 进行逻辑地址和物理地址的转换(映射)
  * CPU 控制器从总线发送在物理地址的内存内容的请求
* 内存
  * 内存发送物理地址内存内容给 CPU
* 操作系统
  * 建立逻辑地址和物理地址之间的映射(确保程序不相互干扰)

![](images/Address_Logical_Physical.JPG)

### 地址安全检查

![](images/Address_Check.JPG)

操作系统为每个进程设置了起始地址和范围, 当程序访问的地址空间处于这个范围内时, 则映射成对应的物理地址. 如果不在这个范围则产生内存异常.

## 连续内存分配

...

## 非连续内存分配

### 为什么

分配给一个程序连续的物理内存不可避免地存在一些问题

* 内碎片、外碎片问题
* 内存利用率较低

针对这些问题, 通过非连续内存分配来解决. 分配给一个程序的物理内存是非连续的, 它的优点

* 更好的内存利用和管理
* 允许共享代码、数据、库等
* 支持动态加载和动态链接

非连续内存分配的缺点就是它本身的开销, 如何建立虚拟地址和物理地址之间的转换

* 软件方案(开销大)
* 硬件方案(分段, 分页)

### 分段 (Segmentation)

段式存储管理的目的是能够实现更细粒度和灵活的分离和共享, 其需要关注两个问题

* 程序/进程的分段地址空间
* 分段如何寻址

进程的地址空间由多个段组成:

* 主程序代码段(Main program)
* 子程序代码段(Subroutines)
* 公用库代码段
* 数据(data)
* 堆(heap)
* 栈(stack)
* 符号表(Symbols)
* 等...

![](images/Segmentation_Address.JPG)

#### 分段地址空间

进程运行时, 按照程序自身的逻辑关系划分为若干个段, 通过对每个段不同的管理, 如访问权限等, 能够实现更好的分离与共享.

段的逻辑地址空间连续, 但是对应的物理地址空间是不连续的.

![](images/Segmentation_Address_Mapping.JPG)

#### 分段寻址方案

1. 段访问机制: 程序逻辑地址的访问需要一个二元组 (s 段号, addr 段内偏移), 二元组可以用段寄存器+地址寄存器实现(如 X86), 也可以用单寄存器地址实现.

   ![](images/Segmentation_Address_Access.JPG)

2. 段访问机制硬件实现:

   ![](images/Segmentation_Address_Access_HW.JPG)

   这里由操作系统将段表信息(段号, 物理内存起始地址, 长度) 写入寄存器, 用于程序运行时的地址映射.

### 分页 (Paging)

分段寻址中, 地址由段号+段内偏移组成, 类似地, 分页寻址中地址访问由页(帧)号+页内偏移组成.

不同的是, 分段中段的 size 是不同的, 可变的. 分页中, 页的 size 是固定的, 为 2^n, 比如 4096B, 8192B.

#### 分页地址空间

* 划分物理地址空间至固定大小的页帧, Page Frame

  大小是 2 的幂次方, 比如: 512B, 4096B, 8192B

* 划分逻辑地址空间至相同大小的页, Page

  大小和物理页帧一样, 比如: 512B, 4096B, 8192B

* 建立方案, 逻辑地址空间(逻辑页)与物理地址空间(物理页)之间的映射, pages to frames

  * 页表
  * MMU/TLB

#### 页帧 (Frame)

物理内存被划分成大小相等的帧. 一个物理地址表示为一个二元组(f, o)

* f : 页帧号, F 位, 共有 2^F 个页帧.
* o : 帧内偏移, S 位, 每帧有 2^S bytes.

Physical address = 2^S * f + o, 如下图所示.

<img src="images/Page_Address_Access.JPG" alt="Page_Address_Access" style="zoom: 67%;" />

举例, 16-bit 的地址空间, 9-bit (512 byte) 大小的页帧. 物理地址表示 = (3, 6)

物理地址 = 2^S * f + o = 2^9 * 3 + 6 = 1542 (F = 7, S = 9, f = 3, o= 6)

<img src="images/Page_Address_Access_1.JPG" alt="Page_Address_Access_1" style="zoom: 67%;" />

#### 页 (Page)

进程逻辑地址空间被划分为大小相等的页, 一个逻辑地址表示为一个二元组(p, o)

* p : 页号(P 位, 一共 2^P 个页). 页号大小一般 != 帧号大小
* o : 页内偏移(S 位, 每页有 2^S bytes).  页内偏移 == 帧内偏移

Virtual address = 2^S * p + o, 如下图所示.

<img src="images/Page_Address_Access_2.JPG" alt="Page_Address_Access_2" style="zoom: 67%;" />

#### 页寻址机制

* 逻辑页到物理页帧的映射
* 逻辑地址中的页号是连续的
* 物理地址中的帧号是不连续的
* 不是所有的逻辑页都有对应的物理页帧, 一般逻辑地址空间会大于物理地址空间

<img src="images/Page_Address_Access_3.JPG" alt="Page_Address_Access_3" style="zoom:67%;" />

页表由操作系统建立, 可以简单理解为一个数组, 页号是 index, 页帧号是 value.

CPU 寻址时根据页表基地址和页号获取该页号对应的页帧, 加上偏移地址, 获得要访问的物理地址.

#### 页表

##### 概述

分页机制是基于页表实现的. 操作系统为每一个运行的程序(进程)建立一张页表, 进程的每个页面在内存中存放的位置都是知道的.

* 一个进程对应一张页表, 页表随进程运行状态的改变而动态变化
* 进程的每个页面对应一个页表项, 页表项的长度是相同的, 由标志位和页帧号组成
  * 标志位用来表示当前页的属性: 存在位(resident bit)、修改位(dirty bit)、引用位(clock/reference bit)
* 页表记录进程页面和实际存放的物理内存之间的映射关系
* 页表基址寄存器 (PTBR: Page Table Base Register)

<img src="images/Page_Table.JPG" alt="Page_Table" style="zoom: 50%;" />

##### 地址转换

16bit 地址的计算机系统,逻辑地址空间是 2^16, 64 kB. 物理内存大小 2^15, 32 kB. 每页大小 2^10, 1024 Byte.
如下图所示. 其中逻辑地址 16bit 表示, 0~9bit 表示页内偏移 o, 10~15bit 表示页号 p; 物理地址 15bit 表示, 0~9bit 表示页内偏移 o, 9~14bit 表示页帧号 f.

<img src="images/Page_Table_1.JPG" alt="Page_Table_1" style="zoom:50%;" />

* 逻辑地址 (4,0): 页号 4对应第五个页表项(从下往上数): 10000000, 其中标志位是: 100, resident bit 是 0, 说明逻辑地址(4,0) 在实际物理地址中是不存在的. 如果 CPU访问这个逻辑地址会抛出一个内存访问缺页异常.
* 逻辑地址 (3,1023): 页号 3对应的标志位是 011, dirty bit: 0、resident bit: 1、clock/reference: 1. 可知逻辑地址 (3,1023) 在物理地址中存在. 其对应的页帧号是 4, 再加上偏移量 1023, 所以逻辑地址(3,1023) 对应的物理地址是 (4,1023).

#### 分页机制的性能问题

##### 时间开销

访问一个内存单元需要 2 次内存访问.

* 第一次访问: 访问页表项, 获取逻辑地址对应的物理地址.
* 第二次访问: 访问物理内存上的数据.

##### 空间开销:

页表本身可能会非常大. 比如 64位机器, 如果每页 1024 Byte, 那么页表项有 2^64/2^10 = 2^54 个. 并且一个进程对应一个页表, n 个进程就是 n 个页表.

这么大的页表不可能放到 CPU 中, 如果存储在内存中那么也会导致 2次内存访问的效率很低.

那么如何解决这两个问题呢:

1. 缓存 (Caching)
2. 间接 (Indirection) 访问

#### 转换后备缓冲区 (Translation Look-aside Buffer)

缓存是用来解决分页机制下页表的时间开销问题.

TLB 是CPU 的 MMU 内存管理单元中的一段缓存, 这段缓存保存的内容是最近访问的页表项, 是经常被访问的页表项, 其余不常用的页表项则保存在内存中.

* TLB 使用关联存储 (associative memory) 实现, 具备快速访问性能
* 如果 TLB 命中, 虚拟地址对应的物理页号可以很快被获取
* 如果 TLB 未命中, 对应的表项被更新到 TLB 中

<img src="images/Page_Table_TLB.JPG" alt="Page_Table_TLB" style="zoom:50%;" />

当访问的物理页表项存在于 TLB 时, 地址转换的过程如下:

1. CPU 给出逻辑地址 p + o, MMU 硬件根据页号 p 查表
2. 找到 p 对应的页表项, 说明要访问的页表项在 TLB 中有副本, 直接取出对应的物理页帧号 f, 加上页内偏移 o, 得到物理地址. 因此有了 TLB, 仅需一次内存访问.

TLB 空间有限, 当访问的物理页表项不存在 TLB 时, 则不得不访问内存中的页表, 找到对应页表项,得到对应的物理页帧号 f, 再与页内偏移量相加得到物理地址, 最后访问该物理地址对应的内存单元. 在找到页表项后, 会同时将 p->f 的页表更新到 TLB 中 (由硬件或者操作系统完整, 取决于不同的CPU硬件实现, X86 由硬件实现, MIPS 由操作系统完成).

未找到 TLB 页表项称为 TLB miss, 这种情况比较少见, 因为如果程序采用局部性原理, 32位系统一页是4K,那么访问 4k 次才会遇到一次 TLB miss. 所以局部性对应 TLB 能否命中非常关键.

##### 局部性原理

* 时间局部性

  如果执行了程序中的某个指令, 那么不久后这条指令很有可能再次被执行; 如果某个数据被访问过,不久之后该数据很可能再次被访问 (因为程序中存在大量的循环).

* 空间局部性

  一旦程序访问了某个存储单元, 在不久之后, 其附近的存储单元也很有可能被访问 (因为很多数据在内存中都是连续存放的).

#### 二级/多级页表

多级页表解决存储页表的空间开销问题. 虽然增加了内存访问次数和开销, 但时间换空间, 节省了保存页表的空间, 然后通过 TLB 来减少时间消耗.

通过间接引用将页号 p 分割成多级:

* 减少每级页表的长度
* 建立页表树形结构

<img src="images/Page_Table_Multi_Level.JPG" alt="Page_Table_Multi_Level" style="zoom:50%;" />

##### 二级页表实现

以 X86 32位系统, 4K 页面大小为例, 其虚拟地址格式如下:

<img src="images/Page_Table_Two_Level.JPG" alt="Page_Table_Two_Level" style="zoom:50%;" />

22~31: 10bits, 页目录, 一级页表, 存放的是每个二级页表的起始地址
12~21: 10bits, 页表项, 二级页表, 存放对应的物理页帧号 f.
0~11: 12bits, 页内偏移 offset

地址翻译过程如下:

* 逻辑地址中的页号分割成 p1 和 p2
* 寄存器中保存了第一级页表的地址, 加上 p1, 获取页表项, 得到对应的二级页表的**起始地址**
* 二级页表的起始地址, 加上 p2, 获取页表项, 得到对应的页帧号 f
* 页帧 f, 加上 o, 得到物理地址. 

<img src="images/Page_Table_Two_Level_Addressing_2.JPG" alt="Page_Table_Two_Level_Addressing_2" style="zoom:50%;" />

<img src="images/Page_Table_Two_Level_Addressing.JPG" alt="Page_Table_Two_Level_Addressing" style="zoom:50%;" />

在第一级页表中, 如果页表项对应的存在位为 0, 则整个二级页表都不用在内存中保存, 这个在单级页表中是无法实现的, 因为不管存不存在映射关系都需要保存该页表项, 因此多级页表能够节省保存页表的空间. 

