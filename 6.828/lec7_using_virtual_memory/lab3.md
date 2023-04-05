# Lab 3: User Environments(Process)

## 预习准备

lab3A-elf.md

给操作系统捋条线.pdf

### ELF 文件结构

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
```

ELF 文件开头放置的是一系列`header`，包括整个 ELF的`header`, 以及后面一个个区块的`header`，这些`header`不直接包含和运行有关的数据, 只包含**元数据**, 帮助我们更好加载真正的数据.

ELF `header` 中指定了每个区块`segment`的位置, 以及`segment`的长度, 从而让我们方便地找到并索引.

这些区块`segment`是编译器和连接器定义的, 每个都有各自的含义. 如`.text`为代码区, 包含要执行的指令; `.data`代表已经初始化的全局变量; `.bss`代表未初始化的全局变量, 等等. 要让进程正确执行, 必须把这些区块加载到正确的内存空间上.

### ELF 文件加载

该 Lab中并没有加载真的可执行文件, 因为还没有文件系统. 所使用的 ELF文件是通过链接器嵌入到内核这个可执行文件中的.

ELF 文件有多个`ProgHdr`, 即可执行文件中的 segment(段), 每个  segment对应一个 program header, 每个 segment 都有指定好的虚拟地址和长度, 只需要从 `ELF`中读取出这些信息, 把数据拷贝到相应位置就可以. 下面是一个可执行文件的例子.

```
$ readelf -l stack

Elf file type is EXEC (Executable file)
Entry point 0x10318
There are 8 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  EXIDX          0x000574 0x00010574 0x00010574 0x00008 0x00008 R   0x4
  PHDR           0x000034 0x00010034 0x00010034 0x00100 0x00100 R E 0x4
  INTERP         0x000134 0x00010134 0x00010134 0x00013 0x00013 R   0x1
      [Requesting program interpreter: /lib/ld-linux.so.3]
  LOAD           0x000000 0x00010000 0x00010000 0x00580 0x00580 R E 0x10000
  LOAD           0x000580 0x00020580 0x00020580 0x00120 0x00124 RW  0x10000
  DYNAMIC        0x00058c 0x0002058c 0x0002058c 0x000f0 0x000f0 RW  0x4
  NOTE           0x000148 0x00010148 0x00010148 0x00044 0x00044 R   0x4
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10
```

有内存空间才能执行指令, 在拷贝 ELF文件中的各个部分到指定位置之前, 需要通过已有的分配器给进程足够多的`page`, 用于存放 ELF文件镜像. 并且光是 ELF文件中的内容还不够, 进程执行还需要栈, 要给进程栈分配空间, 并映射到指定的虚拟地址.

Exercise 2 中`load_icode()`函数的任务是把内核代码中的一个`ELF`镜像加载到指定的地址. 需要注意以下几点:

-   加载之前首先验证`e_magic`是否正确
-   在拷贝 program segment之前, 先将地址映射切换为当前进程的, 而不是继续使用内核的. `x86`可以非常方便地使用 lcr3()函数切换`Page Directory`.
-   ELF 中的虚拟地址指的是用户地址空间, 而不是当前采用的内核地址空间, 需要把指定的用户地址空间换算为内核地址空间, 再进行拷贝.

### XV6 进程管理

Exercise 2 还涉及到了用户进程(环境)的创建及运行(进程切换), env_create(), env_run().

进程 = 程序 + 运行时状态

* 程序: 存储器上的一个可执行文件
* 进程就是程序在内存中的一个执行实例, 即运行时的程序

#### 进程控制块

可执行文件由 ELF文件结构记录管理着文件的信息, 当可执行文件被加载到内存当作进程执行后, 也有类似的数据结构来记录管理进程的执行情况, 称为进程控制块(Process Control Block). PCB中记录了进程运行需要的一切环境和信息, xv6 的 PCB定义如下:

```
[proc.h]
struct proc {
	uint sz; 					// Size of process memory (bytes)进程大小
	pde_t* pgdir; 				// Page table 页表
	char *kstack; 				// Bottom of kernel stack for this process 内核栈位置
	enum procstate state; 		// Process state 程序状态
	int pid; 					// Process ID 进程ID
	struct proc *parent; 		// Parent process 父进程指针
	struct trapframe *tf; 		// Trap frame for current syscall 中断栈帧指针
	struct context *context; 	// swtch() here to run process 上下文指针
	void *chan; 				// If non-zero, sleeping on chan 用来睡眠
	int killed; 				// If non-zero, have been killed 是否被killed
	struct file *ofile[NOFILE]; // Open files 打开文件描述符表
	struct inode *cwd; 			// Current directory 当前工作路径
	char name[16]; 				// Process name (debugging) 进程名字
}
```

每个进程对应一个进程控制块/结构体, xv6 最多支持 64个进程, 所有进程结构体集合在一起形成了进程结构体表, 创建进程的时候寻找空闲的结构体分配出去, 进程退出时再回收.

```
[param.h]
#define NPROC 64 // maximum number of processes

