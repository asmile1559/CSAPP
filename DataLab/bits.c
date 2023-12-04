/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
  
  将return语句替换为一系列的C表达式代码，代码的组织形式如下
  
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      /*简单的描述你的工作是如何实现的*/
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  每一个表达式只能使用如下的形式

  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  1. 整数常数 的范围在 0-255之间，不允许使用32位整数的范围

  2. Function arguments and local variables (no global variables).
  2.函数参数和局部变量

  3. Unary integer operations ! ~
  3. 单目运算符 ! ~

  4. Binary integer operations & ^ | + << >>
  4. 双目运算符 & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.
  每一个表达式或许包含多个运算符,不一定每个运算符都占用一行

  You are expressly forbidden to:
  以下为禁止事项

  1. Use any control constructs such as if, do, while, for, switch, etc.
  1. 使用任何的控制结构

  2. Define or use any macros.
  2. 定义或使用任何宏

  3. Define any additional functions in this file.
  3. 定义任何其他的函数

  4. Call any functions.
  4. 调用任何函数

  5. Use any other operations, such as &&, ||, -, or ?:
  5. 使用其他的运算符

  6. Use any form of casting.
  6. 使用任何形式的类型转换

  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.
  7. 使用任何除了int外的其余数据结构, 数组, 结构体, 联合体
 
  You may assume that your machine:
  你可以假设自己的机器

  1. Uses 2s complement, 32-bit representations of integers.
  1. 使用补码数, 用32位整数表示

  2. Performs right shifts arithmetically.
  2. 使用算数右移

  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.
  3. 进行shift运算时,如果数量小于0或大于31会产生不可预测的结果

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     /* 利用移位运算计算2的幂 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.
由于浮点运算的特点,你的代码规则要求有一定程度的减弱.
允许使用循环和条件控制语句,
允许使用补码和无符号数的所有形式,
允许任意数值的整数和无符号常量,
允许使用算数逻辑或者比较运算符对整数或者无符号数

You are expressly forbidden to:
  以下是被禁止的

  1. Define or use any macros.
  1. 定义或使用任何宏

  2. Define any additional functions in this file.
  2. 定义任何其他函数

  3. Call any functions.
  3. 调用任何函数

  4. Use any form of casting.
  4. 使用任何形式的转换
  
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  5. 使用任何其他形式的数据类型

  6. Use any floating point data types, operations, or constants.
  6. 使用任何浮点数据类型,运算符和常量

NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  1. dlc编译器用于检查题目的合法性

  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  2. 每个函数都有最大操作符的数量限制(整数, 逻辑运算符和比较运算符)

  3. Use the btest test harness to check your functions for correctness.
  3. 使用btest检查程序的正确性

  4. Use the BDD checker to formally verify your functions
  4. 用BDD检查其取验证你的函数

  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.
  5. 每个函数的最大操作数在每个函数的标题注释中给出,
     如果writeup文件和该文件中的最大操作之间存在任何不一致，
     请将该文件视为权威来源。
/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
// 1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y)
{
  int a = ~x & y; // 记录 x 中的 0 与 y 中的 1
  int b = ~y & x; // 记录 y 中的 0 与 x 中的1
  return ~((~a) & (~b)); // 德摩根律
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void)
{
  return 1 << 31;
}
// 2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x)
{
  /** 
   * Tmax + 1 = Tmin
   * ~Tmin = Tmax
   * !(x ^ y) => x == y ? 1 : 0 
   * !!a 排除 -1 的干扰
  */
  int a = x + 1;
  return (!!a) & !(~a ^ x);
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x)
{
  // 填充 x 使其变为 0xffffffff
  x = x | 0x55;
  x = x | (0x55 << 8);
  x = x | (0x55 << 16);
  x = x | (0x55 << 24);
  return !(x ^ (~0));
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x)
{
  return ~x + 1;
}
// 3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x)
{
  int a = ~0x30 + 1;
  int b = ~0x3a + 1;
  a = (x + a) >> 31;
  b = (x + b) >> 31;
  return (!a) & b;
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z)
{
  int m = ~0;
  x = !x;
  return ((x + m) & y) | ((!x + m) & z);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y)
{
  int eq = !(x ^ y); // x == y
  int a = (x >> 31) & 1;
  int b = (y >> 31) & 1;
  int f1 = !((!a) & b); // x > 0, y < 0 => f1 = 0
  int f2 = a & (!b);    // x < 0, y > 0 => f2 = 1
  int r = ((y + (~x + 1)) >> 31) & 1;
  return f1 & (f2 | eq | (!r));
}
// 4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x)
{
  // 一个数的相反数符号与原数不同，0除外
  int a = (~x + 1) | x; 
  a = ((~a) >> 31) & 1;
  return a;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x)
{
  // 真不会， 抄一个网上的
  int b16, b8, b4, b2, b1, b0;
  int sign = x >> 31;
  x = (sign & ~x) | (~sign & x); // 如果x为正则不变，否则按位取反（这样好找最高位为1的，原来是最高位为0的，这样也将符号位去掉了）

  // 不断缩小范围
  b16 = !!(x >> 16) << 4; // 高十六位是否有1
  x = x >> b16;           // 如果有（至少需要16位），则将原数右移16位
  b8 = !!(x >> 8) << 3;   // 剩余位高8位是否有1
  x = x >> b8;            // 如果有（至少需要16+8=24位），则右移8位
  b4 = !!(x >> 4) << 2;   // 同理
  x = x >> b4;
  b2 = !!(x >> 2) << 1;
  x = x >> b2;
  b1 = !!(x >> 1);
  x = x >> b1;
  b0 = x;
  return b16 + b8 + b4 + b2 + b1 + b0 + 1; //+1表示加上符号位
}
// float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf)
{
  // 根据 fshow 找规律
  int scale = uf >> 31;
  int exp = ((uf >> 23) & 0xff);
  int frac = ((uf) << 9) >> 9;

  if (exp == 0xff)
    return uf;

  if (exp != 0)
    exp = exp + 1;
  else
    frac = frac + frac;

  scale = (scale << 8) | exp;
  scale = (scale << 23) | frac;
  return scale;
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf)
{
  int sign = (uf >> 31) & 0x1; // 符号位
  int exp = ((uf >> 23) & 0xff); // 指数位
  int frac = ((uf) << 9) >> 9; // 尾数位
  int de = 1 << (exp - 0x7f); // 基础位置
  int i = 0;
  int d = (0x800000 >> (exp - 0xff));

  if (exp == 0xff)
    return 0x80000000u;

  if(exp > 159) // 超过int的最大范围 127 + 32
    return 0x80000000u;

  if (exp < 127)
    return 0;

  i = frac / d;
  return sign ? -(de + i) : de + i;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x)
{
  unsigned pow = 0;
  unsigned frac = 0x400000;

  if (x < -149)
    return 0;
  else if (x >= 128)
    return 0x07f800000;

  if (x >= -126)
  {
    pow = x + 127;
    pow = pow << 23;
  }
  else
  {
    pow = -(x + 127);
    pow = frac >> pow;
  }
  return pow;
}
