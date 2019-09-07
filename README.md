## reef - basic tools in c

### 为何取名 reef?

[reef triggerfish](https://en.wikipedia.org/wiki/Reef_triggerfish) = Humuhumunukunukuāpuaa，
一种比较特别的鱼名，仅此而已。


### 环境

reef 用标准C99编写，运行于 GNU/Linux 系统, 使用了\_GNU_SOURCE扩展功能， 在X86 Linux、OSX、和某些 ARM Linux上完成测试。
不保证Solaris, FreeBSD, Windows...下顺利运行。


### 目的

c 很难用，她只给了我们基础，而非全部，reef 是这个全部的开始。

reef 包括：

* 字符串、列表、二进制内存块、哈希、字典（mdf）

* 单元测试框架

* 线程安全日志

* 单线程测时工具

* 错误返回(MERR)

* json 数据解析

* message pack 打包、解包

* 一个写着好玩的正则表达式工具

* 一个常见网络服务 的客户端API封装(memcache)

* CGI 协议基础函数

* HTTP 协议基础函数


### 为何返回 MERR* 而非 int

函数返回整型，0为正常，其它异常。

reef 没有遵守这个约定，因为我想在程序运行异常时快速定位是谁在第几行调用了谁，是什么鬼与预期不符。
MERR* 可以，虽然使用起来稍显麻烦。


### MDF 适合干嘛

mdf 类似 Python 中的 dict, Javascript 中的 object, Lua 中的 Table。有丰富的操作函数。可以用来保存 web 开发 90% 左右的数据。


### 出了问题怎么办

先写个 test 进行确认，真有问题，你需要看看源代码。


### 怪异关键字

* QJYQ - 奇技淫巧

  一些很不规范的操作，谨慎使用。

* TCTR - too complex to remember

  现实中太复杂的业务场景，或者仅仅是代码实现得太复杂，时间太久容易忘记。照着注释来就行。
