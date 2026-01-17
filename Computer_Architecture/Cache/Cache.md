# Cache

## Overall

![](reference\mindmap_计算机体系结构第三讲：cache.png)   

## Cache Basic

### 定义

一种体积较小,运行速度更快的存储设备. 充当较大, 运行速度较慢的存储设备中一部分数据的临时存储区域(主存的备份), 用于加速 CPU访问数据.

![](images\Cache_Concepts.jpg) 

### 动机

处理器执行指令速度远超 DRAM 访问速度差距问题(指令访问和数据访问), 从 1:1 到 1000:1(1980-2020年间), 形成"处理器-内存延迟鸿沟".

#### Processor-DRAM Latency Gap

![](images\Processor-DRAM_Latency_Gap.jpg)                       

### 作用

利用时间局部性(重复访问同一数据)和空间局部性(访问相邻数据), 减少处理器访问延迟.

* 时间局部性: 近期被访问的数据很可能再次被访问, 缓存保留最近使用的数据
* 空间局部性: 访问某个地址后, 其临近地址可能被访问, 缓存以块(Cache Line)为单位加载主存数据

### 适用场景

有规律的访问模式, 如连续地址访问. 

总之, 在 CPU芯片内增设高速缓存(Cache), 存储部分主内存数据副本, 速度更快但成本更高, 类似图书馆中“将常用书籍从书架暂放桌面”, 通过局部性原理减少频繁访问主存.

通常分指令缓存与数据缓存.

## Cache Structure

![](images\Cache_Structure.jpg) 

Cache 由 Data RAM 和 Tag RAM 组成.

### Cache line/block

缓存行/块. 缓存中最小的可加载单元, 将主存储器中的数据传送到缓存时, 是以缓存行(或缓存块)的粒度来进行的, 通常为 32/64 Bytes.

图中 A1~A4 为一个 Cache line, A5 ~A8 为另一个 Cache Line.

#### Example

![](images\Cache_Line_1.jpg){style="zoom:10"} 

 

![](images\Cache_Line_2.jpg) 

### Offset

行内偏移. 用于索引一个 Cache Line 中的偏移量, CPU 可以按字节或字来寻址⾼速缓存⾏的内容.

Offset 是内存地址编码中的低位, 如果 Cache line 大小为 64B, 则 offset 占 6 位 (2^6 = 64, 5:0).

### Index

组(Set)索引. ⽤于索引和查找地址在⾼速缓存⾏的哪⼀组(Set)中. 图中 A1, A5, A9, A13 分别在不同的 Cache Line, 就是通过 Index 进行区分.

Index 是内存地址编码中间区域的一部分. 2^(Index 的位数) = Set 个数, 例如 Index 占 2bits, 则 Cache 被分成了 4个组.

### Set

组. 一般把 Cache 划分成多个组, 图中 A、B、C、D  Way中相同的 Index 构成组(Set). Index 可以用 0~3(0b00, 0b01, 0b10, 0b11) 表示.

组. 一般把 Cache 划分成多个组, 图中 A、B、C、D  Way中相同的 Index 构成组(Set). Index 可以用 0~3(0b00, 0b01, 0b10, 0b11) 表示.

例如 A13, B13, C13, D13所在的 Cache Line 的 Index都是 3(0b11), 构成了一个组.

### Way

路. 每个 Set 被分成大小相同的几个块, 即为 Way. Way 是 Set 组中的槽位数量, 等于关联度 (associativity).

比如 4-way set-associative 表示每个 Set 中有 4 个 Way(4 条可放不同 tag 的行), 图中 A1, A5, A9, A13 所在 Cache Line 共同组成了一路, 就是 A Way, 同理还有 B, C, D Way.

![](images\Cache_Structure_Set_Way.jpg) 

### Set-Way

* 层级关系

  缓存 ---> 多个 Set ---> 每个 Set 包含多个 Way ---> 每个 Way 对应一个 Cache line

* 数学关系
   缓存总大小 = Set 数 × Way 数 × 行大小

### Tag

内存地址中的高位一部分被称为 Tag, 用来与 Set 中每个 Way 存储的 Tag 比较, 决定是否命中.

