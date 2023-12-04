# 0. 准备

1. 下载实验包：

* 资源链接 `http://csapp.cs.cmu.edu/3e/bomb.tar`

```bash
$ wget http://csapp.cs.cmu.edu/3e/bomb.tar
$ tar xvf bomb.tar
$ cd bomb
```

2. 熟悉常用的GDB命令:

	打印 `print/p [/fmt] expr`:
		fmt有如下形式
		/x:十六进制
		/d:十进制
		/u:无符号十进制
		/o:八进制
		/t:二进制
		/f:浮点数
		/c:字符数组
	检验(查看内存) `examine/x [/nfu] <addr>`
		n表示要显示的内存单元的个数
		f表示显示方式, 可取如下值:
			x 按十六进制格式显示变量。
			d 按十进制格式显示变量。
			u 按十进制格式显示无符号整型。
			o 按八进制格式显示变量。
			t 按二进制格式显示变量。
			a 按十六进制格式显示变量。
			i 指令地址格式
			c 按字符格式显示变量。
			f 按浮点数格式显示变量。
		u表示一个地址单元的长度
			b表示单字节
			h表示双字节
			w表示四字节
			g表示八字节
		举例:
			x/3uh buf
			表示从内存地址buf读取内容，
			h表示以双字节为一个单位，
			3表示三个单位，
			u表示按十六进制显示
	GUI调试页面 layout(需要有对应的源文件):
		layout src 显示源码
		layout asm 显示汇编代码
		layout split 同时显示源码
		layout prev 显示前一个窗口
		layout next 显示下一个窗口
		layout regs 显示寄存器
	开启单步汇编调试:
		set disassemble-next-line on
		si => 汇编单步进入 step in
		ni => 汇编单步跳过 next line

3. objdump的使用

```bash
$ objdump -d exec > file.s # 将可执行文件反汇编,并保存
```

4. at&t汇编函数调用:

* 参数小于6个时,按照下表的方式进行传参

|       | 1st | 2nd | 3rd | 4th | 5th | 6th |
| ----- | --- | --- | --- | --- | --- | --- |
| 64bit | rdi | rsi | rdx | rcx | r8  | r9  |
| 32bit | edi | esi | edx | ecx | r8d | r9d |
| 16bit | di  | si  | dx  | cx  | r8w | r9w |
| 8bit  | dil | sil | dl  | cl  | r8b | r9b    |

* 当参数多余6个时,多余的参数通过栈传参
* 函数调用的返回值存储在`rax`中
# 1. 了解bomblab任务

bomblab是针对`x86-64`机器代码和操作系统内存的一系列学习实验,这个实验主要是通过针对不同的函数进行分析,以完成对应的数据抽取任务.整个实验分为七个部分(6个常规部分加一个隐藏部分),我个人只找到了前六个炸弹.

## 2.  phase_1: 在内存中寻址字符串

