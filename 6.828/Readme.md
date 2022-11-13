# MIT6.828

## MIT6.828-2017

schedule:

https://pdos.csail.mit.edu/6.828/2017/schedule.html

lecture:

https://pdos.csail.mit.edu/6.828/2017/lec/

tools:

https://pdos.csail.mit.edu/6.828/2017/tools.html

labguide:

https://pdos.csail.mit.edu/6.828/2017/labguide.html

reference:

https://pdos.csail.mit.edu/6.828/2017/reference.html

Triple fault.  Halting for inspection via QEMU monitor.



## Setup on VM ubuntu18.04(x86_64)

注: 参照 2018年的 tools 页面, 2017 的会有很多错误.

错误解决参考 https://zhuanlan.zhihu.com/p/489921553

tools:

https://pdos.csail.mit.edu/6.828/2018/tools.html

### Test Your Compiler Toolchain

```
% objdump -i
% gcc -m32 -print-libgcc-file-name
```

执行上面两个命令, 都正常.

因为虚拟机是 64-bit, 需要安装 32-bit 的库.

```
% sudo apt-get install gcc-multilib
```

### QEMU Emulator

qemu 的安装与 2017年区别很大.

1. 下载 qemu

   ```
   git clone https://github.com/mit-pdos/6.828-qemu.git qemu
   
   ps. 如果网速比较慢的话可以使用镜像
   git clone https://github.com/mit-pdos/6.828-qemu.git qemu
   git clone https://hub.nuaa.cf/mit-pdos/6.828-qemu.git qemu
   ```

2. 配置

   ```
   ./configure --disable-kvm --disable-werror --target-list="i386-softmmu x86_64-softmmu"
   ```

   与 2017年的区别是增加了 "--disable-werror", 否则会有编译错误

3. 编译及安装

   ```
   sudo make
   sudo make install
   ```

### JOS

jos 代码也用 2018 lab1 的, 使用老的 2017 的, 运行 make qemu 总是出现"Triple fault.  Halting for inspection via QEMU monitor"

```
git config --global http.sslverify false	// 解决 Certificate verification failed
git clone https://pdos.csail.mit.edu/6.828/2018/jos.git lab
```

