# **LEC 3:** C and gdb

## gdb

总结一下目前为止常用的 gdb 操作:

### 断点及单步操作

```
b *0x7c00	[0x7C00 地址设置了一个断点]
c			[执行到下一个断点]
si			[单独执行下一条指令]
si N		[一次执行 N 条指令]
```

### 寄存器操作

```
info reg	[打印CPU寄存器的值, r0,r1...]
```

### 内存操作

```
x/i			[检查内存中的指令]
x/Ni ADDR	[反汇编从地址ADDR开始的N个指令, 没有ADDR则表示当前位置, 指令以word/2bytes显示]
x/Nx ADDR	[打印地址ADDR开始的连续N个内存数据, 数据以16进制显示]
			比如显示栈上的数据: x/x $esp
```

![](images/gdb_xi_0x7c00.JPG)

```
(gdb) x/24x $esp
0x7bcc:	0x00007db7	0x00000000	0x00000000	0x00000000
0x7bdc:	0x00000000	0x00000000	0x00000000	0x00000000
0x7bec:	0x00000000	0x00000000	0x00000000	0x00000000
0x7bfc:	0x00007c4d	0x8ec031fa	0x8ec08ed8	0xa864e4d0
0x7c0c:	0xb0fa7502	0xe464e6d1	0x7502a864	0xe6dfb0fa
0x7c1c:	0x16010f60	0x200f7c78	0xc88366c0	0xc0220f01
```

### 参考

更多关于 GDB 的教程参考 :
https://sourceware.org/gdb/current/onlinedocs/gdb/
		[5 Stopping and Continuing](https://sourceware.org/gdb/current/onlinedocs/gdb/Stopping.html#Stopping)
		[8 Examining the Stack](https://sourceware.org/gdb/current/onlinedocs/gdb/Stack.html#Stack)
		[10 Examining Data](https://sourceware.org/gdb/current/onlinedocs/gdb/Data.html#Data)

https://sourceware.org/gdb/documentation/

## c (pointers)

### example

```
#include <stdio.h>
#include <stdlib.h>

void f(void)
{
    int a[4];
    int *b = malloc(16);
    int *c;
    int i;

    printf("1: a = %p, b = %p, c = %p\n", a, b, c);

    c = a;
    for (i = 0; i < 4; i++)
	a[i] = 100 + i;
    c[0] = 200;
    printf("2: a[0] = %d, a[1] = %d, a[2] = %d, a[3] = %d\n",
	   a[0], a[1], a[2], a[3]);

    c[1] = 300;
    *(c + 2) = 301;
    3[c] = 302;
    printf("3: a[0] = %d, a[1] = %d, a[2] = %d, a[3] = %d\n",
	   a[0], a[1], a[2], a[3]);

    c = c + 1;
    *c = 400;
    printf("4: a[0] = %d, a[1] = %d, a[2] = %d, a[3] = %d\n",
	   a[0], a[1], a[2], a[3]);

    c = (int *) ((char *) c + 1);
    *c = 500;
    printf("5: a[0] = %d, a[1] = %d, a[2] = %d, a[3] = %d\n",
	   a[0], a[1], a[2], a[3]);

    b = (int *) a + 1;
    c = (int *) ((char *) a + 1);
    printf("6: a = %p, b = %p, c = %p\n", a, b, c);
}

int main(int ac, char **av)
{
    f();
    return 0;
}
```

### 参考

"指针的算术运算" https://www.runoob.com/w3cnote/c-pointer-detail.html