```asm
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp # 在栈中分配8个字节的空间, 用于保存临时变量等
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi # rdi中存储的是从文件中读入的字符串, rsi中存储的是目标字符串的地址,需要从这个地址中读出字符串
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal> # 调用函数
  400eee:	85 c0                	test   %eax,%eax # 返回值保存在%eax中, test是检验是否为0, test = %eax & %eax,但是不进行赋值,只改变符号位
  400ef0:	74 05                	je     400ef7 <phase_1+0x17> # 如果为零,也即两个字符串相等就跳转至0x400ef7
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq   

0000000000401338 <strings_not_equal>:
  401338:	41 54                	push   %r12 # 保护数据
  40133a:	55                   	push   %rbp
  40133b:	53                   	push   %rbx
  40133c:	48 89 fb             	mov    %rdi,%rbx # 将第一个参数放入%rbx寄存器中
  40133f:	48 89 f5             	mov    %rsi,%rbp # 将第二个参数放入%rbp寄存器中 
  401342:	e8 d4 ff ff ff       	callq  40131b <string_length>
  401347:	41 89 c4             	mov    %eax,%r12d # 计算第一个字符串的长度
  40134a:	48 89 ef             	mov    %rbp,%rdi  # 将第二个字符串作为参数,进行调用
  40134d:	e8 c9 ff ff ff       	callq  40131b <string_length>
  401352:	ba 01 00 00 00       	mov    $0x1,%edx 
  401357:	41 39 c4             	cmp    %eax,%r12d ; 返回第二个字符串串的长度
  # 进行循环, 比较字符是否相等, edx是循环控制变量, 类似for中的i
  40135a:	75 3f                	jne    40139b <strings_not_equal+0x63>
  40135c:	0f b6 03             	movzbl (%rbx),%eax 
  40135f:	84 c0                	test   %al,%al
  401361:	74 25                	je     401388 <strings_not_equal+0x50>
  401363:	3a 45 00             	cmp    0x0(%rbp),%al
  401366:	74 0a                	je     401372 <strings_not_equal+0x3a>
  401368:	eb 25                	jmp    40138f <strings_not_equal+0x57>
  40136a:	3a 45 00             	cmp    0x0(%rbp),%al
  40136d:	0f 1f 00             	nopl   (%rax)
  401370:	75 24                	jne    401396 <strings_not_equal+0x5e>
  401372:	48 83 c3 01          	add    $0x1,%rbx
  401376:	48 83 c5 01          	add    $0x1,%rbp
  40137a:	0f b6 03             	movzbl (%rbx),%eax
  40137d:	84 c0                	test   %al,%al
  40137f:	75 e9                	jne    40136a <strings_not_equal+0x32>
  401381:	ba 00 00 00 00       	mov    $0x0,%edx
  401386:	eb 13                	jmp    40139b <strings_not_equal+0x63>
  401388:	ba 00 00 00 00       	mov    $0x0,%edx
  40138d:	eb 0c                	jmp    40139b <strings_not_equal+0x63>
  40138f:	ba 01 00 00 00       	mov    $0x1,%edx
  401394:	eb 05                	jmp    40139b <strings_not_equal+0x63>
  401396:	ba 01 00 00 00       	mov    $0x1,%edx
  40139b:	89 d0                	mov    %edx,%eax
  40139d:	5b                   	pop    %rbx
  40139e:	5d                   	pop    %rbp
  40139f:	41 5c                	pop    %r12
  4013a1:	c3                   	retq   
```

通过上述代码的分析,可以很清晰的看到源程序需要进行比较的字符串存储位置在`0x402400`处,而字符串的具体长度未知,可以在`strings_not_equal`中通过`string_length`的调用结果得知.最后通过`p /s *0x402400`或者`x /60cb 0x402400`查看字符串.

答案为`Border relations with Canada have never been better.`

# phase_2:了解多参数的函数调用

```asm
0000000000400efc <phase_2>: 参数 %rdi 是读入的字符串
  400efc:	55                   	push   %rbp 
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp 
  400f02:	48 89 e6             	mov    %rsp,%rsi 
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp) # 第一个数需要是1, 否则会引爆炸弹 
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax # 取上一个参数
  400f1a:	01 c0                	add    %eax,%eax # 使这个参数乘2
  400f1c:	39 03                	cmp    %eax,(%rbx) 判断当前参数与上一个参数的二倍是否相同
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx # 地址加4,指向下一个参数
  400f29:	48 39 eb             	cmp    %rbp,%rbx # 一个等比数列,公比为2
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq   

000000000040145c <read_six_numbers>:
  40145c:	48 83 ec 18          	sub    $0x18,%rsp 
  401460:	48 89 f2             	mov    %rsi,%rdx # 参数3
  401463:	48 8d 4e 04          	lea    0x4(%rsi),%rcx # 参数4
  401467:	48 8d 46 14          	lea    0x14(%rsi),%rax
  40146b:	48 89 44 24 08       	mov    %rax,0x8(%rsp) # 使用栈进行传参, 参数7
  401470:	48 8d 46 10          	lea    0x10(%rsi),%rax 
  401474:	48 89 04 24          	mov    %rax,(%rsp) # 使用栈传参,参数8
  401478:	4c 8d 4e 0c          	lea    0xc(%rsi),%r9 # 参数6
  40147c:	4c 8d 46 08          	lea    0x8(%rsi),%r8 # 参数5
  401480:	be c3 25 40 00       	mov    $0x4025c3,%esi # 参数2 -> 格式字符串
  401485:	b8 00 00 00 00       	mov    $0x0,%eax
  40148a:	e8 61 f7 ff ff       	callq  400bf0 <__isoc99_sscanf@plt> # sscanf(char *src, char *fmt, ...);
  40148f:	83 f8 05             	cmp    $0x5,%eax 
  401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>
  401494:	e8 a1 ff ff ff       	callq  40143a <explode_bomb>
  401499:	48 83 c4 18          	add    $0x18,%rsp
  40149d:	c3                   	retq   
```

phase_2中调用了`read_six_numbers`函数,这说明要读入的是6个数字.但是还无法确认六个数字的类型.因此需要从`read_six_numbers`函数入手.

