# Exceptional Control Flow

程序控制流是指程序的运行过程, 程序每次取指执行, 也即顺序执行. 如果希望改变原来的顺序控制流, 一般有两种方式

1. 使用指令改变程序的控制流
* jumps and branches
* call and return
这种改变的是程序的状态

2. 处理系统状态的变化
* 数据从磁盘读取
* 除以0
* 使用 ctrl + c 等系统中断

**ECL 异常控制流存在于系统的所有层级中**

![](images/8-Exceptional%20Control%20Flow-系统流.png)

# Exception

异常是为了响应一些事件而将控制转交给操作系统内核的一种方式

![](images/8-Exceptional%20Control%20Flow-异常是什么.png)

## 异常表

针对每一个异常, 系统会给其进行一个编号, 存储在异常表中. 当 `k` 异常触发时, 硬件会从异常表中找到异常处理程序的地址, 之后转向这个处理程序

## 异常的分类

![](images/8-Exceptional%20Control%20Flow-异常的分类.png)

* 同步异常
* 异步异常

### 异步异常

异步异常是由处理器外部状态变化引起的, 一般称为**中断**, 这种需要设置异常引脚, 在中断处理后, 会跳回指令, 不会影响程序的运行

最常见的中断: `定时器中断`

### 同步异常

1. Traps

Traps 是由程序调用而故意出现的异常, 最常见的 Traps 是系统调用 (system call) , 系统调用可以将用户态一些无权限进行的操作转到内核态进行处理

2. Faults

Faults 是无意但可能恢复的异常, 有些可以恢复, 有些不能恢复. 对 Faults 的处理要么是重新执行指令, 要么是停止程序的执行

3. aborts

aborts 是无意但不可恢复的异常, 只能中止程序的进行

#### system call

![](images/8-Exceptional%20Control%20Flow-系统调用的类别.png)

![](images/8-Exceptional%20Control%20Flow-打开文件.png)

这里将 `system call` 的序号传递给  `%rax` ,然后调用 `syscall` 读取文件, 返回一个文件描述符 `fd` 

这个过程调用完异常处理程序后是执行的下一条指令

#### Fault Example

![](images/8-Exceptional%20Control%20Flow-faultexample.png)

初始化数组并没有真的分配一块数组空间, 当进行访问时, 会出现 **页错误** , 操作系统会将数据从磁盘拷贝到内存中, 之后返回.

这里处理完异常后是重新执行了 `movl` 指令

![](images/8-Exceptional%20Control%20Flow-faultexamp1.png)

当访问错误的内存时, 程序无法恢复, 直接调用系统的信号, 中止程序

# Process

进程就是正在运行的程序实例

进程提供了两个关键的 **抽象**
1. 逻辑控制流:
每个程序好像是在互斥的使用CPU, 由内核提供的服务称为 **上下文的切换**

2. 提供了私有的地址哦空间:
每个程序似乎用于互斥的内存空间进行使用, 由内核提供的服务称为 **虚拟内存**

## 多进程是如何实现的?

### 传统方式的实现

在进行进程切换的时候
1. 将当前寄存器的数据存储进入内存中进行保护
2. 切换到下一个待执行的进程
3. 从内存中加载上次运行时寄存器的信息, 将其对应的地址也进行切换, 这里的地址空间和寄存器的值就称为 **上下文**
4. 开始执行指令

### 现代方式的实现

1. 多个CPU核心进行处理
2. 共享主空间和缓存
3. 独立执行任务

当然, 在每个CPU核心处理时也会使用传统方法针对进程进行切换

##  并发(concurrent)和顺序(sequential)

并发是指两个进程时序在时间上是重叠的, 否则是顺序的

![](images/8-Exceptional%20Control%20Flow-并发.png)

图中, A & B 是并发的, 因为两个交替执行, B & C是顺序的, 因为 B 执行完后 C 才执行, B和C之间不存在时间重叠的可能

## 上下文的切换

![](images/8-Exceptional%20Control%20Flow-上下文的切换.png)

上下文的切换只可能在内核中执行.

**内核** 不是一个正在运行的独立进程,  它始终在上下文的环境中运行, 它是位于地址空间顶部的一段代码

## 进程控制流

**!!!** 在使用Linux系统提供的系统调用函数时, 要始终检查这些函数的返回值, 如果出现问题, 则会将错误代码存放到全局变量 `errno` 中

![](images/8-Exceptional%20Control%20Flow-系统调用检查.png)

一种简单的处理程序

![](images/8-Exceptional%20Control%20Flow-简单的处理程序.png)

使用包装器 `wrappers` 进行错误的处理, 包装器实际就是一个函数, 它具有与原来接口相同的参数和返回值 ,只是将所有从处理程序都放入其中了(一般名字相同, 首字母大写).

![](images/8-Exceptional%20Control%20Flow-包装器.png)

* `pid_t getpid(void)` 获得当前进程的 `pid`
* `pid_t getppid(void)` 获得父进程的 `pid`
## 进程的状态

* running 进程正在执行或等待被调度
* stopped 进程暂停执行, 或暂时不会被调度
* terminated 进程中止

