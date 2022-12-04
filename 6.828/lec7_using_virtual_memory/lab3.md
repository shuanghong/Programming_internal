# Lab 3: User Environments(Process)

## Introduction

在本 lab中, 你将实现基本的内核功能, 以使受保护的用户模式环境(即“进程”)运行. 你将增强 JOS内核, 设置数据结构来跟踪用户环境, 创建单个用户环境, 将程序映像加载到其中并启动它. 还将使 JOS内核能够处理用户环境发出的任何系统调用(system calls)和处理它引起的任何其他异常(exceptions handling).

注: 本 lab中环境 "environments"是进程 "process"的同义词. 两者都是指 "允许你运行程序的抽象“. 我们引入术语“环境”而不是传统的术语“进程”, 是为了强调 JOS环境和 UNIX进程提供不同的接口, 并且不提供相同的语义.

### Getting Started

```
git checkout -b lab3 origin/lab3
git merge lab2
```

Lab 3包含了一些新的源文件, 应该浏览一下:

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

新的文件 `inc/env.h` 包含 JOS中用户环境的基本定义. 内核使用 Env数据结构来跟踪每个用户环境. 在 lab中你最初将只创建一个环境, 但需要设计 JOS内核以支持多个环境. Lab 4 将利用这个特性, 允许用户环境fork 其他环境.

正如你在 `kern/envirv.c`中看到的, 内核维护了三个与环境相关的主要全局变量:

```
struct Env *envs = NULL;		// All environments
struct Env *curenv = NULL;		// The current env
static struct Env *env_free_list;	// Free environment list
```

一旦 JOS启动并运行, `*env` 指针指向一个表示系统中所有环境(进程)的 Env结构数组. 在我们的设计中, JOS内核将支持同一时刻最多 NENV个活跃的环境, 尽管在任何给定时间通常会有更少的运行环境. (NENV 是在 `inc/env.h` 中 `#define` 的常量). 一旦分配了它，envs 数组将包含每个可能的 NENV 环境的 Env数据结构的单个实例.

JOS 内核将所有未激活的 Env结构保存在 `env_free_list` 中, 这种设计允许轻松地分配和回收环境, 因为只需将它们添加到空闲列表中或从空闲列表中删除即可.

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

**env_tf**: 在环境未运行时保存该环境的已保存寄存器值: 即,当内核或其他环境正在运行时. 当从用户模式切换到内核模式时, 内核会保存这些信息, 以便稍后恢复中断的环境.

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

在 lab 2中你在 `mem_init()` 函数中分配了 `pages[]` 数组的地址空间, 用于记录内核中所有的页的信息. 现在你需要进一步去修改 `mem_init()` 函数, 来分配一个类似的 `Env` 结构体数组, 叫做 `envs`.

#### Exercise 1

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

## 参考

https://zhuanlan.zhihu.com/p/74028717

https://www.dingmos.com/index.php/archives/8/

https://www.cnblogs.com/oasisyang/p/15520180.html

https://111qqz.com/2019/03/mit-6-828-lab-3-user-environments/

https://www.cnblogs.com/fatsheep9146/p/5341836.html