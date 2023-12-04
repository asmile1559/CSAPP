# 1 Introduction
This assignment involves generating a total of five attacks on two programs having different security vulnerabilities. Outcomes you will gain from this lab include:

这个作业中在两个程序中包含类五个不同的安全缺陷。经过这个实验， 你将会获得如下内容：

* You will learn different ways that attackers can exploit security vulnerabilities when programs do not safeguard themselves well enough against buffer overflows.
* 你会学习当程序没有进行足够的保护措施进行缓冲区溢出的保护时，攻击者利用安全漏洞进行攻击的不同方式
* Through this, you will get a better understanding of how to write programs that are more secure, as well as some of the features provided by compilers and operating systems to make programs less vulnerable.
* 通过这个，你将会进一步理解如何写出更加安全的程序和编译器与操作系统提供的使程序减少缺陷的方式
* You will gain a deeper understanding of the stack and parameter-passing mechanisms of x86-64 machine code.
* 你会获得在x86-64机器代码基础上栈和参数的传递方式
* You will gain a deeper understanding of how x86-64 instructions are encoded.
* 你会更深理解x86-64指令是如何解码的
* You will gain more experience with debugging tools such as GDB and OBJDUMP.
* 你会有更多机会实验`gdb`和`objdump`

**Note**:In this lab, you will gain firsthand experience with methods used to exploit security weaknesses in operating systems and network servers. Our purpose is to help you learn about the runtime operation of programs and to understand the nature of these security weaknesses so that you can avoid them when you write system code. We do not condone the use of any other form of attack to gain unauthorized access to any system resources. You will want to study Sections 3.10.3 and 3.10.4 of the CS:APP3e book as reference material for this lab.

**注意**:在这个实验中,你得到第一手使用操作系统和网络服务器的漏洞的经验.我们的目的是帮助你学习实时编程和理解安全漏洞的本质,以便于你可以在写系统代码是避免这些问题.


# 2 Logistics
As usual, this is an individual project. You will generate attacks for target programs that are custom generated for you.

## 2.1 Getting Files
You can obtain your files by pointing your Web browser at: http://$Attacklab::SERVER_NAME:15513/INSTRUCTOR: $Attacklab::SERVER_NAME is the machine that runs the attacklab servers. You define it in attacklab/Attacklab.pm and in attacklab/src/build/driverhdrs.h

The server will build your files and return them to your browser in a tar file called `targetk.tar`, where `k` is the unique number of your target programs.

**Note**: It takes a few seconds to build and download your target, so please be patient. Save the targetk.tar file in a (protected) Linux directory in which you plan to do your work. Then give the command: `tar -xvf targetk.tar`. This will extract a directory targetk containing the files described below. You should only download one set of files. If for some reason you download multiple targets, choose one target to work on and delete the rest.

**Warning**: If you expand your targetk.tar on a PC, by using a utility such as Winzip, or letting your browser do the extraction, you’ll risk resetting permission bits on the executable files.

The files in targetk include:

* **README.txt**: A file describing the contents of the directory
* **ctarget**: An executable program vulnerable to code-injection attacks
* **rtarget**: An executable program vulnerable to return-oriented-programming attacks
* **cookie**.txt: An 8-digit hex code that you will use as a unique identifier in your attacks.
* **farm.c**: The source code of your target’s “gadget farm,” which you will use in generating return-oriented programming attacks.
* **hex2raw**: A utility to generate attack strings.
<br>

* **README.txt**: A file describing the contents of the directory
* **ctarget**: 易受代码注入攻击的可执行程序
* **rtarget**: 易受面向返回的编程攻击的可执行程序
* **cookie.txt**: 一个8位数的十六进制代码，您将在攻击中使用它作为唯一标识符。
* **farm.c**: 目标的“小工具农场”的源代码，您将使用它来生成面向返回的编程攻击。
* **hex2raw**: 生成攻击字符串的实用程序。

In the following instructions, we will assume that you have copied the files to a protected local directory, and that you are executing the programs in that local directory.

