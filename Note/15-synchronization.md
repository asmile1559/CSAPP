# join和fork的工作模式

在某一时间，一个进程f包含很多个线程（fork），而这些线程中存在一个线程，它是用来管理其他线程的，当其他线程运行完成退出后，这个线程才会继续向下运行，这个等待的过程就称为join

# 同步

控制进程交错的过程称为同步

# 共享的变量：

一个变量x是被共享的，当且仅当多个线程引用了这个变量的一些实例

shared 的几个问题：

1. 线程的内存模型是什么？

每个线程都有自己独立的线程上下文内容，包括：
* 线程号
* 栈
* 栈指针
* PC
* 状态码
* 通用目的寄存器

其余的内容都由线程共享

但是实际上并不是如上严格执行的，由于它们的空间是相同的，他们之间可以相互访问对方的栈，这就导致了问题

![](images/15-synchronization.png)

这里线程里的ptr指向的是主线程中的msgs，实际上是其他栈中的数据，可以访问说明这里访问了其他栈的内容，这需要注意！

2. 如何将变量映射到内存

![](images/15-synchronization-1.png)

那么哪些是共享的？（判断共享要根据是否被多个线程引用！！！）

* ptr
* cnt
* msgs

## 共享数据的访问必须要小心

![](images/15-synchronization-2.png)

![](images/15-synchronization-3.png)

![](images/15-synchronization-4.png)

![](images/15-synchronization-5.png)

为什么会出现问题？**临界区交叉！**

# 过程图

![](images/15-synchronization-6.png)

![](images/15-synchronization-7.png)

**不安全区是使临界区交错的区域**

![](images/15-synchronization-8.png)

![](images/15-synchronization-9.png)

解决办法：

# 强制互斥！

![](images/15-synchronization-10.png)

# 信号量

信号量是一个非负的全局整数，对信号量由两个操作
* P操作，如果信号量s不为零，那么s减少1，如果为0，则挂起该线程，直到s不为零，重启后之后s再减1
* V操作，增加1，原子操作。操作完检查是否有现成被P操作阻塞，如果有，那么就启动其中一个线程

**信号量的变性：信号量始终大于等于0**

![](images/15-synchronization-11.png)

![](images/15-synchronization-12.png)

![](images/15-synchronization-13.png)

为什么互斥锁可以发挥作用？

![](images/15-synchronization-14.png)

# 生产者消费者模型

![](images/15-synchronization-15.png)

生产者消费者模型需要三个信号量
* mutex互斥锁，用于进行临界区保护
* slots信号量，保存slots的数量
* items信号量，用来保护item是数量

![](images/15-synchronization-16.png)

![](images/15-synchronization-17.png)

![](images/15-synchronization-18.png)


![](images/15-synchronization-19.png)

![](images/15-synchronization-20.png)

# 读写问题

![](images/15-synchronization-21.png)

只对修改加锁，对读取不关注

![](images/15-synchronization-22.png)

![](images/15-synchronization-23.png)

reader中的PV运算是做什么的？只是为了保护readcnt，并不会影响reader功能的实现

![](images/15-synchronization-24.png)

使用生产者-消费者模型可以减少建立线程和销毁线程的大量重复开销

![](images/15-synchronization-25.png)

![](images/15-synchronization-26.png)

![](images/15-synchronization-27.png)

![](images/15-synchronization-28.png)

pthread_once 就是只执行一次后面的操作，这里是对所有的线程，只执行一次初始化echo cmt的操作

# 线程安全

一个线程函数最好调用线程安全的函数

一个函数是线程安全的，当且仅当它再没多个并发的线程访问时总是可以得到正确的结果

有四类函数是线程不安全的
* 函数没有保护共享变量（class 1）
* 函数在多线程之中保存状态（class 2）
* 函数返回一个指向静态变量的指针（class 3）
* 被称为线程不安全的函数（class 4）

## 第一类函数

通过添加信号量解决

## 第二类函数

![](images/15-synchronization-29.png)

解决办法：重写

![](images/15-synchronization-30.png)

## 第三类函数

![](images/15-synchronization-31.png)

解决方法：
1. 重写函数读入调用者传递的变量地址来存储结果
2. 对原函数进行包装，传入参数用于复制输出

## 第四类函数

不调用线程不安全的函数

# 可重入函数

一个函数是可重入的，当且仅当它不含共享变量

![](images/15-synchronization-32.png)

# 线程安全函数

1. 所有C标准库中的函数都是**线程**安全的
2. 大部分unix系统函数都是线程安全的，除了
![](images/15-synchronization-33.png)

# 竞争

![](images/15-synchronization-34.png)

传递地址容易引起竞争，更好的方法是传递**值**，或者进行内存的分配

![](images/15-synchronization-35.png)

# 死锁

一个进程进入死锁，当且仅当他的等待状态永远不会结束！

![](images/15-synchronization-36.png)

![](images/15-synchronization-37.png)

![](images/15-synchronization-40.png)

以次昂同的顺序获取锁和释放锁

![](images/15-synchronization-41.png)





