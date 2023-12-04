# shell

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-进程结构.png)

`shell` 也是一个程序, 这个程序一直在读取和执行用户的输入

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-shell1.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-shell2.png)

但是上面的程序存在一个问题, 这个程序只对 `!bg` 的子进程进行了维护, 如果这个子进程是 `bg` 进程, 那么它退出后不会被回收, 这容易导致内存的泄露, 如何解决?

使用:
# Signal

`signal` 是一个信息, 它通知一个进程在系统中发生了一些事件, 这与 exception 类似,但是是软件实现的.

signal 只是一个整数 `id`, 其具有的信息并不多, 但是也足够使用了

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-信号.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-信号表.png)

## 发送一个信号

内核通过更新进程上下文中的某些状态发送(`send/delivers`)信号到进程中, 除了进程上下文中的一些位改变外, 没有发生其他变化

内核发送信号有以下两个原因
* 内核检测到了一些系统事件, 比如除以0, 子进程结束
* 其他进程发送了请求, 让内核发送一个信号给另一个进程

## 接收一个信号

一个进程在接收一个信号后, 内核会强制其对这个信号进程处理

常见的处理方式有三种
* 忽略这个信号
* 结束这个进程
* 捕捉这个信号, 并进行处理

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-信号处理过程.png)

一个信号是待处理待处理的(pending), 意味着这个信号发送了但是还没有接收
* 在同一时刻, 每一种具体类别的信号只能有一个待处理
* 信号不存在排队的说法, 后来的(相同类型的)信号会被丢弃

一个进程可以阻塞某些信号的接收
* 进程不能阻止信号的传输, 但可以组织信号的响应


## pending/blocked bits

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-pending&blocked.png)

因为pending 和 blocked 位只有一个 ,对信号的处理也只是设置这个位, 所以信号太多也不会有影响

## 进程组

每个进程都属于一个进程组
* `getpgrp()` 返回当前进程所在的进程组
* `setpgrp()` 改变当前进程的进程组

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-进程组.png)

## 如何发送信号
`/bin/kill` 程序可以向一个进程/进程组发送信号
* `kill -id pgrpid` 向进程组发送信号 
* `kill -id -pid` 向进程发送信号

也可以使用键盘发送信号

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-键盘信号.png)

* `fg` 使前台进程继续进行

使用 `kill` 函数进行信号的发送

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-kill函数.png)

* `kill(pid_t pid, signal)` 向某个进程发送信号

## 如何接收信号

在进行进程切换后, 将控制权返回用户代码前, 内核会检查所有待处理的信号, 内核会计算位向量 `pnb`. 如果pnb是0, 那么会返回控制权, 否则选择最小的非零位 `k`, 然后强制进程处理这个信号 `k`, 然后重复这个操作, 最后把控制权还给进程

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-如何处理信号.png)

## 修改信号的处理方法

使用函数 `signal`
```c
handler_t *signal(int signum, handler_t *handler)
```

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-信号处理.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-信号处理例子.png)

### 信号处理是并发的

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-信号并发.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-并发例子.png)

**只有在进程调度切换的时候才会检查信号位是否被设置, 是否需要进行信号处理!!!**

嵌套信号处理
![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-嵌套信号处理.png)

### 阻塞信号

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-阻塞信号.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-阻塞信号的例子.png)

## 安全的信号处理

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-安全信号处理.png)

一些guideline
![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-guidelines.png)

### 异步信号安全

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-异步信号安全.png)

一个异步信号安全的函数, 要么是**可重入**的, 要么是**不可被信号打断的**

#to_explained
**reentrant 可重入**, 后续会讲. 类似于可以中途处理信号/中断的函数

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-一些可重入的解释.png)

不安全的函数可能会导致 **死锁** 

以 `printf` 为例,  `printf` 的访问是有锁的, 当进行 `printf` 运算, 就会限制其他的进程使用 `printf`, 如果在调用 `printf` 时进入了中断/进行信号响应, 且处理程序中也有 `printf`, 那么处理程序中的 `printf` 就会因为拿不到锁而等待, 原函数因为等待处理函数进行而等待, 这就导致了死锁

所以可重入和不被信号打断必须满足至少一条!

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-正确的信号处理程序.png)

上述例子说明, 不能通过信号的数量进行信号的处理, 因为信号本身不会排队, 所以重复信号会被丢弃

因此要使用其他的方法

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-正确的处理.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-慢系统调用.png)

某些指令, 如`read`, 会等待信号/中断到来才实际执行, 这称为慢系统调用. 所以有时候需要挂载两次!

## 同步流避免竞争

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-同步1.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-同步2.png)

**但是!**, 这个程序存在问题, **子进程可能在父进程 `addjobs` 之前就处理完成了, 那么就永远不会删除这个job! 导致内存浪费** 

改进:
![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-改进.png)

虽然无法控制子进程如何运行, **但是可以控制信号处理程序如何运行**

这里首先是 blocked 父子进程的信号, 当子进程运行时, 会解除信号限制, 在子进程运行完毕后会发送信号, 此时有两种情况
1. 父进程还没运行 \
由于父进程还没有运行, 所以子进程发出的信号父进程不会处理, 直到父进程运行 `addjob` 后运行处理才会处理
2. 父进程已经运行了 \
那么就继续运行, 没有问题

## 信号的显式 `wait`

在上面程序中, 我们在信号的处理程序中而不是主程序中执行的 `wait` 操作, 那么, 如何确保前台的工作已经完成了?

一种方法是使用全局变量

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-法1-1.png)

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-法1-2.png)

这里只要等待 `pid` 为0即可

这种情况导致了资源浪费!

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-改进方法1.png)

如果使用 `pause`, 如果在 `pause` 前就已经有信号了, 那么就会导致 `pause` 始终等待

更好的解决办法: 
* `sigsuspend` 

![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-更好的解决办法.png)



![](images/9-Exceptional%20Control%20Flow-signals%20and%20nonlocal%20jump-解决办法.png)


#to_study
# nonlocal jumps

C语言提供了一种用户级的异常控制流形式，称为**非本地跳转**（nonlocal jump），它可以将控制流直接从一个函数转移到另一个当前正在执行的函数，而不需要经过正常的调用和返回序列。而这些都是通过两组函数实现的。

## setjmp函数

```c
#include <setjmp.h>

int setjmp(jmp_buf env);						//返回0
int sigsetjmp(sigjmp_buf env, int savesigs);	//返回0		
```

* setjmp函数在env缓冲区中保存当前的调用环境（PC值、sp值及通用寄存器），以供后面的longjmp使用，并返回0。
* setjmp的返回值不能用来赋值给变量，但却可以作为判断语句（switch、if）的条件。
* sigsetjmp是设计用来被信号处理程序使用的，它的作用和setjmp类似。savesigs用来保存当前环境下的信号状态；

## longjmp函数

```c
#include <setjmp.h>
void longjmp(jmp_buf env, int retval);		//从不返回
void siglongjmp(jmp_buf env, int retval);	//从不返回
```

- longjmp函数从env缓冲区中恢复最近一次setjmp保存的调用环境，然后控制流跳转到该setjmp函数处，并使函数返回**非零**的retval。
- setjmp被调用一次，但可以返回多次；而longjmp被调用后却从不返回，或者说它从最近的setjmp处返回。
