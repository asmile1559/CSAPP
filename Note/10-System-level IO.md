
![](images/10-System-level%20IO-base.png)

# Unix (I/O)

## overview

![](images/10-System-level%20IO-unixioow1.png)

`unix` 将文件看作是字节序列, 其不关系文件的具体实现形式, 因此在这个操作系统中, 一切都可以用文件表示

而对这些文件的操作, 全部取决于文件的 "打开", "读写", "关闭" 的操作

![](images/10-System-level%20IO-文件的操作.png)

* `lseek` locate seek, 返回当前的位置

## 文件的类别

![](images/10-System-level%20IO-文件类别.png)

![](images/10-System-level%20IO-常规文件.png)

![](images/10-System-level%20IO-目录.png)


![](images/10-System-level%20IO-打开文件.png)

![](images/10-System-level%20IO-open参数.png)

`fd` 文件描述符, 是一个很小的整数

![](images/10-System-level%20IO-close文件.png)

传入的是文件描述符而不是文件名

![](images/10-System-level%20IO-read文件.png)

* read 最少读 1 个字符
* read 会阻塞进程/线程

![](images/10-System-level%20IO-write文件.png)

![](images/10-System-level%20IO-shortcount.png)

如果读到了文件末尾 **EOF** , 则直接停止读入, 并将已读入的存储在缓冲区中, 如果从 **EOF** 开始读, 则返回 0

测试代码
```c
/*
* 数据: hello this is a test file and i just type in some
*/

int main()
{
    char buff[16] = {0};
    int fd = open("toread", O_RDONLY);
    int nbytes = 0;
    do
    {
        nbytes = read(fd, buff, 16);
        printf("%d, %s\n", nbytes, buff);
        memset(buff, 0, 16);
    } while (nbytes > 0);
}
/* 
* 输出:
* 16, hello this is a 
* 16, test file and i
* 16, just type in som
* 1, e
* 0, 
*/
```

# File Metadata

![](images/10-System-level%20IO-metadata.png)

用于表示一个文件的各种状态

# Unix 如何表示打开的文件的

![](images/10-System-level%20IO-如何表示.png)

对于每个正在运行的进程, 都有一个与之对应的描述符表(Descriptor table), 对于每个打开的文件, 描述符表都有一个表项(Open file table), 指向该文件的数据结构, 每个文件表表项都是对某个打开文件的引用, 它给出了文件的信息, 文件的位置, 引用次数(当引用次数为0时, 回收内存)等, 文件表项还指向了虚拟节点表(v-node table), 这个表存储了stat中的文件信息

![](images/10-System-level%20IO-打开相同的文件.png)

如果打开相同的文件, 会创造两个表项指向相同的虚拟结点

## 如果使用fork

![](images/10-System-level%20IO-fork1.png)

![](images/10-System-level%20IO-fork2.png)


# 重定向

![](images/10-System-level%20IO-重定向.png)

改变文件描述符指向的对象, 这里是将标准输出指向了了文件, 因此可以向文件中写入数据

![](images/10-System-level%20IO-重定向例子.png)

![](images/10-System-level%20IO-重定向例子1.png)

# 标准 IO函数

![](images/10-System-level%20IO-标准io.png)

标准io带有缓存, 所以效率较高, 但是标准io有很多的问题

各种 IO 的优缺点

![](images/10-System-level%20IO-unixio.png)

![](images/10-System-level%20IO-标准io1.png)

# 如何选择 io

![](images/10-System-level%20IO-如何选择io.png)

# 二进制文件的读取! 避免使用以下函数

![](images/10-System-level%20IO-二进制文件操作.png)