## 2.2 Important Points
Here is a summary of some important rules regarding valid solutions for this lab. These points will not make much sense when you read this document for the first time. They are presented here as a central reference of rules once you get started.

* You must do the assignment on a machine that is similar to the one that generated your targets.
* Your solutions may not use attacks to circumvent the validation code in the programs. Specifically, any address you incorporate into an attack string for use by a ret instruction should be to one of the following destinations:
     * The addresses for functions touch1, touch2, or touch3.
     * The address of your injected code
     * The address of one of your gadgets from the gadget farm.
* You may only construct gadgets from file rtarget with addresses ranging between those for functions start_farm and end_farm. 
<br>
* 你必须在一台与生成目标的机器相似的机器上完成任务。
* 您的解决方案可能不会使用攻击来规避程序中的验证代码。具体来说，您包含在攻击字符串中供ret指令使用的任何地址都应该指向以下目的地之一：
    * 函数touch1、touch2或touch3的地址。
    * 注入代码的地址
    * 小工具场中某个小工具的地址。
* 您只能从文件rtarget构造小工具，其地址范围在函数start_farm和end_farm之间。


# 3 Target Programs

Both CTARGET and RTARGET read strings from standard input. They do so with the function getbuf defined below:

```c
unsigned getbuf()
{
    char buf[BUFFER_SIZE];
    Gets(buf);
    return 1;
}
```

The function Gets is similar to the standard library function gets—it reads a string from standard input (terminated by ‘\n’ or end-of-file) and stores it (along with a null terminator) at the specified destination. In this code, you can see that the destination is an array buf, declared as having BUFFER_SIZE bytes. At the time your targets were generated, BUFFER_SIZE was a compile-time constant specific to your version of the programs.

函数Gets类似于标准库函数Gets——它从标准输入中读取字符串（以“\n”或文件结尾终止），并将其与null终止符一起存储在指定的目标。在这段代码中，您可以看到目标是一个数组buf，声明为具有BUFFER_SIZE字节。在生成目标时，BUFFER_SIZE是特定于程序版本的编译时间常数。

Functions Gets() and gets() have no way to determine whether their destination buffers are large enough to store the string they read. They simply copy sequences of bytes, possibly overrunning the bounds of the storage allocated at the destinations.

函数 Gets()和 gets() 无法确定它们的目标缓冲区是否足够大以存储它们读取的字符串。它们只是复制字节序列，可能会超出在目的地分配的存储范围。

If the string typed by the user and read by getbuf is sufficiently short, it is clear that getbuf will return 1, as shown by the following execution examples:

如果用户键入并由getbuf读取的字符串足够短，那么很明显getbuf将返回1，如以下执行示例所示：

```bash
unix> ./ctarget
# Cookie: 0x1a7dd803
# Type string: Keep it short!
# No exploit. Getbuf returned 0x1
# Normal return
```
Typically an error occurs if you type a long string:

```bash
unix> ./ctarget
# Cookie: 0x1a7dd803
# Type string: This is not a very interesting string, but it has the property ...
# Ouch!: You caused a segmentation fault!
# Better luck next time
```

(Note that the value of the cookie shown will differ from yours.) Program RTARGET will have the same behavior. As the error message indicates, overrunning the buffer typically causes the program state to be corrupted, leading to a memory access error. Your task is to be more clever with the strings you feed CTARGET and RTARGET so that they do more interesting things. These are called exploit strings.

（请注意，显示的cookie值将与您的不同。）程序RTARGET将具有相同的行为。正如错误消息所示，溢出缓冲区通常会导致程序状态损坏，从而导致内存访问错误。你的任务是更巧妙地处理你提供给CTARGET和RTARGET的字符串，让它们做更有趣的事情。这些被称为漏洞利用字符串。

Both `CTARGET` and `RTARGET` take several different command line arguments:
* -h: Print list of possible command line arguments
* -q: Don’t send results to the grading server
* -i FILE: Supply input from a file, rather than from standard input

“CTARGET”和“RTARGET”都采用了几个不同的命令行参数：
* -h：打印可能的命令行参数列表
* -q：不将结果发送到评分服务器
* -i FILE：提供来自文件的输入，而不是来自标准输入

