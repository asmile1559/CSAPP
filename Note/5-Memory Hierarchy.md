# 内存层次结构

# RAM

RAM分为 SRAM 和 DRAM

![](images/5-Memory%20Hierarchy-S&DRAM.png)

其他的存储结构 
1. ROM: 只读存储器
2. PROM: 可编程(一次)只读存储器
3. EPROM: 可擦写只读存储器(紫外光, X光)
4. EEPROM: 电擦写只读存储器
5. EEPROMs(flash): 可多次擦鞋存储器

# BUS

用来传输数据, 介于I/O, 内存与CPU之间

# 内存的读取过程

1. CPU将地址放到总线上, 利用 IO桥访问内存
2. 内存通过总线经过IO桥传入CPU的总线接口
3. CPU读取总线数据, 放入到寄存器中

内存的写入与读取过程相反, 写地址, 写接口, 写内存

# IO总线

![](images/5-Memory%20Hierarchy-IO总线.png)

# CPU 如何读入 DISK Sector

1. CPU发送读指令

![](images/5-Memory%20Hierarchy-readdisk1.png)

2. . 磁盘控制器读取逻辑块对应的扇区, DMA方式读取数据

![](images/5-Memory%20Hierarchy-readdisk2.png)

3. 磁盘控制器向CPU发送中断, 通知 CPU 数据读取完成

![](images/5-Memory%20Hierarchy-readdisk3.png)

# SSD

![](images/5-Memory%20Hierarchy-ssd.png)

ssd的读写是以页为单位的, 必须将整页清除才可以写入

# locality

**程序的局部性**: 程序的局部性一般可以分为两种, 时间的局部性, 最近引用的存储器位置可能在不久的将来再次被引用; 空间的局部性, 引用临近存储器位置的倾向

![](images/5-Memory%20Hierarchy-局部性.png)

**局部性的例子**

好的局部性
```c
for (int i = 0; i < M; i++)
{
	for(int j = 0; j < N; j++)
	{
		sum += a[i][j];
	}
}
```
因为 `j` 是顺序变换的, 满足空间的局部性, 因此效率很高

不好的局部性
```c
for (int j = 0; j < N; j++)
{
	for(int i = 0; i < M; i++)
	{
		sum += a[i][j];
	}
}
```
先访问行, 导致空间跳跃, 效率降低

# 内存的层次结构

![](images/5-Memory%20Hierarchy-内存的层次设计.png)

# cache

![](images/5-Memory%20Hierarchy-cache.png)

**缓存击中(hit)** 缓存中存在cpu需要的内容, 那么cpu就会直接获取该数据
**缓存不命中(miss)** :缓存中不存在CPU需要的内容, 那么就会从Mempry 读到缓存, 再交给CPU读取

## 几种缓存不命中的情况

![](images/5-Memory%20Hierarchy-cachemiss.png)

1. 冷不命中, 缓存中没有内容, 自然不能命中
2. 容量不命中, 缓存的容量低于CPU需要的数据容量
3. 冲突不命中, 类似于哈希冲突