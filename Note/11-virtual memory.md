# 一个使用物理地址寻址的系统

![](images/11-virtual%20memory-物理地址寻址.png)

# 一个使用虚拟地址进行寻址的系统

![](images/11-virtual%20memory-虚拟地址寻址.png)

# 虚拟化
当你虚拟化一个资源时, 你向该资源的用户显示该资源不同的视图, 你通过介入这种资源的访问过程实现这一点

# 地址空间

![](images/11-virtual%20memory-地址空间.png)

# 为什么需要虚拟内存
* 能够更加有效的使用主存: 使用DRAM作为cache称为虚拟地址的一部分
* 简化内存的管理, 每个进程得到统一的线性地址空间
* 独立地址空间:每个进程保证访问地址的私有化


# VM作为缓存工具

![](images/11-virtual%20memory-缓存工具.png)

虚拟内存被划分成了很多**页(Pages)**. 这些页的一部分指向了物理DRAM(DRAM很小, 作为缓存), 有一些映射告诉我们哪些页面已经被缓存

![](images/11-virtual%20memory-chache的组织.png)

## 页表

页表是一个记录页入口的表, 这个表是虚拟页和物理页的一个映射, 由内核维护, 是进程上下文的一部分

![](images/11-virtual%20memory-页表.png)

如上图所示: 页表中的一些指向的是DRAM缓存(标蓝色, 已缓存), 一些指向的是硬盘(标灰色, 未缓存), 还有的未分配(标白色, null)

### 页击中(page hit)

访问地址被转化到DRAM上的情况称为**页击中**, 此时返回的是DRAM的物理地址 

### 页错误(Page Fault)

没有发生页击中时的情况, 此时就需要牺牲DRAM中的一页去读取磁盘中的数据, 使磁盘中的数据进入缓存, 再由CPU访问

![](images/11-virtual%20memory-页错误1.png)

![](images/11-virtual%20memory页错误2.png)

![](images/11-virtual%20memory-页错误3.png)

![](images/11-virtual%20memory页错误4.png)

上面四张图描述了页错误的处理方法
* Page miss 触发异常
* 选择一块缓存页, 将其与硬盘中的数据交换
* 更新页表的指向
* CPU读取数据

## 分配内存页

页表内分配一块空间, 使其指向硬盘中的地址

![](images/11-virtual%20memory-分配页.png)

## 程序的局部性使得有效

![](images/11-virtual%20memory-局部性的影响.png)

# 虚拟内存作为内存管理方法

每个进程都有自己的页表, 这个页表可以映射到物理空间的所有位置

![](images/11-virtual%20memory-vm-内存管理.png)

提高了物理内存的使用率, 减少了内存碎片的产生

另外,虚拟内存还可以实现共享内存, 只需要使两个进程的页表指向相同的物理内存即可, 这就是shared library的原理

![](images/11-virtual%20memory-有点.png)

![](images/11-virtual%20memory简化链接和加载.png)

# VM帮助内存保护

![](images/11-virtual%20memory-内存保护.png)

设置一些保护的位, 在MMU访问地址时, 首先检查这些位, 之后根据权限执行下一步操作

# 地址翻译

![](images/11-virtual%20memory-地址翻译.png)

![](images/11-virtual%20memory-翻译的符号.png)

## 页表如何使用

![](images/11-virtual%20memory-页表如何使用.png)

 实际上, 虚拟地址的后 `p` 位对应的就是偏移量, 这与物理地址的偏移量是一致的, 而虚拟地址的前 `n-p` 位是页表中的地址, 用于从页表中查表

![](images/11-virtual%20memory-pagehit的访问过程.png)

`PTEA`是页表的地址, `PTE`是页表, `PA`是物理地址

![](images/11-virtual%20memory-pagefault的访问.png)

![](images/11-virtual%20memory-页表缓存的访问.png)

## TLB

TLB是硬件缓存, 它存储了最近访问的PTE表地址

![](images/11-virtual%20memory-TLB.png)

![](images/11-virtual%20memory-Access%20TLB.png)

### TLB HIT
![](images/11-virtual%20memory-TLBHit.png)

![](images/11-virtual%20memory-TLB-miss.png)

# 多级页表

如果只有一个页表, 那么页表的就实在太大了

使用多级页表可以减小页表的尺寸

![](images/11-virtual%20memory-多级页表.png)

多级页表的查询

![](images/11-virtual%20memory-多级页表的查询.png)



![](images/11-virtual%20memory-简单的TLB.png)

![](images/11-virtual%20memory-翻译2.png)

![](images/11-virtual%20memory-翻译3.png)

# Intel Memory System

![](images/11-virtual%20memory-英特尔缓存.png)

![](images/11-virtual%20memory-端到端的地址翻译.png)

![](images/11-virtual%20memory-PTE.png)

![](images/11-virtual%20memory-l4pet.png)

![](images/11-virtual%20memory-整个翻译过程.png)

![](images/11-virtual%20memory-提高L1速度的方法.png)

`CT` :Cache Tag, `CI`: Cache Index, `CO` : Cache Offset

# frok和execve是如何执行的

![](images/11-virtual%20memory.png)

![](images/11-virtual%20memory-1.png)

# Memory map

![](images/11-virtual%20memory-2.png)


![](images/11-virtual%20memory-共享对象的实现.png)

![](images/11-virtual%20memory-.png)

#to_study 
copy-on-write
在读(相同空间)的时候使用相同的空间, 在写的时候执行复制操作, 这样减少了资源的使用

![](images/11-virtual%20memory-3.png)

![](images/11-virtual%20memory-4.png)

# mmap系统调用
* `void *mmap(void* start, int len, int prot, int flags, int fd, int offset)`

![](images/11-virtual%20memory-5.png)

![](images/11-virtual%20memory-6.png)





