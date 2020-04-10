# Lecture 1: O/S overview
原文: [https://pdos.csail.mit.edu/6.828/2017/lec/l-overview.txt](https://pdos.csail.mit.edu/6.828/2017/lec/l-overview.txt)

## Overview

* 6.828 目标

  * 了解操作系统的设计和实现
  * 通过构建小型 O/S 实践经验

* O/S 的目的是什么?

  * 支持应用程序
  * 为了便利和可移植性, 对硬件进行抽象
  
* 将硬件多路复用到多个应用程序中
  * 隔离应用程序以控制 bug
  * 允许在应用程序之间共享
  * 提供高性能

* O/S 的设计方法是什么?
  * 从小的方面看: h/w 管理库
  * 从大的方面看: 物理机器 -> 抽象的一个 w/ 更好的性能

* 组织: 分层的图片

  ​    h/w: CPU, mem, disk, &c

  ​    kernel services

  ​    user applications: vi, gcc, &c

  * 我们非常关心接口和内部内核结构

* O/S 内核通常提供什么服务?
    * processes
    * memory allocation
    * file contents
    * directories and file names
    * security
    * many others: users, IPC, network, time, terminals

* O/S 抽象是什么样子的?

  * 应用程序只能通过系统调用(system calls)来查看它们

  * 例子, 来自 UNIX (如 Linux, OSX, FreeBSD):

    ```
    fd = open("out", 1);
    write(fd, "hello\n", 6);
    pid = fork();
    ```

* 为什么 O/S 设计/实现 困难/有趣 ?
  * 环境是无情的: 古怪的 h/w, 弱调试器
  * 它必须是效率高的 (因此是低层的?)...但是抽象/可移植(因此是高级的 ?)
  * 功能强大的(因此许多特征?)...但是简单(因此一些可组合的构建块?)
  * 特点交互: fd = open();…;fork()
  * 行为交互: CPU 优先级 vs 内存分配器
  * 开放式问题: 安全;性能
  
* 你会很高兴了解了操作系统, 如果你
  * 想解决以上问题
  * 关心引擎盖下发生了什么
  * 必须构建高性能系统
  * 需要诊断 bug 或安全问题



## Class structure

* 讲义
  * O/S 的想法
  * 详细检查 xv6, 一个传统的 O/S
  * xv6 编程作业来激发讲课
  * 关于最近一些主题的论文

* 实验: JOS, 一个小型的 x86 O/S
  *  构建它, 5个实验 + 你选择的最终实验
  * 内核接口: 公开硬件, 但保护——很少抽象!
  * 非特权用户级库: fork, exec, pipe…
  * 应用程序: 文件系统, shell, ..
  * 开发环境: gcc, qemu
  * 实验1出来了
* 两次考试: 期中考, 期末考试



## Introduction to system calls

* 6.828 主要是关于系统调用接口的设计和实现, 让我们看看程序如何使用接口, 我们将集中讨论 UNIX (Linux, Mac, POSIX).

* 一个简单的例子: “ls” 调用了什么系统调用 ?
  * 跟踪系统调用:
    * 在 OSX 上: sudo dtruss /bin/ls
    * 在 Linux 上: strace /bin/ls
  * 这么多系统调用!
  
* 示例: 将输入复制到输出

  ```
  #include <unistd.h>
  
  #define BUFSIZE 128
  char buf[BUFSIZE];
  
  int main() {
    int n;
  
    n = read(0, buf, BUFSIZE);	// fd 0 is "standard input"
    write(1, buf, n);		// 1 is "standard output"
  }
  ```


- 示例: 创建一个文件

  ```
  #include <unistd.h>
  #include <fcntl.h>
  
  int main(void) {
    int fd = creat("output.txt", 0666);
    write(fd, "ooo\n", 4);
  }
  ```

- 示例: 重定向标准输出

  ```
  #include <unistd.h>
  #include <fcntl.h>
  
  int main(void) {
    int fd = creat("output.txt", 0666);	// 返回 fd 3, 因为 fd 2 标准错误 stderr
    dup2(fd, 1);	// 为 fd 3 创建副本 fd 1, 两个fd指向同一个底层I/O对象, 标准输出被重定向到fd 3
    write(1, "rrr\n", 4);
  }
  ```