[proc.c]
struct {
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;
```

进程结构体表是一个全局的数据, 配了一把锁给它, 锁主要用来保护进程的状态和上下文.



#### 进程运行状态

#### 任务状态段

#### 进程切换

## Introduction

在本 lab中, 你将实现基本的内核功能, 以使受保护的用户模式环境(即“进程”)运行. 你将增强 JOS内核, 设置数据结构来跟踪用户环境, 创建单个用户环境, 将程序映像加载到其中并启动它. 还将使 JOS内核能够处理用户环境发出的任何系统调用(system calls)和处理它引起的任何其他异常(exceptions handling).

注: 本 lab中环境 "environments"是进程 "process"的同义词. 两者都是指 "允许你运行程序的抽象“. 我们引入术语“环境”而不是传统的术语“进程”, 是为了强调 JOS环境和 UNIX进程提供不同的接口, 并且提供不同的语义.

### Getting Started

```
git checkout -b lab3 origin/lab3
git merge lab2
```

Lab3 包含了一些新的源文件, 应该浏览一下:

| `inc/`  | `env.h`       | Public definitions for user-mode environments, 用户环境(进程)公共定义 |
| ------- | ------------- | ------------------------------------------------------------ |
|         | `trap.h`      | Public definitions for trap handling, 陷入(用户态与内核态的切换)处理的公共定义 |
|         | `syscall.h`   | Public definitions for system calls from user environments to the kernel |
|         | `lib.h`       | Public definitions for the user-mode support library         |
| `kern/` | `env.h`       | Kernel-private definitions for user-mode environments        |
|         | `env.c`       | Kernel code implementing user-mode environments              |
|         | `trap.h`      | Kernel-private trap handling definitions                     |
|         | `trap.c`      | Trap handling code, 陷入处理的代码实现                       |
|         | `trapentry.S` | Assembly-language trap handler entry-points, 陷入处理程序入口点汇编语言 |
|         | `syscall.h`   | Kernel-private definitions for system call handling          |
|         | `syscall.c`   | System call implementation code                              |
| `lib/`  | `Makefrag`    | Makefile fragment to build user-mode library, `obj/lib/libjos.a` |
|         | `entry.S`     | Assembly-language entry-point for user environments          |
|         | `libmain.c`   | User-mode library setup code called from `entry.S`           |
|         | `syscall.c`   | User-mode system call stub functions                         |
|         | `console.c`   | User-mode implementations of `putchar` and `getchar`, providing console I/O |
|         | `exit.c`      | User-mode implementation of `exit`                           |
|         | `panic.c`     | User-mode implementation of `panic`                          |
| `user/` | `*`           | Various test programs to check kernel lab 3 code             |

此外, lab2 的一些源文件在 lab3中被修改了. 

你可能还想再看一看 [lab tools guide](https://pdos.csail.mit.edu/6.828/2017/labguide.html),它包含了调试与本 lab相关的用户代码的信息.

### Lab Requirements

略

### Inline Assembly

在本 lab中你可能会发现 GCC的内联汇编语言特性很有用, 不过也可以不使用它来完成实验. 至少你需要能够理解我们提供的源代码中已经存在的内联汇编语言片段(“asm”语句). 你可以在[reference materials](https://pdos.csail.mit.edu/6.828/2017/reference.html) 找到几个关于 GCC内联汇编语言的信息来源.

## Part A: User Environments and Exception Handling

新的文件 `inc/env.h` 包含 JOS中用户环境的基本定义. 内核使用 Env数据结构来跟踪每个用户环境. 在 lab中你最初将只创建一个环境, 但需要设计 JOS内核以支持多个环境. Lab4 将利用这个特性, 允许用户环境 fork其他环境.

正如你在 `kern/envirv.c`中看到的, 内核维护了三个与环境相关的主要全局变量:

```
struct Env *envs = NULL;		// All environments
struct Env *curenv = NULL;		// The current env
static struct Env *env_free_list;	// Free environment list
```

一旦 JOS启动并运行, `*env` 指针指向一个表示系统中所有环境(进程)的 Env结构数组. 在我们的设计中, JOS内核将支持同一时刻最多 NENV个活跃的环境, 尽管在任何给定时间通常会有更少的运行环境. (NENV 是在 `inc/env.h` 中 `#define` 的常量). 一旦分配了它，envs 数组将包含每个可能的 NENV 环境的 Env数据结构的单个实例.

JOS内核将所有未激活的 Env结构保存在 `env_free_list` 中, 这种设计允许轻松地分配和回收环境, 因为只需将它们添加到空闲列表中或从空闲列表中删除即可.

内核使用 `curenv` 符号在任何给定时间跟踪当前正在执行的环境. 在引导期间, 在第一个环境运行之前, `curenv` 最初被设置为 NULL.

### Environment State(进程状态)

`inc/env.h` 中的 `Env` 结构定义如下:

```
struct Env {
	struct Trapframe env_tf;	// Saved registers
	struct Env *env_link;		// Next free Env
	envid_t env_id;			// Unique environment identifier
	envid_t env_parent_id;		// env_id of this env's parent
	enum EnvType env_type;		// Indicates special system environments
	unsigned env_status;		// Status of the environment
	uint32_t env_runs;		// Number of times environment has run

	// Address space
	pde_t *env_pgdir;		// Kernel virtual address of page dir
};
```

**env_tf**: 在环境未运行时保存该环境的已保存寄存器值: 即当内核或其他环境正在运行时, 当从用户模式切换到内核模式时, 内核会保存这些信息, 以便稍后恢复中断的环境.

**env_link**: 指向 `env_free_list`上的下一个 Env. `env_free_list` 指向列表上的第一个空闲环境

**env_id**: 内核在这里存储一个值, 唯一标识当前使用 Env结构的环境 (即使用 `envs` 数组中的这个特定槽位). 在一个用户环境终止后, 内核可能会将相同的 Env结构重新分配到另一个环境中 —— 但是新环境的 `env_id` 将不同于旧环境, 即使新环境重用 `envs` 数组中的相同槽位

**env_parent_id**: 内核在这里存储创建该环境的环境的 `env_id`. 通过这种方式, 环境可以形成一个“家族树”, 这将有助于做出关于允许哪些环境对谁做什么事情的安全决策

**env_type**: 这是用来区分特殊环境的. 对于大多数环境, 它将是 `ENV_TYPE_USER`. 在后面的 lab中, 我们将为特殊的系统服务环境介绍更多的类型

**env_status**: 该变量包含以下值之一
		`ENV_FREE`: 指示 `Env` 结构处于非活动状态, 因此位于 `env_free_list` 上
		`ENV_RUNNABLE`: 指示 `Env` 结构表示正在等待运行的环境
		`ENV_RUNNING`: 指示 `Env` 结构表示当前正在运行的环境
		`ENV_NOT_RUNNABLE`: 指示 `Env` 结构表示当前活动的环境, 但它目前还没有准备好运行: 例如, 因为它正在等待来自另一个环境的进程间通信(IPC) --- 阻塞态
		`ENV_DYING`: 指示 `Env` 结构表示僵尸环境. 僵尸环境下次陷入内核时将被释放. 在 lab4之前我们不会使用这个标志

**env_pgdir**: 此变量保存此环境的页目录的内核虚拟地址

就像 Unix进程一样, JOS环境结合了“线程”和“地址空间”的概念. 线程主要由保存的寄存器( `env_tf`字段)定义, 地址空间由 `env_pgdir` 指向的页目录和页表定义. 要运行一个环境, 内核必须用保存的寄存器和适当的地址空间来设置CPU.

我们的结构`struct Env` 类似于 xv6中的结构`struct proc`. 这两个结构都在 `Trapframe` 结构中保存环境(即进程)的用户模式寄存器状态. 在 JOS中, 不同的环境不像 xv6中的进程那样有自己的内核堆栈, 内核中一次只能有一个活动的 JOS环境, 因此 JOS只需要一个内核堆栈.

### Allocating the Environments Array

在 lab2中你在 `mem_init()` 函数中分配了 `pages[]` 数组的地址空间, 用于记录内核中所有的页的信息. 现在你需要进一步去修改 `mem_init()` 函数, 来分配一个类似的 `Env` 结构体数组, 叫做 `envs`.

### Exercise 1

修改 `kern/pmap.c` 中的 `mem_init()` 来分配和映射 `envs` 数组. 这个数组由 `Env` 结构的 `NENV` 个实例组成, 就像分配 `pages` 数组的方式一样. 与 `pages` 数组一样.
`envs` 也应该映射到虚拟地址 `UENVS`处(在 `inc/memlayout.h` 中定义), 是用户模式只读的, 以便用户进程可以从该数组读取.

你应该运行代码并确保 `check_kern_pgdir()` 成功.

```
    // LAB 3: Your code here.
    envs = (struct Env*)boot_alloc(sizeof(struct Env*) * NENV);
    memset(envs, 0, sizeof(struct Env*) * NENV);

    // LAB 3: Your code here.
    // 在页表中建立映射关系, envs 的物理地址映射到虚拟地址 UENVS
    cprintf("mem_init() map envs to virtual address:UENVS, PTSIZE:%d, ROUNDUP:%d\n", PTSIZE, ROUNDUP((sizeof(struct Env)*NENV), PGSIZE));
    boot_map_region(kern_pgdir, UENVS, ROUNDUP((sizeof(struct Env)*NENV), PGSIZE), PADDR(envs), PTE_U);

运行结果(4194304 = 4096x1024, 98304= 4096x96):
mem_init() map envs to virtual address:UENVS, PTSIZE:4194304, ROUNDUP:98304
check_kern_pgdir() succeeded!

```

### Creating and Running Environments

你将需要编写运行用户环境所需的 `kern/env.c`代码, 因为我们还没有文件系统, 所以我们将设置内核来加载嵌入在内核中的静态二进制映像. JOS将此二进制文件作为 ELF可执行映像嵌入内核中.

Lab3 `GNUmakefile` 在 `obj/user/`目录中生成了许多二进制映像. 在 `kern/Makefrag`文件中, 你会发现一些魔法将这些二进制文件直接“链接”到内核可执行文件中, 就好像它们是 .o文件一样. 链接器命令行上的 `-b binary`选项会将这些文件作为“原始”未解释的二进制文件链接, 而不是作为编译器生成的常规 .o文件链接. (就链接器而言, 这些文件根本不必是 ELF文件——它们可以是任何格式, 例如文本文件或图片) 如果在构建内核后查看`obj/kern/kernel.sym`, 你会注意到链接器“神奇地”产生了许多有趣的符号, 这些符号具有晦涩的名字, 如_binary_obj_user_hello_start, _binary_obj_user_hello_end 和 _binary_obj_user_hello_size. 链接器通过修改二进制文件的文件名来生成这些符号名称; 这些符号为常规内核代码提供了引用嵌入式二进制文件的方法.

在 `kern/init.c` `i386_init()`中, 你将看到在某个环境中运行这些二进制映像之一的代码. 但是建立用户环境的关键功能还不完备, 你需要填写它们.

### Exercise 2

完成 `env.c` 文件中的下面函数.

```
在编写这些函数时, 你可能会发现新的 cprintf %e很有用——它打印与错误代码对应的描述. 例如,
	r = -E_NO_MEM;
	panic("env_alloc: %e", r);
will panic with the message "env_alloc: out of memory".
```

- env_init()