通过阅读`read_six_numbers`函数的内部,可以看到只显式地传入了七个参数,第一个参数没有出现,说明是由外部传入的,也即`read_six_numbers`至少有一个输入字符串,而这个字符串就是我们的输入字符串. 另外格式字符串的存储位置在第二个参数(`0x4025c3`)处可以读到.根据这里,已经可以知道这个函数的作用了.

phase_2函数则是对输入的字符串处理后得到了一系列的整数,并且这些整数还有一定的要求.通过分析,可以看到这个循环要求传入的数字以1为开始,且后一个数字需要是前一个数字的2倍,因此也不难看出这是一个等比数列,公比为2,首项为1.

答案即为`1 2 4 8 16 32`

# phase_3:分支匹配

```asm
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx # 取 (%rsp)+0xc 的地址
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx # 取 (%rsp)+0x8 的地址
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi # 取格式字符串
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt> # 调用ssacnf, 说明需要读入两个数字
  400f60:	83 f8 01             	cmp    $0x1,%eax 
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp) ;第一个数据必须小于7
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8) # 根据第一个数进行选择,第二个数必须符合第一个数的选择, *是地址访问运算符,这里是访问从0x402470+8%rax处的数据地址,对应了后面的这些分支,不同的数字对应的是不同的分支
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax # 对比分支与输入的第二个数字是否相同
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq   
```

phase_3整体上比较简单,只是一个分支匹配问题

# phase_4: 递归

```asm
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx # 用来保存第二个读入的数字
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx # 用来保存第一个读入的数字
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax # 读入两个数
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp) # 第一个数必须小于等于14
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx # edx是14,这里要记好
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	callq  400fce <func4>
  40104d:	85 c0                	test   %eax,%eax # 与运算,返回值必须是0
  40104f:	75 07                	jne    401058 <phase_4+0x4c>
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp) # 只要让第二个数字是0就可以了
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq   


0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax
  400fd4:	29 f0                	sub    %esi,%eax
  400fd6:	89 c1                	mov    %eax,%ecx
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx # 逻辑右移,取符号位
  400fdb:	01 c8                	add    %ecx,%eax # eax + 符号位
  400fdd:	d1 f8                	sar    %eax # 算术右移1位 => /2, 简单结束递归,只需要让第一个参数等于 14 / 2 = 7即可
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx
  400fe2:	39 f9                	cmp    %edi,%ecx
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24> # 直到edi小于等于ecx为止
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx 
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>
  400fee:	01 c0                	add    %eax,%eax # 递归终止
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
  400ff7:	39 f9                	cmp    %edi,%ecx 
  400ff9:	7d 0c                	jge    401007 <func4+0x39> # edi大于等于ecx, 递归终止
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax # eax = rax + 1 * rax + 1
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3                   	retq   # 返回值必须是0
```

# phase_5:字符串加密

```asm
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp
  401067:	48 89 fb             	mov    %rdi,%rbx
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax # 设置金丝雀,防止内存溢出
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
  401078:	31 c0                	xor    %eax,%eax
  40107a:	e8 9c 02 00 00       	callq  40131b <string_length>
  40107f:	83 f8 06             	cmp    $0x6,%eax
  401082:	74 4e                	je     4010d2 <phase_5+0x70>
  401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx # 对数据进行了加密
  40108f:	88 0c 24             	mov    %cl,(%rsp) # 将字符送入栈中
  401092:	48 8b 14 24          	mov    (%rsp),%rdx # 将字符保存在rdx中
  401096:	83 e2 0f             	and    $0xf,%edx # 只保留字符的后八位
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx # 根据0x4024b0的内存信息找到对应的映射表
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1) 
  4010a4:	48 83 c0 01          	add    $0x1,%rax
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi # 目标字符串地址,根据此地址寻找目标字符串
  4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
  4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	callq  400b30 <__stack_chk_fail@plt>
  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3                   	retq   

```

# phase_6:链表排序