进程的终止由三种可能
1. 得到信号终止
2. main执行结束终止
3. 调用 `exit` 函数终止

* 中止进程 `void exit(int status)` 返回 0 表示正常, 其余为异常
无论只要调用了 `exit` 只会返回一次

## 创建进程

从一个父进程创建一个子进程调用的函数称为 `fork` , 如果 `fork` 返回值为 0 的进程是子进程, 返回值非零的进程为父进程, 返回 `-1` 表示错误

子进程与父进程几乎是独立的
* 子进程的内存空间是对父进程内存空间的一个拷贝(由于是虚拟内存, 寄存器和内存地址的值虽然是相同的, 但是指向的物理内存位置是不同的)
* 子进程复制父进程打开的文件描述符, 因此子进程可以访问父进程打开的文件
* 子进程的 PID 与父进程不同

* `pid_t fork(void)` 创建一个进程
`fork` 调用一次, 返回两次(父进程和子进程的 `pid`), `fork` 没有办法控制父子进程的执行顺序

![](images/8-Exceptional%20Control%20Flow-fork的例子.png)

### 进程图(Process Graphs)

进程图是用来表示进程顺序的一种图

![](images/8-Exceptional%20Control%20Flow-进程图.png)

![](images/8-Exceptional%20Control%20Flow-进程图的解释.png)

![](images/8-Exceptional%20Control%20Flow-更多的fork.png)

![](images/8-Exceptional%20Control%20Flow-嵌套fork父进程图.png)

![](images/8-Exceptional%20Control%20Flow-嵌套fork子进程图.png)

## 子进程的回收

当进程终止后, 它等待系统的回收(Reap). 如果一个父进程创建了一个子进程, 那么最好是父进程等待子进程执行完毕后再退出. 原因在于当进程终止后, 其不会消失, 还保留了一些系统的资源, 比如退出状态, OS表等, 这样的进程就称为僵尸进程

**回收**: 是父进程通过 `wait` 或者 `waitpid` 函数处理终止的子进程的一种方式, 系统内阁通过父进程的执行结果删除僵尸进程

如果父进程没有回收僵尸进程会怎样? \
内核会将子进程挂在进程号为1的系统进程中 `init` 进程, `init进程` 会负责回收这些子进程

![](images/8-Exceptional%20Control%20Flow-僵尸进程的举例.png)

![](images/8-Exceptional%20Control%20Flow-子进程不停止.png)

* `int wait(int *child_status)` 用于与子进程之间同步,
	* 父进程通过 `wait` 函数回收子进程.
	* `wait` 会是当前进程暂停, 直到有一个子进程终止
	* 返回值是终止子进程的 `pid`
	* 如果 `child_status != NULL` 那么, 这个指针指向的整数将被设置为某个值, 这个值标志着子进程终止的原因与退出状态(**相当于一个返回值**), 这些返回值被定义在头文件 `wait.h` 中

![](images/8-Exceptional%20Control%20Flow-子进程状态的宏定义.png)

![](images/8-Exceptional%20Control%20Flow-wait的例子.png)

![](images/8-Exceptional%20Control%20Flow-复杂的例子.png)

* `pid_t waitpid(pid_t pid, int &status, int options` 
	* 等待某个进程(`pid`) 的结束

![](images/8-Exceptional%20Control%20Flow-waitpid的例子.png)

`int execve(char *filename, char *argv[], char *envp[])`

![](images/8-Exceptional%20Control%20Flow-execve.png)

* 执行 `filename` 对应的程序(二进制或者脚本文件) 
* `argb[0]` 恒等于 `filename`
* `envp` 是环境变量列表

![](images/8-Exceptional%20Control%20Flowexecve1.png)

这个函数对应的栈帧如下图:

![](images/8-Exceptional%20Control%20Flow-execve2.png)

`execve` 创建一个新的栈, 这个栈顶是 `main` 函数, 然后向上生长, 保留一些填充, 然后是传入的指针.

传入的第一个参数是 `argc`, 被放入`%rdi` 中, 第二个参数是 `argv` 的首地址被放入 `%rsi` 中, 第三个参数是 `envp` 的首地址, 被放入 `rdx` 中

`execve` 实际上是将当前进程替换成新的要执行的进程, 下图展示了其调用过程:

![](images/8-Exceptional%20Control%20Flow-execve3.png)

* 读入二进制文件
* 将旧的内存 `free` 掉, 并创建新的内存
* 将段数据进行替换 , `PC` 指针指向代码段的开始位置, 重新执行这个进程, 但是这个过程中, **进程的pid没有变化**

## 为什么要使用 fork + execve的方式而不是有一个专门的接口实现直接创建一个执行某个功能的子进程?

在一些情况下, `fork`  的作用就非常有用了, 比如如果执行相同的代码, 那么fork 就已经可以完成任务了, 不需要再有更加复杂的功能了, 而如果需要执行其他的代码, 那么就只是再添加一个 `execve` 的函数而已, 这使得不同的任务能发挥不同的作用