  初始化 envs数组中的所有 Env结构,并将它们添加到 env_free_list中. 还调用 env_init_percpu, 它将分段硬件配置为特权级别 0(内核)和特权级别 3(用户)的独立段.

  ```
  void env_init(void)
  {
  	// Set up envs array
  	// LAB 3: Your code here.
  	for(int i = NENV-1; i >= 0; i--)
  	{
  		envs[i].env_id = 0;
  		envs[i].env_status = ENV_FREE;
  		envs[i].env_link = env_free_list;
  		env_free_list = &envs[i];
  	}
  	// Per-CPU part of the initialization
  	env_init_percpu();
  	cprintf("env_init() done!\n");	
  }
  ```

- env_setup_vm()

  为新环境分配一个页目录(每个环境/进程都有一个自己的 page directory), 并初始化新环境地址空间的内核部分.

  只设置页目录表中和操作系统内核相关的页目录项, 用户环境的页目录项不需要设置, 因为所有用户环境的页目录表中和操作系统相关的页目录项都是一样的(除了虚拟地址 UVPT, 这个也会单独进行设置)

  ```
  static int
  env_setup_vm(struct Env *e)
  {
  	int i;
  	struct PageInfo *p = NULL;
  
  	// Allocate a page for the page directory
  	if (!(p = page_alloc(ALLOC_ZERO)))
  		return -E_NO_MEM;
  
  	// Now, set e->env_pgdir and initialize the page directory.
  	//
  	// Hint:
  	//    - The VA space of all envs is identical above UTOP
  	//	(except at UVPT, which we've set below).
  	//	See inc/memlayout.h for permissions and layout.
  	//	Can you use kern_pgdir as a template?  Hint: Yes.
  	//	(Make sure you got the permissions right in Lab 2.)
  	//    - The initial VA below UTOP is empty.
  	//    - You do not need to make any more calls to page_alloc.
  	//    - Note: In general, pp_ref is not maintained for
  	//	physical pages mapped only above UTOP, but env_pgdir
  	//	is an exception -- you need to increment env_pgdir's
  	//	pp_ref for env_free to work correctly.
  	//    - The functions in kern/pmap.h are handy.
  
  	// LAB 3: Your code here.
      // 自增引用计数
  	p->pp_ref++;
      // 页目录的虚拟地址
  	e->env_pgdir = (pde_t*)page2kva(p);
  	cprintf("e->env_pgdir:0x%08x\n",e->env_pgdir);
  
      // 这部分的页目录值和 kern_pgdir是一致的
      memcpy(e->env_pgdir, kern_pgdir, PGSIZE);
   
      //Map the directory below UTOP
      /*
      for(i = 0; i < PDX(UTOP); i++) 
      {
          e->env_pgdir[i] = 0;        
      }
      */
      
      //Map the directory above UTOP
      /*
      for(i = PDX(UTOP); i < NPDENTRIES; i++) 
      {
          e->env_pgdir[i] = kern_pgdir[i];
      }
      */
  	
  	// UVPT maps the env's own page table read-only.
  	// Permissions: kernel R, user R
  	e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_P | PTE_U;
  	cprintf("env_setup_vm() done!\n");	
  
  	return 0;
  }
  
  ```

- region_alloc()

