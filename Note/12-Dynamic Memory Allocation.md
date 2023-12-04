程序使用动态内存分配器去在运行时获取虚拟内存， 动态内存分配器获取的虚拟内存被称为“堆”

目前为止，有两种常见的内存分配方式
* 显式分配：`malloc`, `free`
* 隐式分配 用户分配内存，系统具有垃圾回收机制

`void *malloc(size_t size)` 成功返回指针，失败返回NULL
`free(void *p)` 

# allocation！

![](images/12-Dynamic%20Memory%20Allocation-allocation如何进行.png)

1. 上图显示了分配的过程，这些运算在堆中进行
2. 在进行内存分配时需要保证“**双字对齐**”， 要求起始地点为偶数， 这样方便寻址

## malloc存在的问题
* malloc不能控制分配块的大小
* malloc必须立即响应用户的请求
* malloc只能从未分配的空间中取块->malloc不能移动分配的块,不能压缩块
* 在进行free时，必须使用之前调用malloc返回的指针

## 内存分配的评价表现:
1. 吞吐量
2. 峰值内存利用率

### 吞吐量(Throughput)

吞吐量就是在单位时间里强求的次数,例如在10秒内进行了5k次malloc和free, 那么吞吐量就是1000 运算/秒

吞吐量用来评价malloc的运算速度

### 峰值内存利用率(Peak Memory Utilization)

衡量malloc效率有多高, 例如使用malloc分配10字节的数据, 真正的有效载载是10字节,其余的占用属于开销

有效载荷的总和除以堆大小的最大值就称为峰值内存利用率

![](images/12-Dynamic%20Memory%20Allocation-23.png)
#### 碎片Fragmentation
碎片分为两类
* 内部碎片
* 外部碎片

内部碎片是指有效载荷小于块的大小

![](images/12-Dynamic%20Memory%20Allocation.png)

外部碎片是指堆中有足够内存时,但是没有满足条件的区域进行分配

![](images/12-Dynamic%20Memory%20Allocation-1.png)

# 建立一个allocator

![](images/12-Dynamic%20Memory%20Allocation-2.png)

![](images/12-Dynamic%20Memory%20Allocation-3.png)

## 知道需要free多少空间

![](images/12-Dynamic%20Memory%20Allocation-4.png)

多分配一个空间用于记录块的大小

## 如何追踪空闲的块

方法1: 使用隐式列表

![](images/12-Dynamic%20Memory%20Allocation-5.png)

无论是分配了还是没分配,都在块的开头留一个字节保存内存空间的大小

方法2: 使用显式列表,存放一个指针指向下一个空闲的空间

![](images/12-Dynamic%20Memory%20Allocation-6.png)

![](images/12-Dynamic%20Memory%20Allocation-7.png)

方法3: 多个空闲列表 \
方法4:使用平衡二叉树

![](images/12-Dynamic%20Memory%20Allocation-8.png)

使用字节对齐来进行实现隐式列表, 因为使用malloc进行分配的数据都是 8bit 的倍数,所以分配的size后3位比如 `8->1000, 16->10000, 24->11000` 都是0, 这样,就可以用后三位表示是否分配, 在计算size时, 只需要把后三位屏蔽掉即可

![](images/12-Dynamic%20Memory%20Allocation-9.png)

图中, 分配空间最后一字节会留有一个标志着结尾的块, 这个块在后面的操作中有很大帮助

## 寻找空闲的块

![](images/12-Dynamic%20Memory%20Allocation-10.png)

有三种搜索方式
* 从头开始搜索:从堆的起始位置搜索
* next搜索:从上一次分配的位置处搜索
* 最佳匹配搜索:寻找最接近分配量的位置,可能分配效率降低了,但是提高了空间利用率

## 分块split

![](images/12-Dynamic%20Memory%20Allocation-11.png)

## 如何释放一个块

![](images/12-Dynamic%20Memory%20Allocation-12.png)

最简单的形式:直接释放,但是会导致外部碎片

**需要合并空闲的块**

![](images/12-Dynamic%20Memory%20Allocation-13.png)

那么如何合并前置的块?

![](images/12-Dynamic%20Memory%20Allocation-14.png)

**进行双向搜索!**

在分配的空间尾部留一个可以记录分配长度的数据, 可以通过这个数据向前追溯

![](images/12-Dynamic%20Memory%20Allocation-15.png)

boundary tag的缺点: 产生了内部碎片

如何提高效率?

仍然使用字节对齐, 空余的三位中,一位标识该块是否是空闲的, 一位表示前面的块是否是空闲的(只对分配的空间存在这样的设定, 未分配的空间仍然有header和footer)

![](images/12-Dynamic%20Memory%20Allocation-16.png)

![](images/12-Dynamic%20Memory%20Allocation-17.png)

![](images/12-Dynamic%20Memory%20Allocation-18.png)