图中 T-x 都是一个个的 Tag. Tag占用的存储空间不会计入 Cache Size

### Meta data

为了实现读写策略和替换算法, Cache 中还包含了一些元数据.

#### Tag

匹配内存地址中的 Tag, 决定是否命中.

#### Valid Bit

有效位, 标识缓存行是否包含有效数据.

#### Dirty Bit

标识缓存行是否被修改, 用于写回策略.

## Cache Type(Placement Policy)

[Cache placement policies - Wikipedia](https://en.wikipedia.org/wiki/Cache_placement_policies)

[【计算机组成原理】Cache地址映射机制详解：直接映射、全相联映射与组相联映射 - 知乎](https://zhuanlan.zhihu.com/p/1908573376575939209)

缓存放置策略是指决定特定内存块在进入 CPU 缓存时所放置位置的策略, 一块内存不可能随意放置在缓存中的任意位置; 它可能会受到缓存放置策略的限制, 被限定在特定的缓存行或一组缓存行中.

在缓存中放置内存块有三种不同的策略:直接映射, 全相联, 组相联, 对应不同的 Set-Way 的关系.

![](images\Cache_Placement_Policy.jpg) 



### Direct Mapped Cache

![](images\Cache_Direct_Mapped_1.jpg) 

#### 核心思想

每个主存块在 Cache 中都有一个唯一固定的映射位置. 这种映射关系是确定的, 类似于每个物品在仓库中都有一个预先分配好的货架编号.

直接映射 Cache 分多个 Set, 每个 Set 只有一个 Cache line(Way = 1)

#### 映射规则

Cache 块号 (Index) = (主存块号) mod (Cache 总块数)

![](images\Cache_Direct_Mapped_2.jpg) 

#### 地址结构: | Tag | Index | Offset |

* Index: (Set 索引) 位数 = log2(Cache 总块数)
* Tag: 位数 = 地址中除去 Index 位数后的剩余高位

![](images\Cache_Direct_Mapped_3.jpg) 



![](images\Cache_Direct_Mapped_4.jpg) 

#### 优点

硬件实现简单, 成本较低, 地址转换和查找速度快 (直接通过 Index 字段定位到 Cache Line, 只需进行一次 Tag 比较)

#### 缺点

映射方式固定, 容易产生冲突(取模后的 Cache Index 相同). 如果程序频繁访问的多个主存块恰好映射到同一个Cache 块, 会导致这些块在 Cache 中被反复替换, 显著降低命中率(此现象称为“抖动”或冲突缺失)

### Fully Associative Cache

![](images\Cache_Fully_Associative_1.jpg) 

#### 核心思想

全相联映射是直接映射的另一个极端, 主存中的任何一个块都可以映射到 Cache 中的任何一个空闲块位置. 这种方式提供了最大的映射灵活性.

#### 映射规则

#### 地址结构: | Tag | Offset |

整个 Cache 只有一个 Set(因此没有 Index 位), 所有 Cache Line 都在这个 Set 中.

CPU访问时, 需要将主存地址中的 Tag 与 Cache 中所有有效块的 Tag 进行并行比较, 以确定是否命中.

![](images\Cache_Fully_Associative_2.jpg) 

#### 优点

映射高度灵活, Cache 空间利用率高, 因映射产生的冲突概率最低 (只要Cache中有空闲块即可装入).

硬件实现简单, 成本较低, 地址转换和查找速度快 ( 直接通过 Index 字段定位到 Cache Line, 只需进行一次 Tag 比较)

#### 缺点

Tag 比较逻辑复杂且成本高(需要大量的硬件比较器并行工作), 查找速度可能受比较规模影响. 因此全相联映射通常用于规模较小的 Cache, 如 TLB (Translation Lookaside Buffer)

### N-Way Set Associative Cache

![](images\Cache_Set_Associative_1.jpg)  

#### 核心思想

这是直接映射和全相联映射的一种折中方案.

首先将 Cache 分为若干个组 (Set), 每个 Set内包含 N 个路 (Cache Line 个数如 2-Way, 4-Way). 主存块首先通过确定的规则映射到 Cache 中的一个特定组, 然后在这个组内部的 N 个块位置中, 采用全相联的方式自由选择一个空闲块进行存放.

#### 映射规则

Cache 组号 (Set Index) = (主存块号) mod (Cache 中的总 Set 数)

与直接映射的区别是取模的 Cache 总块数变成总 Set 数, 比如 8个块分成 4个 Set

#### 地址结构: | Tag | Index | Offset |

* Set 组数: Cache 总块数 / N-Way
* Index: Set Index= log2(Set 组数)
* Tag: 地址中除去 Index 位数后的剩余高位

![](images\Cache_Set_Associative_2.jpg) 

#### 优点

在冲突概率和硬件成本之间取得了良好的平衡, 其冲突概率低于直接映射, 而硬件实现成本低于全相联映射, 是现代处理器 Cache 设计中应用最为广泛的方式.

#### 缺点

相较于直接映射, 硬件略为复杂. 在同一组内的 N 个块之间仍然可能发生冲突, 但冲突范围和概率已显著减小.

### Summary

![](images\Cache_Placement_Policy_Summary.jpg) 

## Cache Access

### Addressing: Tag,Index,Offset

![](images\Cache_Addressing.jpg) 

#### ARMv7 example

4-way set associative 32KB data cache, 8-word cache line length(32 bytes).

缓存总大小 = Set 数 × Way 数 × 行大小

* Offset: log2(32) = 5, bit [4:0]

  实际访问 Cache line 时可以是要访问一个字、半字或者一个字节. 使用地址的位 [4:2] 来从该行内的 8 个字中进行选择.

* Index: Set 数 = Cache 总块数 / N-Way, 每个 Way 代表一个 Cache line, 32 bytes

  32KB / (4 x 32B) = 256, 8bits, bit[12:5]

*  Tag: 32bit 地址中剩下 [31:13] 用作 Tag

![](images\Cache_Set_Associative_4Way_32bits.jpg) 

   

#### ARMv8 example

4-way set associative 32KB L1 cache, 16-word cache line length(64 bytes).

![](images\Cache_Set_Associative_4Way_32bits_ARMv8.jpg) 



### Finding Data in Cache

Lectures/cs61c-fa24/caches.pdf

![](images\Cache_Finding_Data.jpg) 

#### Direct Mapped Cache Access

8 sets, 3 index bits, each set has 1 cache line

1. 根据 Index 值索引到 Set, 也索引到 cache line
2. 对比地址中的 Tag 和 cache line 中的 Tag
3. 如果 Tag 匹配并且 Valid bit 有效, 则命中.

![](images\Cache_Direct_Mapped_Data_Access_1.jpg) 



#### Set Associative Cache Access

1. 根据 Index 值索引到 Set, 
2. 地址中的 Tag 与 Set 中的所有 cache line 一一对比
3. 如果某个 cache line 中的 Tag 匹配并且 Valid bit 有效, 则命中.

![](images\Cache_Set_Associative_Data_Access.jpg) 

#### Cache Hit

* 所寻找的数据就在缓存中
* 从缓存中获取该数据并传送到处理器

#### Cache Miss

* 所寻找的数据未在缓存中
* 需要到内存中查找该数据, 将数据放入缓存中, 并传送给处理器

有 3个类型的 Cache miss.

1. Compulsory miss: 强制性不命中, 例如首次访问, 空的缓存必定会造成不命中
2. Capacity miss: 容量原因的不命中, 即缓存太小不足以存储内存的内容
3. Conflict miss: 冲突原因不命中, 多个数据同时映射到同一 cache block (direct mapped cache 比较常见)

![](images\Cache_Direct_Mapped_Data_Access_2.jpg) 

#### Policies

缓存策略使我们能够描述何时应将一行分配到数据缓存, 以及当执行命中数据缓存的存储指令时应该发生什么. 在缓存操作中可以做出多种不同的选择

* 分配策略(allocation policy): 是什么原因导致来自外部存储器的一行被放入缓存?
* 写策略(write policy): 当核心执行一次写操作并命中缓存时,应该如何控制?
* 替换策略(replacement policy): 在组相联缓存中, 控制器如何决定使用哪一行来存放新进入的数据?

##### Allocation policy

当 CPU 执行缓存查找且发生 Cache miss 时, 必须确定是否执行缓存行填充并将该地址从内存中复制过来.

* Read allocation (RA): 读分配策略仅在读取时分配缓存行. 如果 CPU 执行写操作且在缓存中未命中, 缓存不会受到影响, 写操作会传递到下一层级. 也称 No write-allocate(写不分配).

* Write allocation (WA): 写分配策略会为在缓存中未命中的读取或写入分配缓存行(因此更准确地称为读写缓存分配策略). 对于在缓存中未命中的内存读取和内存写入, 都会执行缓存行填充. 

  该策略通常与写回写策略结合(Write Back)使用.

##### Write policy

当 CPU 执行存储指令时, 会对要写入的地址进行缓存查找, 如果写操作命中缓存,有两种选择

* Write-through: 写穿透(写直达), 数据同时写入 Cache 和主存; 写入主存会花更长时间; 实现简单
* Write-back: 写回
  * 将数据写入缓存, 并将 dirty bit 设置为 1
  * 当该行从缓存中被驱逐时, 再写入内存
  * 通常可以减少内存访问流量, 因为在驱逐之前可能会对同一位置进行多次写入

##### Common Combinations

* Write-through, No write-allocate: 写直达, 无写分配
  * 当写操作命中时, 缓存和主存同时更新
  * 当写操作未命中时, 不将块加载到缓存, 只更新主存
  * 当读操作未命中时, 仍将该行加载到缓存
* Write-back, Write-allocate:写回, 写分配
  * 当读或写操作未命中时, 将该行加载到缓存
  * 当写操作时, 只更新缓存并将 dirty bit 设为 1

##### Replacement policy

[refer to Tsinghua-os-2018 lec9-第九讲 页面置换算法]

当出现容量/冲突未命中 (capacity/conflict miss) 时, 需要决定替换哪个块.

* 对于 direct mapped cache, 没有选择空间(只能替换对应位置)
* 对于 N-way associative cache, 必须实现替换策略
* 需要在降低 miss rate 和实现复杂度之间进行权衡

###### Random Replacement

随机选择一个缓存行进行驱逐, 实现简单但性能不稳定. 在实现中可能选择伪随机数.

###### First In, First Out (FIFO)

* 思路: 选择在缓存中驻留时间最长 Cache line 进行替换
* 实现: 
  * 维护一个记录所有位于 Cache 中的 Cache line 链表
  * 链表元素按驻留 Cache 的时间排序, 链首最长, 链尾最短
  * 替换时选择链首页面进行置换, 新页面加到链尾
* 特征
  * 实现简单但性能较差, 置换的 Cache line 可能是经常访问的

###### Least Recently Used (LRU)

最近最少使用.

- 思路: 通过过去预测未来, 替换最长时间未被使用的条目.
  如果某一 Cache line 最近没有被使用, 那么它在近期再次被访问的可能性通常较低(时间局部性原理)或者说某Cache line 长时间未被访问, 则它们在将来还可能会长时间不会访问.

- 实现

  - 链表实现, 维护一个按最近一次访问时间排序的页面链表.
    - 链表首节点是最近刚刚使用过的页面, 链表尾节点是最久未使用的页面
    - 访问 Cache 时, 找到相应 Cache line 并把它移到链表之首
    - Cache Miss 时替换链表尾节点的 Line
  - 堆栈
    - 访问 Cache line 时, 将此 Line 压入栈顶, 并抽出栈内相同的 Line
    - Cache Miss 时, 替换栈底的 Line

- 特征

  - 实践中效果很好

  - 需要跟踪访问顺序, 并快速找到最旧的缓存行

  - 纯 LRU 需要复杂的逻辑, 开销比较大

    

## Multi-level Cache

[cache知识分享系列之1：cache基本原理_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1g24y187zc/?spm_id_from=333.1387.favlist.content.click&vd_source=9428dc34f6d63cc56fe254146d145a47)

Inclusion Policy

​	Inclusive multilevel cache

​	Non-inclusive multilevel caches

​	Exclusive multilevel caches

## Finding Data in a Cache

[caches.pdf]