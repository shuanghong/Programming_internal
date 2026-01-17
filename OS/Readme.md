## CSAPP:

CMU 镇系神课, 课程内容覆盖了编码、体系结构、操作系统、编译链接、网络、并发等. CSAPP 可以看作是计算机科学的总览.
全书有三个核心的视角:

1. 程序视角: 编译链接, 地址重定位
2. 硬件CPU视角: 指令的执行, 存储结构
3. 操作系统视角: 程序加载成为进程, 进程管理

参考:
https://csdiy.wiki/%E4%BD%93%E7%B3%BB%E7%BB%93%E6%9E%84/CSAPP/
CSAPP 的中文讲解: https://www.bilibili.com/video/BV1cD4y1D7uR/

### 相关课程

1. CMU 15213 Introduction to Computer Systems (ICS)

    主页: https://www.cs.cmu.edu/~213/

    课件: https://www.cs.cmu.edu/~213/lectures/

    历史课件: 

    https://www.cs.cmu.edu/~213/resources.html

    http://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/

    CMU 18-613 Computer Systems

    主页: https://www.andrew.cmu.edu/course/18-613/

    课件: https://www.andrew.cmu.edu/course/18-613/schedule.html

2. 南大 ICS + PA
    计算机系统基础(一),(二),(三), 可以看作是 "CSAPP-like" 的课程, 分别对应第二章机器级表示、第七章链接、第八章异常控制流、第六章存储器层次结构等.
    https://www.icourse163.org/course/NJU-1001625001
    https://www.icourse163.org/course/NJU-1001964032
    https://www.icourse163.org/course/NJU-1002532004

    PA 大实验, 实现一个功能完备的模拟器 NEMU(NJU EMUlator), 在其上运行游戏“仙剑奇侠传”
    https://nju-projectn.github.io/ics-pa-gitbook/ics2021/index.html

    课件:
    https://github.com/liuxinyu123/cs_basis

### 相关读书笔记

[第01章：计算机系统漫游 | CSAPP重点解读](https://fengmuzi2003.gitbook.io/csapp3e/)

[【读薄 CSAPP】零 系列概览 | 小土刀 4.0](https://www.wdxtub.com/blog/csapp/thin-csapp-0)

## Computer System Architecture
计算机组成原理、体系结构是学习 OS的前置知识.

### 相关课程

1. 南大 ICS

2. UCB CS61C, 伯克利 CS61 系列的最后一门课程，深入计算机的硬件细节.
    https://inst.eecs.berkeley.edu/~cs61c/sp22/
  
3. UCB CS 152, Computer Architecture and Engineering, 是 CS61C 的进阶

     https://www2.eecs.berkeley.edu/Courses/CS152/

4. CSCE 513, Computer Architecture

     https://passlab.github.io/CSCE513/

### 相关书籍

1. 入门读物: 《大话计算机》

2. 基础读物: 

   《程序是怎样跑起来的》

   H&P -《计算机组成与设计:硬件/软件接口》

   袁春风 -《计算机组成与体系结构》

   胡伟武 -《计算机体系结构基础》

3. 进阶读物: 

   H&P -《计算机体系结构:量化研究方法》

   姚永斌 -《超标量处理器设计》


## OS

### 相关课程

1. MIT 6.828

   https://pdos.csail.mit.edu/6.828/2018/schedule.html

2. 清华, 操作系统原理

   [清华 操作系统原理_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1uW411f72n?p=19)

   课件:

   [OS2014 - OscourseWiki (tsinghua.edu.cn)](http://os.cs.tsinghua.edu.cn/oscourse/OS2014#Course_Introduction) 
   [OS2018spring - OscourseWiki (tsinghua.edu.cn)](http://os.cs.tsinghua.edu.cn/oscourse/OS2018spring) 
   [OS2020spring - OscourseWiki (tsinghua.edu.cn)](http://os.cs.tsinghua.edu.cn/oscourse/OS2020spring) [Releases · dramforever/os-lectures-build · GitHub](https://github.com/dramforever/os-lectures-build/releases) 
   PS. 2014 年的课程章节与视频比较匹配, 2018 课件内容与视频更匹配, 2020 年课件最新, 包含一些 RISC-V 的内容.

   其他课件参考:

   [os-syllabi/material/tsinghua-os-2019-spring at master · computer-system-education/os-syllabi · GitHub](https://github.com/computer-system-education/os-syllabi/tree/master/material/tsinghua-os-2019-spring)

   https://github.com/LearningOS

   学习笔记:

   [OperatingSystemInDepth/README.md at main · OXygenMoon/OperatingSystemInDepth · GitHub](https://github.com/OXygenMoon/OperatingSystemInDepth/blob/main/Deep_into_OperatingSystem.md)

3. CS162 Operating System

   https://inst.eecs.berkeley.edu/~cs162/fa20/

   [UCB CS162: Operating System - CS自学指南 (csdiy.wiki)](https://csdiy.wiki/操作系统/CS162/)

### 相关书籍

* Operating Systems - Three Easy Pieces 
  中文书: 操作系统导论
  官网: https://pages.cs.wisc.edu/~remzi/OSTEP/
  		相比书, 增加了 "Security" 章节

  翻译:
  	[操作系统导论（中文版） | ostep-chinese](https://itanken.github.io/ostep-chinese/)

  ​	https://github.com/remzi-arpacidusseau/ostep-translations/tree/master/chinese

* Operating Systems: Principles and Practice (2nd Edition) https://www.kea.nu/files/textbooks/ospp/
  CS162 Operating System 教材, 更加侧重 OS 的设计而非组成, 一共四卷, 第二章 Concurrency 尤其精彩

* 操作系统真相还原

参考
	https://www.zhihu.com/question/31863104
	https://book.douban.com/subject/25984145/
	https://book.douban.com/subject/26745156/
	https://book.douban.com/subject/36560814/

## 参考:
CS自学指南:
https://csdiy.wiki/

https://github.com/PKUFlyingPig/cs-self-learning/issues/128

https://blog.mwish.me/2022/05/02/Arch-Blog-Overviews/