  为环境分配和映射物理内存. 分配即分配物理页, 使用的是 page_alloc(); 映射即安装到页目录和页表中.

  注意要先把起始地址和终止地址进行页对齐, 对其之后我们就可以以页为单位, 为其一页一页的分配内存, 并且修改页目录表和页表

  ```
  static void
  region_alloc(struct Env *e, void *va, size_t len)
  {
  	// LAB 3: Your code here.
  	void *start = (void *)ROUNDDOWN((uint32_t)va, PGSIZE);
  	void *end = (void *)ROUNDUP((uint32_t)va+len, PGSIZE);
  
      struct PageInfo *p = NULL;
      void *i = NULL;
      int ret = 0;
      for(i = start; i < end; i += PGSIZE)
      {
      	p = page_alloc(0);
          if(p == NULL)
          {
  			panic("region_alloc(), allocation failed!!!");       
          }
   		// 修改页目录表和页表
          ret = page_insert(e->env_pgdir, p, i, PTE_W|PTE_U);
          if(ret != 0)
          {
              panic("region_alloc(), page_insert failed!!!");
          }
  	}
       
  	// (But only if you need it for load_icode.)
  	//
  	// Hint: It is easier to use region_alloc if the caller can pass
  	//   'va' and 'len' values that are not page-aligned.
  	//   You should round va down, and round (va + len) up.
  	//   (Watch out for corner-cases!)
  }
  ```

- load_icode()