![](images/12-Dynamic%20Memory%20Allocation-19.png)

![](images/12-Dynamic%20Memory%20Allocation-20.png)

## 分配政策

![](images/12-Dynamic%20Memory%20Allocation-21.png)

## 隐式列表

![](images/12-Dynamic%20Memory%20Allocation-22.png)

## 显式空闲列表

![](images/12-Dynamic%20Memory%20Allocation-24.png)

对一个分配的空间, 其形式与原有形式一样, 包括head和footer, 但是对于空闲的块, 设置了两个指针, 其中一个指针用于指向前一块空闲空间, 另一个指针指向后一块空闲空间

![](images/12-Dynamic%20Memory%20Allocation-25.png)

如何分配显示列表?

![](images/12-Dynamic%20Memory%20Allocation-26.png)

**如何free空间!**

常见的方法\
1. LIFO政策.将free的块放入free list的开始位置
* 优点是简单和常数级别的操作时间
* 缺点是会产生大量的碎片

![](images/12-Dynamic%20Memory%20Allocation-27.png)

![](images/12-Dynamic%20Memory%20Allocation-28.png)

![](images/12-Dynamic%20Memory%20Allocation-29.png)

![](images/12-Dynamic%20Memory%20Allocation-30.png)

2. 地址顺序政策, 将空间插入到地址之间的空闲块中,满足当前块地址大于前一块地址小于后一块地址
* 优点是碎片更少
* 缺点是需要查找时间

![](images/12-Dynamic%20Memory%20Allocation-31.png)

## 创建隔离的空闲列表

思想是每一类不同尺寸的空闲块都有属于自己的列表

![](images/12-Dynamic%20Memory%20Allocation-32.png)

![](images/12-Dynamic%20Memory%20Allocation-33.png)

![](images/12-Dynamic%20Memory%20Allocation-34.png)

## 垃圾回收机制

![](images/12-Dynamic%20Memory%20Allocation-35.png)

![](images/12-Dynamic%20Memory%20Allocation-36.png)

![](images/12-Dynamic%20Memory%20Allocation-37.png)

将内存看作有向图, 每个节点对应一个块,每条边对应一个指针

具体做法是:
1. 使用一个额外的标记位(例如上面提到的剩余三个标记位的第三个), 从根节点开始标记,直到标记完所有的结点,
2. 从堆的开始进行扫描, 找到未被标记的接待你,这些结点就是垃圾

![](images/12-Dynamic%20Memory%20Allocation-38.png)

上图中,黑框没有标记的部分被free了

![](images/12-Dynamic%20Memory%20Allocation-39.png)

![](images/12-Dynamic%20Memory%20Allocation-40.png)

## 内存操作的危险与漏洞

![](images/12-Dynamic%20Memory%20Allocation-41.png)

![](images/12-Dynamic%20Memory%20Allocation-42.png)

`(), [], ->, .` 的优先级要高于 `*, &` !!!

```c
int *p; // p 的右边没什么,左边是指针符号,所以p是指针
int *p[]; // p 的左右都有运算符,但是右边运算符优先级更高,所以先考虑右边,p是一个数组,这个数组存放的内容是 int *指针
int *(p[]); //同上
int **p; // p是一个指针, 指向的是一个指针
int (*p)[]; // 由于()优先级高于[], 所以p是一个指针,这个指针指向的是 int [] 数组
int *f(); // 由于()优先级高于*, 所以f是一个函数,这个函数返回一个 int *类型的指针
int (*f)(); // 由于()平级, 左移从左向右看, f是一个指针,这个指针指向一个函数
int (*(*f())[])(); // f是一个函数,这个函数返回一个指针, 这个指针指向一个数组,这个数组存储返回值为 int *类型的函数
int (*(*x[])())[5]; // x是一个数组,这个数组存储指针类型,这个指针指向一个返回指针的函数,返回的指针指向一个 int [5] 的数组
``` 

![](images/12-Dynamic%20Memory%20Allocation-43.png)

## 以下都是错误的例子!!!!

![](images/12-Dynamic%20Memory%20Allocation-44.png)

![](images/12-Dynamic%20Memory%20Allocation-45.png)

![](images/12-Dynamic%20Memory%20Allocation-46.png)

![](images/12-Dynamic%20Memory%20Allocation-47.png)

![](images/12-Dynamic%20Memory%20Allocation-48.png)

![](images/12-Dynamic%20Memory%20Allocation-49.png)

![](images/12-Dynamic%20Memory%20Allocation-50.png)

![](images/12-Dynamic%20Memory%20Allocation-51.png)

![](images/12-Dynamic%20Memory%20Allocation-52.png)

![](images/12-Dynamic%20Memory%20Allocation-53.png)

![](images/12-Dynamic%20Memory%20Allocation-54.png)

![](images/12-Dynamic%20Memory%20Allocation-55.png)