Your exploit strings will typically contain byte values that do not correspond to the ASCII values for printing characters. The program HEX2RAW will enable you to generate these raw strings. See Appendix A for more information on how to use HEX2RAW.

利用漏洞字符串通常包含与用于打印字符的ASCII值不对应的字节值。HEX2RAW程序将使您能够生成这些原始字符串。有关如何使用HEX2RAW的更多信息，请参阅附录A。

Important points:

* Your exploit string must not contain byte value 0x0a at any intermediate position, since this is the ASCII code for newline (‘\n’). When Gets encounters this byte, it will assume you intended to terminate the string.
* HEX2RAW expects two-digit hex values separated by one or more white spaces. So if you want to create a byte with a hex value of 0, you need to write it as 00. To create the word `0xdeadbeef` you should pass “ef be ad de” to HEX2RAW (note the reversal required for little-endian byte ordering).
<br>
* 您的攻击字符串在任何中间位置都不能包含字节值0x0a，因为这是换行符（'\n'）的ASCII代码。当Gets遇到此字节时，它将假定您打算终止该字符串。
* HEX2RAW需要用一个或多个空格分隔的两位十六进制值。因此，如果你想创建一个十六进制值为0的字节，你需要把它写成00。要创建单词“0xdeadbeef”，您应该将“ef be ad de”传递给HEX2RAW（注意小端字节排序所需的反转）。使用hex2raw如果不使用文件而是直接输入,那么需要eof(ctrl+d)

When you have correctly solved one of the levels, your target program will automatically send a notification to the grading server. For example:

```bash
unix> ./hex2raw < ctarget.l2.txt | ./ctarget
# Cookie: 0x1a7dd803
# Type string:Touch2!: You called touch2(0x1a7dd803)
# Valid solution for level 2 with target ctarget
# PASSED: Sent exploit string to server to be validated.
# NICE JOB!
```

|Phase|Program|Level|Method|Function|Points|
|---|---|---|---|---|--|
|1|CTARGET|1| CI |touch1 |10|
|2|CTARGET|2| CI |touch2 |25|
|3|CTARGET|3| CI |touch3 |25|
|4|RTARGET|2| ROP| touch2|35|
|5|RTARGET|3| ROP| touch3| 5|

* CI: Code injection
* ROP: Return-oriented programming
* Figure 1: Summary of attack lab phases

The server will test your exploit string to make sure it really works, and it will update the Attacklab scoreboard page indicating that your userid (listed by your target number for anonymity) has completed this phase.

You can view the scoreboard by pointing your Web browser at http://$Attacklab::SERVER_NAME:15513/scoreboard

Unlike the Bomb Lab, there is no penalty for making mistakes in this lab. Feel free to fire away at CTARGET and RTARGET with any strings you like.

服务器将测试您的攻击字符串，以确保它真的有效，并更新Attacklab记分板页面，指示您的用户ID（为匿名起见，按目标编号列出）已完成此阶段。

您可以通过将Web浏览器指向http://$Attacklab:：SERVER_NAME:15513/scoreboard来查看记分牌

与炸弹实验室不同，在这个实验室里犯错误不会受到惩罚。你可以随意用任何你喜欢的字符串攻击CTARGET和RTARGET。

**IMPORTANT NOTE**: You can work on your solution on any Linux machine, but in order to submit your solution, you will need to be running on one of the following machines:

`INSTRUCTOR: Insert the list of the legal domain names that you established in buflab/src/config.c. `

Figure 1 summarizes the five phases of the lab. As can be seen, the first three involve code-injection (CI) attacks on CTARGET, while the last two involve return-oriented-programming (ROP) attacks on RTARGET.

# 4 Part I: Code Injection Attacks
For the first three phases, your exploit strings will attack CTARGET. This program is set up in a way that the stack positions will be consistent from one run to the next and so that data on the stack can be treated as executable code. These features make the program vulnerable to attacks where the exploit strings contain the byte encodings of executable code.