  解析 ELF二进制映像,就像 bootloader 那样, 并将其内容加载到新环境的用户地址空间中.

  参考 boot/main.c中 bootloader 加载 kernel image的过程. 区别在于, bootloader是从 disk中加载 kernel, 而load_icode()要加载的二进制文件已经在 memory中了.

  有以下几点注意:

  1. 只有 p_type=ELF_PROG_LOAD的段才需要被被加载. 在加载 program segment时, 是 load到 user environment, 因此需要在 load之前使用 lcr3指令切换到当前 environment 的 page directory
  2. ph->p_va 是需要被加载到的虚地址
  3. ph->p_memsz 是整个在内存中占的大小, 也是我们申请空间时的大小
  4. 从 binary + ph->p_offset 开始的 ph->p_filesz字节需要被复制到 ph->p_va处
  5. 需要考虑一些 ELF头的入口点处理, 将该环境的指令寄存器 eip的值设置为该 elf格式文件的 e_entry的值
  6. 在指定完 program的 entry point之后, 需要将 page directory切换回 kernel directory

  ```
  static void
  load_icode(struct Env *e, uint8_t *binary)
  {
  	// Hints:
  	//  Load each program segment into virtual memory
  	//  at the address specified in the ELF segment header.
  	//  You should only load segments with ph->p_type == ELF_PROG_LOAD.
  	//  Each segment's virtual address can be found in ph->p_va
  	//  and its size in memory can be found in ph->p_memsz.
  	//  The ph->p_filesz bytes from the ELF binary, starting at
  	//  'binary + ph->p_offset', should be copied to virtual address
  	//  ph->p_va.  Any remaining memory bytes should be cleared to zero.
  	//  (The ELF header should have ph->p_filesz <= ph->p_memsz.)
  	//  Use functions from the previous lab to allocate and map pages.
  	//
  	//  All page protection bits should be user read/write for now.
  	//  ELF segments are not necessarily page-aligned, but you can
  	//  assume for this function that no two segments will touch
  	//  the same virtual page.
  	//
  	//  You may find a function like region_alloc useful.
  	//
  	//  Loading the segments is much simpler if you can move data
  	//  directly into the virtual addresses stored in the ELF binary.
  	//  So which page directory should be in force during
  	//  this function?
  	//
  	//  You must also do something with the program's entry point,
  	//  to make sure that the environment starts executing there.
  	//  What?  (See env_run() and env_pop_tf() below.)
  
  	// LAB 3: Your code here.
  
  	struct Elf *elf = (struct Elf *)binary;
  	if (elf->e_magic != ELF_MAGIC)
  	{
  		panic("invalid ELF file!");
  	}
  	
  	struct Proghdr *ph,*end_ph;
  	ph = (struct Proghdr *)((uint8_t *)elf + elf->e_phoff);
  	end_ph = ph + elf->e_phnum;
  	cprintf("ph:0x%08x, end_ph:0x%08x\n", ph, end_ph);
  	
  	lcr3(PADDR(e->env_pgdir));
  	
  	for (; ph < end_ph; ph++)
  	{
  		if(ph->p_type==ELF_PROG_LOAD){
  			if((ph->p_memsz - ph->p_filesz) < 0)
  			{
  				panic("p_memsz < p_filesz");
  			}
  			region_alloc(e, (void*)ph->p_va, ph->p_memsz);
  			memcpy((void*)ph->p_va, (void*)binary + ph->p_offset, ph->p_filesz);
  			memset((void*)(ph->p_va + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
  		}
  	}
  	e->env_tf.tf_eip = elf->e_entry;
  	lcr3(PADDR(kern_pgdir));
  	
  	// Now map one page for the program's initial stack
  	// at virtual address USTACKTOP - PGSIZE.
  	// LAB 3: Your code here.
  	region_alloc(e, (void*)(USTACKTOP - PGSIZE), PGSIZE);
  	
  	cprintf("load_icode() done!\n");	
  }
  ```

- env_create()

