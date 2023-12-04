# 缓存的组织结构

![](images/6-Cache%20Memories-缓存的组织结构.png)

缓存总共有 S 组, 每组有 E 调线, 每条线中存有 B个数据块

有效位 v 指示缓存中的数据是否有效,  标志位 tag 帮助进行寻址

# cache 如何进行读取

1. CPU向cache发送地址, 询问并要求该地址返回字, cache 得到的地址被分为三个部分: 

![](images/6-Cache%20Memories-3parts.png)
2. cache首先利用 set index 找到第 s 组, 然后检查 tag 寻找匹配的block, 最后用 b 的偏移找到数据

## 直接映射缓存 (E = 1)

![](images/6-Cache%20Memories-cache寻址.png)

![](images/6-Cache%20Memories-直接映射缓存的模拟.png)

先查 s , 再比较tag, 如果正确, 则开始根据 b 读入数据, 否则从内存中读入数据, 覆盖掉原有的 tag 位和 block 数据, 并将有效位置为 1.

## E路高速相连缓存(E >= 2 )

![](images/6-Cache%20Memories-E路相连缓存.png)

![](images/6-Cache%20Memories-2wayexample.png)

# cache 如何进行写入

两种写入方式: 1. 遇到修改即写入 2. 在被其他数据替换时进行判断是否被修改后写入

![](images/6-Cache%20Memories-intel的缓存设置.png)

# cache 性能

![](images/6-Cache%20Memories-性能.png)

当没有 hit 时, 会存在 miss 惩罚, 这个惩罚相当的大, 因此缓存对非常小的 hit 的变化率也会有很大的敏感度

# 书写缓存友好的代码

访问局部变量可以使变量放入寄存器中, 加速引用

**读吞吐量(读入带宽):** 每秒钟可以从内存中读入的字节数 

**存储山:** 将读吞吐量看作一个空间和时间局部性的函数

![](images/6-Cache%20Memories-存储山.png)

增加步长和增加size会导致空间的局部性变弱, 所以性能也会变差.

# 矩阵乘法的几种优化

![](images/6-Cache%20Memories-ijk.png)

假设一个 block 可以存储 4 组数据

由于 i 是对行访问,  那么每次只有第一个会miss, 然后读取内存, 其余都会 hit(空间的局部性), 因此 miss 率为 0.25 

由于 k 是俺列访问, 那么由于空间不连续, 所以每次都会 hit , 所以 miss 率为 1

c不在内层循环里, 不考虑

![](images/6-Cache%20Memories-jik.png)

jik模式也一样

![](images/6-Cache%20Memories-kji.png)

kij 模式则可以减少为命中率, ikj也一样

![](images/6-Cache%20Memories-几种性能比较.png)


# 使用分块利用空间局部性

 