在前三个阶段，您的漏洞字符串将攻击CTARGET。该程序的设置方式是，从一次运行到下一次运行，堆栈位置将保持一致，因此堆栈上的数据可以被视为可执行代码。这些功能使程序容易受到攻击，因为漏洞字符串包含可执行代码的字节编码。

## 4.1 Level 1
For Phase 1, you will not inject new code. Instead, your exploit string will redirect the program to execute an existing procedure.

对于阶段1，您将不会注入新代码。相反，您的漏洞利用字符串将重定向程序以执行现有过程。

Function getbuf is called within CTARGET by a function test having the following C code:

```c
void test()
{
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

When getbuf executes its return statement (line 5 of getbuf), the program ordinarily resumes execution within function test (at line 5 of this function). We want to change this behavior. Within the file ctarget, there is code for a function touch1 having the following C representation:

当getbuf执行其返回语句（getbuf的第5行）时，程序通常会在函数测试中恢复执行（在该函数的第5行里）。我们想改变这种行为。在文件ctarget中，有一个函数touch1的代码，具有以下C表示：

```c
void touch1()
{
    vlevel = 1; /* Part of validation protocol */
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

Your task is to get CTARGET to execute the code for touch1 when getbuf executes its return statement, rather than returning to test. Note that your exploit string may also corrupt parts of the stack not directly related to this stage, but this will not cause a problem, since touch1 causes the program to exit directly.

您的任务是让CTARGET在getbuf执行其return语句时执行touch1的代码，而不是返回测试。请注意，您的漏洞利用字符串也可能损坏与此阶段没有直接关系的堆栈部分，但这不会造成问题，因为touch1会导致程序直接退出。

**Some Advice:**
• All the information you need to devise your exploit string for this level can be determined by examining a disassembled version of CTARGET. Use objdump -d to get this dissembled version.
• The idea is to position a byte representation of the starting address for touch1 so that the ret instruction at the end of the code for getbuf will transfer control to touch1.
• Be careful about byte ordering.
• You might want to use GDB to step the program through the last few instructions of getbuf to make sure it is doing the right thing.
• The placement of buf within the stack frame for getbuf depends on the value of compile-time constant BUFFER_SIZE, as well the allocation strategy used by GCC. You will need to examine the disassembled code to determine its position.

**一些建议：**
• 设计该级别的漏洞利用字符串所需的所有信息都可以通过检查CTARGET的反汇编版本来确定。使用objdump-d可以获得这个经过分解的版本。
• 其想法是定位touch1起始地址的字节表示，以便getbuf代码末尾的ret指令将控制权转移到touch1。
• 注意字节排序。
• 您可能希望使用GDB逐步完成getbuf的最后几条指令，以确保它做的是正确的事情。
• buf在getbuf堆栈帧中的位置取决于编译时常数BUFFER_SIZE的值，以及GCC使用的分配策略。您需要检查已分解的代码以确定其位置。

## 4.2 Level 2
Phase 2 involves injecting a small amount of code as part of your exploit string. Within the file ctarget there is code for a function touch2 having the following C representation:

第2阶段涉及注入少量代码作为漏洞利用字符串的一部分。在文件ctarget中，存在用于具有以下C表示的函数touch2的代码：

```c
void touch2(unsigned val)
{
    vlevel = 2; /* Part of validation protocol */
    if (val == cookie) {
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    } else {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

Your task is to get CTARGET to execute the code for touch2 rather than returning to test. In this case, however, you must make it appear to touch2 as if you have passed your cookie as its argument.

您的任务是让CTARGET执行touch2的代码，而不是返回test。然而，在这种情况下，您必须让它看起来像是touch2，就好像您已经将cookie作为其参数传递了一样。

**Some Advice:**
• You will want to position a byte representation of the address of your injected code in such a way that ret instruction at the end of the code for getbuf will transfer control to it.
• Recall that the first argument to a function is passed in register %rdi.
• Your injected code should set the register to your cookie, and then use a ret instruction to transfer control to the first instruction in touch2.
• Do not attempt to use jmp or call instructions in your exploit code. The encodings of destination addresses for these instructions are difficult to formulate. Use ret instructions for all transfers of control, even when you are not returning from a call.
• See the discussion in Appendix B on how to use tools to generate the byte-level representations of instruction sequences.

**一些建议：**
•您将希望以这样一种方式定位注入代码地址的字节表示，即getbuf代码末尾的ret指令将把控制权转移给它。
•回想一下，函数的第一个参数是在寄存器%rdi中传递的。
•您注入的代码应将寄存器设置为cookie，然后使用ret指令将控制权转移到touch2中的第一条指令。
•不要试图在利用漏洞的代码中使用jmp或调用指令。这些指令的目的地地址编码很难制定。使用ret指令进行所有控制权转移，即使您没有从呼叫返回。
•请参阅附录B中关于如何使用工具生成指令序列的字节级表示的讨论。

## 4.3 Level 3
Phase 3 also involves a code injection attack, but passing a string as argument. Within the file ctarget there is code for functions hexmatch and touch3 having the following C representations:

第3阶段还涉及代码注入攻击，但传递一个字符串作为参数。在文件ctaarget中，有用于函数hexmatch和touch3的代码，它们具有以下C表示：

```c
/* Compare string to hex represention of unsigned value */
int hexmatch(unsigned val, char *sval)
{
    char cbuf[110];
    /* Make position of check string unpredictable */
    char *s = cbuf + random() % 100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval)
{
    vlevel = 3; /* Part of validation protocol */
    if (hexmatch(cookie, sval)) {
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    } else {
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}
```

Your task is to get CTARGET to execute the code for touch3 rather than returning to test. You must make it appear to touch3 as if you have passed a string representation of your cookie as its argument. 

您的任务是让CTARGET执行touch3的代码，而不是返回测试。你必须让它看起来像touch3，就好像你传递了一个cookie的字符串表示作为它的参数。

**Some Advice**:
• You will need to include a string representation of your cookie in your exploit string. The string should consist of the eight hexadecimal digits (ordered from most to least significant) without a leading “0x.”
• Recall that a string is represented in C as a sequence of bytes followed by a byte with value 0. Type “man ascii” on any Linux machine to see the byte representations of the characters you need.
• Your injected code should set register %rdi to the address of this string.
• When functions hexmatch and strncmp are called, they push data onto the stack, overwriting portions of memory that held the buffer used by getbuf. As a result, you will need to be careful where you place the string representation of your cookie.

**一些建议**：
•您需要在漏洞利用字符串中包含cookie的字符串表示。字符串应由八个十六进制数字组成（从最高到最低有效），不带前导“0x”
•回想一下，字符串在C中表示为一个字节序列，后跟一个值为0的字节。在任何Linux机器上键入“man-ascii”以查看所需字符的字节表示。
•注入的代码应将寄存器%rdi设置为此字符串的地址。
•当调用函数hexmatch和strncmp时，它们会将数据推送到堆栈上，从而覆盖内存中包含getbuf使用的缓冲区的部分。因此，您需要小心放置cookie的字符串表示的位置。

# 5 Part II: Return-Oriented Programming
Performing code-injection attacks on program RTARGET is much more difficult than it is for CTARGET, because it uses two techniques to thwart such attacks:
• It uses randomization so that the stack positions differ from one run to another. This makes it impossible to determine where your injected code will be located.
• It marks the section of memory holding the stack as nonexecutable, so even if you could set the program counter to the start of your injected code, the program would fail with a segmentation fault.

对程序RTARGET执行代码注入攻击比CTARGET困难得多，因为它使用两种技术来阻止这种攻击：
•它使用随机化，因此每次运行的堆栈位置不同。这使得无法确定注入代码的位置。
•它将保存堆栈的内存部分标记为不可执行，因此即使您可以将程序计数器设置为注入代码的开头，程序也会因分段错误而失败。

Fortunately, clever people have devised strategies for getting useful things done in a program by executing existing code, rather than injecting new code. The most general form of this is referred to as return-oriented programming (ROP) [1, 2]. The strategy with ROP is to identify byte sequences within an existing program that consist of one or more instructions followed by the instruction ret. Such a segment is referred to as a gadget.

幸运的是，聪明的人已经制定了策略，通过执行现有代码，而不是注入新代码，在程序中完成有用的事情。最常见的形式称为面向返回编程（ROP）[1，2]。ROP的策略是识别现有程序中的字节序列，该序列由一条或多条后面跟着指令ret.这样的段被称为小工具。

![Figure2](/images/Fig2.png)

**Stack Figure 2**: Setting up sequence of gadgets for execution. Byte value 0xc3 encodes the ret instruction. gadget. Figure 2 illustrates how the stack can be set up to execute a sequence of n gadgets. In this figure, the stack contains a sequence of gadget addresses. Each gadget consists of a series of instruction bytes, with the final one being 0xc3, encoding the ret instruction. When the program executes a ret instruction starting with this configuration, it will initiate a chain of gadget executions, with the ret instruction at the end of each gadget causing the program to jump to the beginning of the next.

**堆栈图2**：设置小工具的执行顺序。字节值0xc3对ret指令进行编码。小工具图2展示了如何设置堆栈来执行一系列n个小工具。在该图中，堆栈包含一系列小工具地址。每个小工具都由一系列指令字节组成，最后一个字节是0xc3，用于编码ret指令。当程序从该配置开始执行ret指令时，它将启动一系列小工具执行，每个小工具末尾的ret指令会导致程序跳到下一个小工具的开头。

A gadget can make use of code corresponding to assembly-language statements generated by the compiler, especially ones at the ends of functions. In practice, there may be some useful gadgets of this form, but not enough to implement many important operations. For example, it is highly unlikely that a compiled function would have popq %rdi as its last instruction before ret. Fortunately, with a byte-oriented instruction set, such as x86-64, a gadget can often be found by extracting patterns from other parts of the instruction byte sequence.

小工具可以使用与编译器生成的汇编语言语句相对应的代码，尤其是函数末尾的代码。在实践中，可能有一些这种形式的有用小工具，但不足以实现许多重要的操作。例如，编译后的函数不太可能在返回之前将`popq %rdi`作为其最后一条指令。幸运的是，对于面向字节的指令集，如x86-64，通常可以通过从指令字节序列的其他部分提取模式来找到小工具。

For example, one version of rtarget contains code generated for the following C function:

```c
void setval_210(unsigned *p)
{
    *p = 3347663060U;
}
```

The chances of this function being useful for attacking a system seem pretty slim. But, the disassembled machine code for this function shows an interesting byte sequence:

这个函数对攻击系统有用的可能性似乎很小。但是，这个函数的反汇编机器代码显示了一个有趣的字节序列：

```asm
0000000000400f15 <setval_210>:
    400f15: c7 07 d4 48 89 c7 movl $0xc78948d4,(%rdi)
    400f1b: c3 retq
```

The byte sequence 48 89 c7 encodes the instruction `movq %rax, %rdi`. (See Figure 3A for the encodings of useful movq instructions.) This sequence is followed by byte value c3, which encodes the ret instruction. The function starts at address 0x400f15, and the sequence starts on the fourth byte of the function. Thus, this code contains a gadget, having a starting address of 0x400f18, that will copy the 64-bit value in register %rax to register %rdi.

字节序列48 89 c7对指令`movq %rax，%rdi`进行编码。（有关有用的movq指令的编码，请参见图3A。）此序列后面是字节值c3，它对ret指令进行编码。函数从地址0x400f15开始，序列从函数的第四个字节开始。因此，此代码包含一个起始地址为0x400f18的小工具，它将把寄存器%rax中的64位值复制到寄存器%rdi。

Your code for RTARGET contains a number of functions similar to the setval_210 function shown above in a region we refer to as the gadget farm. Your job will be to identify useful gadgets in the gadget farm and use these to perform attacks similar to those you did in Phases 2 and 3.

RTARGET的代码包含许多函数，这些函数类似于上面显示的setval_210函数，位于我们称之为小工具场的区域中。您的工作将是在小工具场中识别有用的小工具，并使用这些小工具执行类似于第2阶段和第3阶段的攻击。

**Important**: The gadget farm is demarcated by functions start_farm and end_farm in your copy of rtarget. Do not attempt to construct gadgets from other portions of the program code.

**重要**：小工具场由rtarget副本中的函数start_farm和end_farm划分。不要试图从程序代码的其他部分构造小工具。

## 5.1 Level 2
For Phase 4, you will repeat the attack of Phase 2, but do so on program RTARGET using gadgets from your gadget farm. You can construct your solution using gadgets consisting of the following instruction types, and using only the first eight x86-64 registers (%rax–%rdi).

对于第4阶段，您将重复第2阶段的攻击，但使用小工具场中的小工具在程序RTARGET上执行此操作。您可以使用由以下指令类型组成的小工具构建解决方案，并且只使用前八个x86-64寄存器（%rax–%rdi）。

`movq` : The codes for these are shown in Figure 3A.
`popq` : The codes for these are shown in Figure 3B.
`ret` : This instruction is encoded by the single byte 0xc3.
`nop` : This instruction (pronounced “no op,” which is short for “no operation”) is encoded by the single byte 0x90. Its only effect is to cause the program counter to be incremented by 1.

`movq`：这些代码如图3A所示。
`popq`：这些代码如图3B所示。
`ret `：此指令由单字节0xc3编码。
`nop`：此指令（发音为“no op”，是“no operation”的缩写）由单字节0x90编码。它的唯一作用是使程序计数器增加1。

**Some Advice**:
* All the gadgets you need can be found in the region of the code for rtarget demarcated by the functions start_farm and mid_farm.
* You can do this attack with just two gadgets.
* When a gadget uses a popq instruction, it will pop data from the stack. As a result, your exploit string will contain a combination of gadget addresses and data.

**一些建议**：
* 您需要的所有小工具都在函数start_farm和mid_farm之间。
* 您只需使用两个小工具即可进行此攻击。
* 当小工具使用popq指令时，它将从堆栈中弹出数据。因此，您的漏洞字符串将包含小工具地址和数据的组合。

## 5.2 Level 3
Before you take on the Phase 5, pause to consider what you have accomplished so far. In Phases 2 and 3, you caused a program to execute machine code of your own design. If CTARGET had been a network server, you could have injected your own code into a distant machine. In Phase 4, you circumvented two of the main devices modern systems use to thwart buffer overflow attacks. Although you did not inject your own code, you were able inject a type of program that operates by stitching together sequences of existing code.

在你进入第五阶段之前，停下来思考一下你迄今为止取得了什么成就。在第2和第3阶段中，您让一个程序执行您自己设计的机器代码。如果CTARGET是一个网络服务器，您可以将自己的代码注入到远处的机器中。在第4阶段，您绕过了现代系统用来阻止缓冲区溢出攻击的两个主要设备。虽然您没有注入自己的代码，但您可以注入一种通过将现有代码序列拼接在一起来操作的程序类型。

You have also gotten 95/100 points for the lab. That’s a good score. If you have other pressing obligations consider stopping right now.

你在实验室也得了95/100分。这是一个不错的分数。如果你有其他紧迫的义务，考虑现在就停止。

Phase 5 requires you to do an ROP attack on RTARGET to invoke function touch3 with a pointer to a string representation of your cookie. That may not seem significantly more difficult than using an ROP attack to invoke touch2, except that we have made it so. Moreover, Phase 5 counts for only 5 points, which is not a true measure of the effort it will require. Think of it as more an extra credit problem for those who want to go beyond the normal expectations for the course.

阶段5要求您对RTARGET进行ROP攻击，以使用指向cookie字符串表示的指针调用函数touch3。这似乎并不比使用ROP攻击来调用touch2困难得多，除非我们已经做到了。此外，第5阶段只占5分，这并不是衡量其所需努力的真正标准。对于那些想超出正常预期的人来说，这更像是一个额外的学分问题。