```asm
00000000004010f4 <phase_6>:
  4010f4:	41 56                	push   %r14
  4010f6:	41 55                	push   %r13
  4010f8:	41 54                	push   %r12
  4010fa:	55                   	push   %rbp
  4010fb:	53                   	push   %rbx
  4010fc:	48 83 ec 50          	sub    $0x50,%rsp
  401100:	49 89 e5             	mov    %rsp,%r13
  401103:	48 89 e6             	mov    %rsp,%rsi
  401106:	e8 51 03 00 00       	callq  40145c <read_six_numbers> # 读入六个数字
  40110b:	49 89 e6             	mov    %rsp,%r14
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  401114:	4c 89 ed             	mov    %r13,%rbp
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax # 最大值不超过6, 最小值还要大于0
  40111b:	83 e8 01             	sub    $0x1,%eax # 要求每个数字减1后要小于5
  40111e:	83 f8 05             	cmp    $0x5,%eax
  401121:	76 05                	jbe    401128 <phase_6+0x34>
  401123:	e8 12 03 00 00       	callq  40143a <explode_bomb>
  401128:	41 83 c4 01          	add    $0x1,%r12d
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d
  401130:	74 21                	je     401153 <phase_6+0x5f> # 重复6次,也即要求每个数字都这样
  401132:	44 89 e3             	mov    %r12d,%ebx 
  401135:	48 63 c3             	movslq %ebx,%rax
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax
  40113b:	39 45 00             	cmp    %eax,0x0(%rbp) # 判断当前数字与其余数字是否相同
  40113e:	75 05                	jne    401145 <phase_6+0x51> # 每个数字都要进行判断, 这里结束后说明数字是不同的
  401140:	e8 f5 02 00 00       	callq  40143a <explode_bomb>
  401145:	83 c3 01             	add    $0x1,%ebx
  401148:	83 fb 05             	cmp    $0x5,%ebx 
  40114b:	7e e8                	jle    401135 <phase_6+0x41> 
  40114d:	49 83 c5 04          	add    $0x4,%r13 # 取下一个数字
  401151:	eb c1                	jmp    401114 <phase_6+0x20>
  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi
  401158:	4c 89 f0             	mov    %r14,%rax
  40115b:	b9 07 00 00 00       	mov    $0x7,%ecx
  401160:	89 ca                	mov    %ecx,%edx
  401162:	2b 10                	sub    (%rax),%edx
  401164:	89 10                	mov    %edx,(%rax)
  401166:	48 83 c0 04          	add    $0x4,%rax
  40116a:	48 39 f0             	cmp    %rsi,%rax
  40116d:	75 f1                	jne    401160 <phase_6+0x6c> # 用7减去每一个数,因此原来的数与新数关于7互补
  40116f:	be 00 00 00 00       	mov    $0x0,%esi
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
  401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx
  40117a:	83 c0 01             	add    $0x1,%eax
  40117d:	39 c8                	cmp    %ecx,%eax
  40117f:	75 f5                	jne    401176 <phase_6+0x82>
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
  401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx # 这个地址是一个链表, 这个链表的长度为16字节
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2) # 上述循环是将链表的地址拷贝到对应位置
  40118d:	48 83 c6 04          	add    $0x4,%rsi
  401191:	48 83 fe 18          	cmp    $0x18,%rsi
  401195:	74 14                	je     4011ab <phase_6+0xb7>
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx # 获取下一个查找次数,查找次数=新数字, 拷贝的顺序是根据我们传入的数字相关
  40119a:	83 f9 01             	cmp    $0x1,%ecx
  40119d:	7e e4                	jle    401183 <phase_6+0x8f>
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax # 清除索引,重新寻找对应链表
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx # 指向链表头
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
  4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx
  4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax
  4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi
  4011ba:	48 89 d9             	mov    %rbx,%rcx
  4011bd:	48 8b 10             	mov    (%rax),%rdx
  4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx)
  4011c4:	48 83 c0 08          	add    $0x8,%rax
  4011c8:	48 39 f0             	cmp    %rsi,%rax
  4011cb:	74 05                	je     4011d2 <phase_6+0xde>
  4011cd:	48 89 d1             	mov    %rdx,%rcx
  4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9> 
  4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx) # 反转链表
  4011d9:	00 
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax
  4011e3:	8b 00                	mov    (%rax),%eax
  4011e5:	39 03                	cmp    %eax,(%rbx) # 判断链表是否是递减的
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>
  4011e9:	e8 4c 02 00 00       	callq  40143a <explode_bomb>
  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx # p = p->next
  4011f2:	83 ed 01             	sub    $0x1,%ebp
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb>
  4011f7:	48 83 c4 50          	add    $0x50,%rsp
  4011fb:	5b                   	pop    %rbx
  4011fc:	5d                   	pop    %rbp
  4011fd:	41 5c                	pop    %r12
  4011ff:	41 5d                	pop    %r13
  401201:	41 5e                	pop    %r14
  401203:	c3                   	retq   
```

```
0x6032d0 <node1>:       332     1       6304480 0
0x6032e0 <node2>:       168     2       6304496 0
0x6032f0 <node3>:       924     3       6304512 0
0x603300 <node4>:       691     4       6304528 0
0x603310 <node5>:       477     5       6304544 0
0x603320 <node6>:       443     6       0       0
```