  使用 env_alloc分配一个环境, 并调用 load_icode将 ELF二进制文件加载到其中

  ```
  void
  env_create(uint8_t *binary, enum EnvType type)
  {
  	struct Env *new_env;
  	int ret = env_alloc(&new_env,0);
  	if(ret != 0)
  	{
  		panic("env_create() env_alloc fail: %e",ret);
  	}
  	
  	new_env->env_type = type;
  	cprintf("env_create() done before load_icode()\n");
  
  	load_icode(new_env, binary);
  }
  ```

- env_run()

  启动一个给定的环境并以用户模式运行(上下文切换, context switch).

  ```
  void
  env_run(struct Env *e)
  {
  	// Step 1: If this is a context switch (a new environment is running):
  	//	   1. Set the current environment (if any) back to
  	//	      ENV_RUNNABLE if it is ENV_RUNNING (think about
  	//	      what other states it can be in),
  	//	   2. Set 'curenv' to the new environment,
  	//	   3. Set its status to ENV_RUNNING,
  	//	   4. Update its 'env_runs' counter,
  	//	   5. Use lcr3() to switch to its address space.
  	// Step 2: Use env_pop_tf() to restore the environment's
  	//	   registers and drop into user mode in the
  	//	   environment.
  
  	// Hint: This function loads the new environment's state from
  	//	e->env_tf.  Go back through the code you wrote above
  	//	and make sure you have set the relevant parts of
  	//	e->env_tf to sensible values.
  
  	// LAB 3: Your code here.
  	if((curenv != NULL) && curenv->env_status == ENV_RUNNING)
  	{
  		curenv->env_type = ENV_RUNNABLE;
  	}
  	curenv = e;
  	e->env_status = ENV_RUNNING;
  	e->env_runs++;
  	lcr3(PADDR(e->env_pgdir));
  	
      //保存环境
  	env_pop_tf(&e->env_tf);
  }
  ```

下面是在调用用户代码之前的代码调用图, 确保你理解每一步的目的.

- `start` (`kern/entry.S`)
- `i386_init` (`kern/init.c`)
  - `cons_init`
  - `mem_init`
  - `env_init`
  - `trap_init`(目前还没实现)
  - `env_create`
  - `env_run`
    - `env_pop_tf`

完成之后, 编译内核并在 QEMU下运行它. 如果一切顺利, 系统会进入用户空间并执行 hello二进制文件, 直到它使用int 指令进行系统调用. 这时就会出现问题, 因为 JOS还没有设置硬件来允许从用户空间到内核空间的任何形式的转换. 当 CPU发现它没有被设置来处理这个系统调用中断时, 会生成一个一般保护异常, 发现它不能处理, 生成一个双重故障异常, 发现它也不能处理, 最后放弃所谓的 "triple fault". 通常, CPU会复位, 系统会重启. 

我们将很快解决这个问题, 但现在我们可以使用调试器检查是否进入用户模式. 使用 `make qemu-gdb` 并在`env_pop_tf` 处设置断点, 这应该是实际进入用户模式之前命中的最后一个函数. 

使用 `si` 单步运行, 处理器应该在 `iret`指令之后进入用户模式. 然后在用户环境的可执行文件中看到第一条指令, 即 `lib/entry.S` 中 `start` `label` 处的 `cmpl`指令. 

现在使用 `b *0x…`在 `hello`中的 `sys_cputs()`中设置 `int $0x30`处的断点(参见 `obj/user/hello.asm` 找到用户空间地址). 这个 `int`是一个系统调用, 将一个字符显示到控制台. 如果你不能执行到 `int`, 说明你的地址空间设置或程序代码有问题. 

完成代码后, 运行 make, make qemu, 出现了  "triple fault"

```
xxx:~/workspace/6.828_2018/lab$ make qemu
...
6828 decimal is 15254 octal!
edata_2017 end addr: 0xf018c014, bss end_2017 addr: 0xf018eff4
edata end addr: 0xf018e0e0, bss end addr: 0xf018efe0
Physical memory: 131072K available, base = 640K, extended = 130432K
nextfree: 0xf018f000, bss end addr: 0xf018efe0
nextfree: 0xf0190000
mem_init() kern_pgdir: 0xf018f000, kern_pgdir addr: 0xf018efec
mem_init() kern_pgdir[0]: 0, kern_pgdir[1]: 0, ...kern_pgdir[PGSIZE-1]: 0
mem_init() UVPT: 0xef400000, PDX(UVPT): 0x3bd, kern_pgdir[PDX(UVPT)] physical addr: 0x0018f000
nextfree: 0xf01d0000
nextfree: 0xf01d1000
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
mem_init() map envs to virtual address:UENVS, PTSIZE:4194304, ROUNDUP:98304
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
env_init() done!
e->env_pgdir:0xf03bc000
[00000000] new env 00001000
env_create() done before load_icode()
ph:0xf011b364, end_ph:0xf011b3e4
load_icVNC server running on `127.0.0.1:5900'
6828 decimal is 15254 octal!
edata_2017 end addr: 0xf018c014, bss end_2017 addr: 0xf018eff4
edata end addr: 0xf018e0e0, bss end addr: 0xf018efe0
Physical memory: 131072K available, base = 640K, extended = 130432K
nextfree: 0xf018f000, bss end addr: 0xf018efe0
nextfree: 0xf0190000
mem_init() kern_pgdir: 0xf018f000, kern_pgdir addr: 0xf018efec
mem_init() kern_pgdir[0]: 0, kern_pgdir[1]: 0, ...kern_pgdir[PGSIZE-1]: 0
mem_init() UVPT: 0xef400000, PDX(UVPT): 0x3bd, kern_pgdir[PDX(UVPT)] physical addr: 0x0018f000
nextfree: 0xf01d0000
nextfree: 0xf01d1000
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
mem_init() map envs to virtual address:UENVS, PTSIZE:4194304, ROUNDUP:98304
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
env_init() done!
e->env_pgdir:0xf03bc000
[00000000] new env 00001000
env_create() done before load_icode()
ph:0xf011b364, end_ph:0xf011b3e4
load_icode() done!
env_run() start...
EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde60 ESP=eebfde54
EIP=00800b44 EFL=00000092 [--S-A--] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
SS =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
DS =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
FS =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
GS =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
LDT=0000 00000000 00000000 00008200 DPL=0 LDT
TR =0028 f018eb60 00000067 00408900 DPL=0 TSS32-avl
GDT=     f011b300 0000002f
IDT=     f018e340 000007ff
CR0=80050033 CR2=00000000 CR3=003bc000 CR4=00000000
DR0=00000000 DR1=00000000 DR2=00000000 DR3=00000000 
DR6=ffff0ff0 DR7=00000400
EFER=0000000000000000
Triple fault.  Halting for inspection via QEMU monitor.
```

调试:

1. 在一个 terminal 运行 make qemu-gdb, 另一个 terminal 运行 make gdb

2. 在`env_pop_tf`设置断点, 这是进入用户模式之前的最后一个函数, 运行到此函数

   <img src="images/env_pop_tf.png" alt="env_pop_tf" style="zoom:67%;" />

3.  `si` 单步运行, 在 `iret`指令之后进入用户模式. 在用户环境的可执行文件中看到第一条指令, 即 `lib/entry.S` 中 `start` `label` 处的 `cmpl`指令. 

   <img src="images/iret.png" alt="iret" style="zoom:50%;" />

4. 查看 obj/user/hello.asm, 在 sys_cputs int $0x30 设断点, int指令是一个系统调用.

   由于系统调用还没有实现, 这里继续往下执行就会触发 triple fault(下图中的 3)

   <img src="images/sys_cputs.png" alt="sys_cputs" style="zoom:50%;" />

   <img src="images/Triple_fault.png" alt="Triple_fault" style="zoom:50%;" />

### env_pop_tf

env_pop_tf() 在 env_run() 中最后调用, 是进入用户模式之前的最后一个函数.
其实就是将栈指针 esp指向该环境(进程)的 env_tf, 将 env_tf 中存储的寄存器的值弹出到对应寄存器中, 然后通过 iret 指令弹出栈中的元素分别到 EIP, CS, EFLAGS 对应寄存器, 并跳转到 `CS:EIP` 存储的地址执行.
当使用 iret指令返回到一个不同特权级运行时, 还会弹出堆栈段选择子及堆栈指针分别到 SS与 SP寄存器, 这样相关寄存器都从内核设置成了用户程序对应的值, EIP存储的是程序入口地址.

```
void
env_pop_tf(struct Trapframe *tf)
{
	asm volatile(
		"\tmovl %0,%%esp\n"	//	esp指向tf结构，弹出时会弹到tf里
		"\tpopal\n"			//  弹出tf_regs中值到各通用寄存器
		"\tpopl %%es\n"		//  弹出tf_es 到 es寄存器
		"\tpopl %%ds\n"		//  弹出tf_ds 到 ds寄存器
		"\taddl $0x8,%%esp\n"   //  跳过tf_trapno和tf_err
		"\tiret\n"			//  中断返回 弹出tf_eip,tf_cs,tf_eflags,tf_esp,tf_ss到相应寄存器
		: : "g" (tf) : "memory");
	panic("iret failed");  /* mostly to placate the compiler */
}

struct PushRegs {
        /* registers as pushed by pusha */
        uint32_t reg_edi;
        uint32_t reg_esi;
        uint32_t reg_ebp;
        uint32_t reg_oesp;              /* Useless */
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

#### Trapframe

Trapframe 存储的是当前环境(进程)的寄存器的值, `env_pop_tf`中便是将 trapframe的起始地址赋值给 esp, 然后用的这个顺序将栈中元素弹出到对应寄存器中的. 其中 popal是弹出 tf_regs到所有的通用寄存器中, 接着弹出值到 es, ds寄存器, 接着跳过 trapno和 errcode, 调用 iret分别将栈中存储数据弹出到 EIP, CS, EFLAGS寄存器中.

### User <-> Kernel

用户栈和内核栈的切换, 这个过程 ss, sp, eflags, cs, eip 在中断发生时由处理器压入, 通用寄存器部分则需要自己实现.

 <img src="images/user_kernel_switch.png" alt="user_kernel_switch" style="zoom:50%;" />



## 参考

https://pdos.csail.mit.edu/6.828/2018/labs/lab3/

https://zhuanlan.zhihu.com/p/74028717

https://www.dingmos.com/index.php/archives/8/

https://www.cnblogs.com/oasisyang/p/15520180.html

https://111qqz.com/2019/03/mit-6-828-lab-3-user-environments/

https://www.cnblogs.com/fatsheep9146/p/5341836.html

https://github.com/shishujuan/mit6.828-2017/blob/master/docs/lab3.md

https://github.com/shishujuan/mit6.828-2017/blob/master/docs/lab3-exercize.md

