## reef - web basic tools in c

### 为何取名 reef?

[reef triggerfish](https://en.wikipedia.org/wiki/Reef_triggerfish) = Humuhumunukunukuāpuaa，
一种比较特别的鱼名，仅此而已。


### 环境

reef 用标准C99编写，运行于 GNU/Linux 系统，使用了\_GNU_SOURCE扩展功能。
暂不支持Solaris, FreeBSD, Mac OS X...下运行


### 目的

c 很难用，她只给了我们基础，而非全部，reef 是这个全部的开始。


### 为何返回 MER* 而非 int

函数返回整型，0为正常，其它异常。

reef 没有遵守这个约定，因为我们想在程序运行异常时快速定位是谁调用了谁，在第几行调用了谁，
是什么鬼与预期不符。 MERR* 可以，虽然使用起来稍显麻烦。


### MDF 适合干嘛

mdf 类似 Python 中的 dict, Javascript 中的 object。可以用来保存 web 开发 90% 左右的数据。


### 出了问题怎么办

先写个 test 进行确认，真有问题，你需要看看源代码。
