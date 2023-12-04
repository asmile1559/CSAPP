# 0. 准备

```bash
# 程序源码
wget http://csapp.cs.cmu.edu/3e/shlab-handout.tar
# 讲义
wget http://csapp.cs.cmu.edu/3e/shlab.pdf
```

# 1. 基础知识

## fork函数

```c
#include <unistd.h>
#include <sys/types.h>
/*
* 创建一个子进程
* 
* args：
*     void
* return (int)：
*     pid(p), 0(c): 执行成功返回两次，父进程返回子进程的pid，子进程返回0
*     -1: 执行失败返回-1，并不创建子进程
*/
pid_t fork(void);
```

## execve函数

```c
#include <unistd.h>

extern char **environ;   /* defined in libc */

/*
* 执行指定的程序，使其替代该进程，因为进程被替换了，所以不会存在返回结果
* execve执行成功后，其后部分的代码也不会执行
* 
* args：
*     char *filename：可执行文件路径
*     char *argv[]：执行参数，以NULL结尾
*     char **environ：环境变量
* return (int)：
*     -1: 执行失败返回 -1
*    void: 执行成功不返回
*/
int execve(char *filename, char *argv[], char *const environ[]);
```

## waitpid函数

```c
#include <sys/types.h>
#include <sys/wait.h>

/*
* 等待进程号为 pid 的子进程,用于父进程回收子进程
* 注意,waitpid只能等待一个子进程,当存在多个子进程时,需要对所有的子进程进行注册
*
* args:
*     pid_t pid:子进程的pid, 如果pid>0, 那么等待集合是一个单独的子进程,如果pid=-1,那么等待集合是该父进程创建的所有子进程
*     int *statusp: 一般为NULL,用来接收waitpid的返回结果,其存在对应的宏,不过不常使用
*     int options: 一般用于设置等待方式, WNOHANG->不阻塞等待,可以表示后台进程, WUNTRACED->等待直到运行完成,可以表示前台进程,宏可以使用'|'混合使用
* return (int):
*     0: 如果使用WNOHANG,且进程没有结束
*     pid: 进程结束返回子进程的pid
*     -1: 调用失败返回, 没有子进程返回
*/

int waitpid(pid_t pid, int *statusp, int options);


/*
* 等价于 waitpid(-1, statusp, 0);
*/

pid_t wait(int *statusp);

```

## signal 相关

```c
#define _POSIX_SOURCE

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

/*
* 信号处理函数的重定义
* args：
*     int sig: 信号序号
* return (void):
*     void
*/
typedef void (*__sighandler_t) (int); 
typedef __sighandler_t sighandler_t;
/* 
* 进程组: 信号的发送与进程组相关,当发送信号时可以向单个进程发送, 也可以向进程组发送信号
* 进程组相关函数:
*     1. getpgrp
*     2. setpgid
*/

/*
获取当前进程的进程组id
args: 
	void
return:
	pid_t 当前进程的进程组id
*/
pid_t getpgrp(void); 

/*
设置当前进程的进程组id
args:
	pid_t pid: 原进程号
	pid_t pgid: 修改后的进程号
	如果pid是0,那么就使用当前进程的pid, 如果pgid是0,那么就使用pid指定的进程id作为进程组id
return (int) :
	0: 操作成功
	-1: 错误
*/
int setpgid(pid_t pid, pid_t pgid);

/*
 发送一个信号
 args: 
     pid_t pid: 对象进程或进程组, 进程组为负数(-1)是对进程组1的所有进程发送信号
     int sig: 具体信号序号, signal.h中存在定义
return (int):
	0 :成功
	-1:失败
*/
int kill(pid_t pid, int sig);

/*
信号的注册
args:
	int sig: 信号的序号
	sighandler_t handler: 新的信号处理函数
return (sighandler_t):
	sighandler_t: 成功,原有的信号处理函数
	SIG_ERR: 失败
*/
sighandler_t signal(int sig, sighandler_t handler);

/* 信号的阻塞和解除阻塞信号  */
/*
信号的阻塞是将信号放入待处理队列,而不是简单抛弃这个信号

 1. 隐式阻塞机制: 在使用默认处理程序时,内核默认阻塞任何当前处理程序正在处理信号类型的待处理的信号
 2. 显示阻塞机制:程序可以采用sigprocmask函数及其辅助函数显示阻塞和解除阻塞信号
 */

/* 
sigset_t: A set of signals to be blocked, unblocked, or waited for. 类似于bitmap

#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
{
  unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;
*/

/*
清空sigset/初始化空的sigset
args:
	sigset_t *sigset: 待初始化的sigset
return (int):
	0: 成功
	-1: 失败,并设置errno
*/
int sigemptyset(sigset_t *sigset);

/*
填充sigset为全1/初始化空的sigset
args: 
	sigset_t *sigset: 待初始化的sigset
return (int):
	0: 成功
	-1: 失败,并设置errno
*/
int sigfillset(sigset_t *sigset);

/* 
添加信号进入sigset
args:
	sigset_t sigset* :待添加的sigset
	int sig: 信号序号
return (int):
	0: 成功
	-1: 失败,并设置errno
*/
int sigaddset(sigset_t *sigset, int sig);

/* 
删除信号进入sigset
args:
	sigset_t sigset* :待删除的sigset
	int sig: 信号序号
return (int):
	0: 成功
	-1: 失败,并设置errno
*/
int sigdekset(sigset_t *sigset, int sig);

/*
改变当前阻塞的信号集合
args:
	int how: 设置阻塞方式:
		SIG_BLOCK: 把set中的信号添加到blocked中 block |= sigset
		SIG_UNBLOCK: 根据sigset, 将block中的信号删除 block &= ~sigset
		SIG_SETMASK: 将block用sigset覆盖 block = sigset
	const sigset_t *sigset: 设置的sigset
	sigset_t *oldsigset: 原有的sigset
*/

int sigprocmask(int how, const sigset_t *sigset, sigset_t *oldsigset);

```

### 如何编写信号处理程序?

1. 处理程序尽量简单
2. 处理程序只调用异步信号安全的函数
3. 保存和恢复errno

```c
void sighandler(int sig)
{
	int olderrno = errno;
	// ...
	errno = olderrno;
	return;
}
```

4. 阻塞所有信号,保护对全局共享函数的访问
5. 用volatile声明全局变量
6. 用`sig_atomic_t`声明标志变量

### 如何正确编写信号处理函数

1. 信号是不排队的,当一个同样的信号到达,只会处理一个信号,其余信号会被丢弃

因此 **不可以用信号对程序中发生的事件进行计数**

ps aux指令的进程运行状态:
```
D    uninterruptible sleep (usually IO)
I    Idle kernel thread
R    running or runnable (on run queue) 仅在前台进程会存在R状态
S    interruptible sleep (waiting for an event to complete) 后台进程为S状态
T    stopped by job control signal
t    stopped by debugger during the tracing
W    paging (not valid since the 2.6.xx kernel)
X    dead (should never be seen)
Z    defunct ("zombie") process, terminated but not reaped by its parent
```