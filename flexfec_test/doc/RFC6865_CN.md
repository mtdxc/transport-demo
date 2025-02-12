<h1>FEC帧的简单Reed-Solomon前向纠错（FEC）方案</h1>

# 摘要

本文为有限域（也称为伽罗瓦域）GF（2^^m）[2<=m<=16]上的Reed-Solomon码，描述了一种完全指定的简单前向纠错（FEC）方案。可保护沿FECFRAME定义线路line上的任意媒体流。考虑到Reed-Solomon码具有吸引人的特性，由于它提供了对分组擦除的最佳保护，并且源符号是编码符号的一部分，这可大大简化解码。然而，付出的代价是对最大源块大小、编码符号的最大数量的限制，以及比低密度奇偶校验（LDPC）码更高的计算复杂度。

# 1.介绍

使用前向纠错（FEC）码是提高内容交付协议（CDP）和应用的单播、多播和广播的可靠性的经典解决方案。[RFC6363]描述了将FEC方案用于媒体交付应用程序的通用框架，例如用于基于实时传输协议（RTP）的实时流媒体应用程序。类似地，[RFC5052]描述了基于异步分层编码（ALC）[RFC5775]和面向NACK的可靠多播（NORM）[RFC5740]传输协议，将FEC方案用于对象交付应用程序（这边对象是文件）的通用框架。

更具体地说，[RFC5053]和[RFC5170]等FEC方案为对象传递协议（如ALC和NORM）引入了基于稀疏奇偶校验矩阵的擦除码。在处理“小”对象时，这些代码在处理方面是有效的，但在擦除恢复能力方面不是最佳的。

本文描述的Reed-Solomon FEC码属于最大距离可分（MDS）码，在擦除恢复能力方面是最佳的。这意味着接收器可从任何一组精确的k个编码符号中恢复k个源符号。这也是对称编码，这意味着k个源符号是编码符号的一部分。然而，它对最大源块大小和编码符号数量方面有限制。由于媒体交付应用程序的实时属性，通常会限制最大源块大小，因此对于许多（但不是所有）用例，这在FECFRAME上下文中并不是一个主要问题。此外，虽然里德-所罗门码的编码/解码复杂度高于[RFC5053]或[RFC5170]码，即使是软件编解码器实现，对于大多数使用情况也是可行的。

许多处理可靠内容传输或内容存储的应用程序已经依赖于基于分组的Reed-Solomon擦除恢复码。特别是，他们中的许多人使用Luigi Rizzo[RS codec][Rizzo97]的Reed Solomon编解码器。本文档的目标是指定一个与此编解码器兼容的简单Reed-Solomon方案。

更具体地说，[RFC5510]引入了里德-所罗门码，及与[RFC5052]框架兼容的若干相关FEC方案。本文件继承了[RFC5510]第8节“为擦除信道的Reed-Solomon代码规范”，基于Vandermonde矩阵的核心Reed-Solomon码的规范，并指定了一与FECFRAME[RFC6363]兼容的简单FEC方案：

FEC编码ID为8的全指定的FEC方案，指定了一种简单的方法来使用GF（2^^m）上的Reed-Solomon码，其中2<=m<=16，以保护任意应用数据单元（ADU）流。

因此，不考虑[RFC5510]中的第4、5、6和7节，及[RFC5052]定义的特定格式和过程，并将其替换为FECFRAME特定格式和过程。

例如，使用该方案，来自一个或多个媒体传输应用的一组应用数据单元（ADU，如RTP包）被分组在一个ADU块Block中，并作为一个整体来进行FEC编码。对于GF（2^^8）上的Reed-Solomon码，能一起保护的ADU数量有严格限制，因为编码符号数量n必须小于或等于255。当使用更大的有限域大小（m>8）时，该约束会得到减少，但代价是增加计算复杂度。

# 2. 术语

本文件中的关键词“必须”、“不得”、“要求”、“应”、“不应”、“应”、“不应”、“建议”、“可”和“可选”应按照RFC 2119[RFC2119]中所述进行解释。


# 3. 定义、符号和缩写

## 3.1. 定义

本文件使用以下术语和定义。其中一些术语和定义是[RFC5052]FEC方案指定的：

- 源符号：编码过程中使用的数据单位。在本规范中，每个ADU始终有一个源符号。
- 编码符号：编码过程产生的数据单位。对于对称编码来说，源符号是编码符号的一部分。
- 修复符号：编码符号中不是源符号的符号。
- 编码比率：k/n比率，即源符号数与编码符号数之比。根据定义，编码比率为：0<编码比率<=1。接近1的比率表示在编码过程中产生了少量修复符号。
- 对称编码：FEC编码，其中源符号是编码符号的一部分。本文件中介绍的Reed-Solomon编码是对称的。
- 源代码块：建议一起编码的k个源代码块
- 包擦除通道：数据包被丢弃（例如，被拥塞的路由器丢弃，或者因为传输错误的数量超过了物理层代码的纠正能力）或被接收的通信路径。当接收到数据包时，假定该数据包未损坏。

其中一些术语和定义是特定于[RFC6363]框架的：
- 应用数据单元（ADU）：提供给传输层的源数据单元负载。根据用例，ADU可使用RTP封装。
- （源）ADU流：与传输层流标识符（例如标准5元组{源IP地址、源端口、目标IP地址、目标端口、传输协议}）关联的ADU序列。根据使用情况，**多个ADU流可通过同一FECFRAME来保护**。
- ADU块：FECFRAME实例为了FEC方案，而考虑一起编码的一组ADU。与流ID（F[]、长度（L[]）和padding（Pad[]）字段一起，它们构成了执行FEC编码的源符号集。
- ADU信息（ADUI）：由ADU和相关流ID、长度和填充字段组成的一堆数据（第4.3节）。它将用作源符号。
- FEC框架配置信息（FFCI）：控制FECFRAME操作的信息。FFCI允许同步发送方和接收方实例的FECFRAME。
- FEC源数据包：在发送方（接收方），提交给（接收到）包含ADU的传输协议的有效载荷，以及明确源FEC载荷ID（必须存在于本文件定义的FEC方案中，见第5.1.2节）。
- FEC修复包：在发送方（接收方）提交（接收到）传输协议的有效载荷，包含一个修复符号，以及修复FEC载荷ID，可能还有一个RTP报头。

上述术语如图1所示（以发送者的角度）：
```
   +----------------------+
   |     Application      |
   +----------------------+
              |
              | (1) Application Data Units (ADUs)
              |
              v
   +----------------------+                           +----------------+
   |       FECFRAME       |                           |                |
   |                      |-------------------------->|   FEC Scheme   |
   |(2) Construct source  |(3) Source Block           |                |
   |    blocks            |                           |(4) FEC Encoding|
   |(6) Construct FEC     |<--------------------------|                |
   |    source and repair |                           |                |
   |    packets           |(5) Explicit Source FEC    |                |
   +----------------------+    Payload IDs            +----------------+
              |                Repair FEC Payload IDs
              |                Repair symbols
              |
              |(7) FEC source and repair packets
              v
   +----------------------+
   |   Transport Layer    |
   |     (e.g., UDP)      |
   +----------------------+
        
```
图1：本文件中使用的术语（发送方）。

## 3.2. 符号

本文件使用以下符号。其中一些是FEC方案指定的。
- k 表示源块中源符号的数量
- max_k 表示任何源块的最大源符号数
- n 表示为源块生成的编码符号数
- E 表示编码符号长度（以字节为单位）
- GF(q) 表示具有q元素的有限域（也称为伽罗瓦域）。在本文件中，我们假设q=2^^m
- m定义有限域中元素的长度，单位为位。在本文件中，m等于2<=m<=16。
- q定义有限域中的元素数。本规范中有：q=2^^m。
- CR表示“编码比率”，即k/n。
- a^^b表示a升到b的幂。

其中一些是特定于帧的：
- B表示每个ADU块的ADU数量。
- max_B表示任何ADU块的最大ADU数量。

## 3.3. 缩写
本文件使用以下缩写：
- ADU代表应用程序数据单元
- ADUI代表应用程序数据单元信息
- ESI代表编码符号ID
- FEC代表前向纠错（或擦除）码
- FFCI代表FEC框架配置信息
- FSSI代表FEC方案特定信息
- MDS代表最大距离可分离代码
- SBN代表源块编号
- SDP代表会话描述协议。

# 4. 与ADU块和源块创建相关的通用程序

This section introduces the procedures that are used during the ADU block and the related source block creation for the FEC scheme considered.
本节介绍FEC方案建议的，在ADU块和相关源块创建过程。

## 4.1. 限制
本规范有以下限制：
- 每个ADUI必须有只一个源符号，因此每个ADU必须有一个源符号；
- 每个FEC修复包必须只有一个修复符号
- 每个ADU块必须只有一个源块。

## 4.2. ADU块创建
存在两种影响ADU块创建的限制：
- 在FEC方案级别：有限字段大小（m参数）直接影响最大源块大小和最大编码符号数；
- 在FECFRAME实例级别：目标用例可具有实时约束，这些约束可以/将定义最大ADU块大小。

注意，术语“最大源块大小”和“最大ADU块大小”取决于所采用的观点（FEC方案与FEC帧实例）。然而，在本文件中，两者均指相同的值，因为第4.1节要求每个ADU只有一个源符号。我们现在详细介绍这些方面。

有限字段大小参数m定义此字段中非零元素的数量，等于：q-1=2^^m-1。该q-1值也是源块可以产生的理论最大编码符号数。例如，当m=8（默认值）时，最多有2^^8-1=255个编码符号，所以 k < n <= 255。给定目标FEC编码速率（例如，在启动FECFRAME实例时由最终用户或上层应用程序提供，并考虑已知或估计的分组丢失率），发送方计算：

max_k = floor((2^^m - 1) * CR)

该max_k值为发送方留出足够的空间来生成所需数量的修复符号。由于每个ADU有一个源符号，因此max_k也是每个ADU块的最大ADU数的上限。

源ADU流可能具有实时约束。当有多个流，具有不同的实时约束时，让我们考虑最严格的约束（参见[RCF6363]，第10.2节，项目6，当几个流被全局保护）时的建议。在这种情况下，ADU块的最大ADU数量不得超过某个阈值，因为它直接影响解码延迟。ADU块大小越大，解码器可能必须等待的时间越长，直到接收到足够数量的编码符号才能成功解码，因此解码延迟越大。当目标用例已知时，这些实时约束会导致ADU块大小的上限max_rt。

例如，如果用例指定了最大解码延迟l，并且如果每个源ADU覆盖连续媒体的持续时间d（我们在此假设恒定比特率ADU流的简单情况），则ADU块大小不得超过：
 max_rt = floor(l / d)

 编码后，该块将产生一组最多n=max_rt/CR的编码符号。这些n个编码符号必须以每秒n/l个数据包的速率发送。例如，d=10毫秒，l=1秒，最大值=100 ADUs。

 如果我们考虑所有这些约束，我们会发现：

 max_B = min(max_k, max_rt)

 该max_B参数是可构成ADU块的ADU数量的上限。

 ## 4.3. 源块创建

 在最一般的形式中，FECFRAME 和 Reed-Solomon FEC方案旨在保护一组不同的流。由于流彼此之间没有关系，因此每个流的ADU大小可能有显著的不同。即使在单个流的特殊情况下，ADU大小也可以大的变化（例如，H.264流的“图片组”（GOP）的各种帧将有不同大小）。必须解决这种多样性，因为Reed-Solomon FEC方案要求每个源块必须有相同的编码符号大小（E参数）。由于本规范要求每个ADU只有一个源符号，因此E必须足够大，来包含ADU块的所有ADU及其前置3字节（见下文）。

当每个源块确定E后（默认情况下，由FFCI/FSSI指定，S=0，第5.1.1.2节），E等于该源块最大ADU的大小加上3（对于提前加的3个字节；见下文）。在这种情况下，在接收到该源块的第一个FEC修复数据包时，由于该数据包必须包含一个修复符号（第5.1.3节），接收端可确定用于该源块的E参数。

在E固定的情况下（由FFCI/FSSI规定，S=1，第5.1.1.2节），则E必须大于或等于该源块最大ADU的大小加上3（对于提前加的3个字节；见下文）。如果不是这样，将返回一个错误。如何处理该错误是特定于用例的（例如，可以使用适当的机制在更新的FFCI消息中向接收机传送较大的e参数），本规范不考虑该错误。

ADU块始终编码为单个源块。该ADU区块中总共有B <= max_B个ADU。对于ADU i，如果0<=i<=B-1，则在前面加上3个字节（图2）：

- 第一个字节F[i]（流ID）包含与此ADU所属的源ADU流关联的整数标识符。假设单个字节就足够了，或者换言之，单个FECFRAME实例将保护不超过256个流。
- 以下2个字节L[i]（长度）包含此ADU的长度，按网络字节顺序排列（即大端字节）。该长度用于ADU本身，不包括F[i]、L[i]或Pad[i]字段。

然后，在字段Pad[i]中将零填充添加到ADU i（如果需要），以便对齐，大小精确到E字节。由ADU i和F[i]、L[i]和Pad[i]字段产生的数据单元称为ADU信息（或ADUI）。每个ADUI只贡献源块的一个源符号。

```
                     Encoding Symbol Length (E)
   < ----------------------------------------------------------------- >
   +----+--------+-----------------------+-----------------------------+
   |F[0]|  L[0]  |        ADU[0]         |            Pad[0]           |
   +----+--------+----------+------------+-----------------------------+
   |F[1]|  L[1]  | ADU[1]   |                         Pad[1]           |
   +----+--------+----------+------------------------------------------+
   |F[2]|  L[2]  |                    ADU[2]                           |
   +----+--------+------+----------------------------------------------+
   |F[3]|  L[3]  |ADU[3]|                             Pad[3]           |
   +----+--------+------+----------------------------------------------+
   \_________________________________  ________________________________/
                                     \/
                            simple FEC encoding

   +-------------------------------------------------------------------+
   |                              Repair 4                             |
   +-------------------------------------------------------------------+
   .                                                                   .
   .                                                                   .
   +-------------------------------------------------------------------+
   |                              Repair 7                             |
   +-------------------------------------------------------------------+

    图2: 源块创建, 编码比率为 1/2 (源块和修复块个数相同; 在这里是4), S = 0.
```
请注意，初始3个字节和可选填充都不通过网络发送的。但是，在FEC编码期间会考虑它们。这意味着，如果FEC解码成功，丢失某个FEC源数据包（例如，包含该FEC源数据包的UDP数据报）的接收器将能够恢复ADUI。由于最初的3个字节，此接收器将消除填充（如果有）并识别相应的ADU流。

# 5. 任意ADU流GF（2^^m）上的简单Reed-Solomon FEC格式

这个完全指定的FEC方案指定在GF（2^^m）上使用Reed-Solomon码，其中2<=m<=16，并对任意分组流使用简单的FEC编码。

## 5.1. 格式和代码
### 5.1.1. FEC框架配置信息
FEC框架配置信息（或FFCI）包括必须在发送方和接收方之间通信的信息[RFC6363]。更具体地说，它使FECFRAME发送方和接收方实例的同步。它包括强制性要素和方案特定要素，详情如下。

#### 5.1.1.1. 强制性信息
FEC编码ID：分配给这个完全指定的FEC方案的值必须是IANA分配的8（第8节）。

当SDP用于传递FFCI时，此FEC编码ID必须包含在RFC 6364[RFC6364]中指定的“FEC修复流”属性的“编码ID”参数中

#### 5.1.1.2. FEC方案特定信息
FEC方案特定信息（FSSI）包括特定于当前FEC方案的元素。更准确地说
- 编码符号长度（E）：一个非负整数，小于2^^16，表示每个编码符号的字节长度（“严格”模式，即，如果S=1），或任何编码符号的最大长度（即，如果S=0）。
- 严格（S）标志：当设置为1时，该标志表示E参数是会话每个块的实际编码符号长度值（除非更新的FFCI另有通知，如果用例或CDP考虑了这种可能性）。当设置为0时，该标志指示E参数是会话的每个块的最大编码符号长度值（除非更新的FFCI另有通知，如果用例或CDP考虑了这种可能性）。
- m参数（m）：定义有限域中元素长度的整数，单位为位。我们有：2<=m<=16。

发送方（里德-所罗门编码器）和接收方（里德-所罗门解码器）都需要这些元件。

当SDP用于传递FFCI时，该FEC方案特定信息必须以RFC 6364[RFC6364]中规定的文本表示形式，包含在“FEC修复流”属性的“fssi”参数中。例如：

   a=fec-repair-flow: encoding-id=8; fssi=E:1400,S:0,m:8

如果另一种机制要求FSSI作为不透明的八位字节字符串携带（例如在Base64编码之后），则编码格式由图3中的以下3个八位字节组成：
- 编码符号长度（E）：16位字段。
- 严格（S）标志：1位字段。
- m参数（m）：7位字段。
```
    0                   1                   2
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Encoding Symbol Length (E)  |S|     m       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
                      Figure 3: FSSI encoding format.

## 显式源FEC负载ID
FEC源数据包必须包含一个附加到数据包末尾的显式源FEC有效负载ID，如图4所示。
```
   +--------------------------------+
   |           IP Header            |
   +--------------------------------+
   |        Transport Header        |
   +--------------------------------+
   |              ADU               |
   +--------------------------------+
   | Explicit Source FEC Payload ID |
   +--------------------------------+
```
    Figure 4: 具有显式源FEC负载ID的FEC源数据包结构.

更准确地说，显式源FEC负载ID由源块编号、编码符号ID和源块长度组成。前两个字段的长度取决于m参数（在FFCI第5.1.1.2节中单独传输）:

   -  源块编号 (SBN) ((32-m)-bit field): 此字段标识此FEC源数据包所属的源块.
   -  编码符号ID (ESI) (m-bit field): 此字段标识此FEC源数据包中包含的源符号。对于源符号，该值为0<=ESI<=k-1.
   -  源块长度 (k) (16-bit field): 此字段提供此源块的源符号数，即k参数. 如果m<=8时16位太多，而在8<m<=16时需要16位。因此，我们提供了一个单一的通用格式，与m无关.
```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Source Block Number (24 bits)       | Enc. Symb. ID |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Source Block Length (k)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
   图5:m=8的源FEC负载ID编码格式（默认值）.

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Source Block Nb (16 bits)   |   Enc. Symbol ID (16 bits)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Source Block Length (k)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
        图6:m=16的源FEC负载ID编码格式.

   图5和图6分别说明了m=8和m=16的源FEC负载ID的格式.

5.1.3.  修复FEC负载ID

   FEC修复数据包必须包含修复FEC负载ID，该ID在修复符号之前，如图7所示。每个FEC修复数据包必须只包含一个修复符号。
```
   +--------------------------------+
   |           IP Header            |
   +--------------------------------+
   |        Transport Header        |
   +--------------------------------+
   |      Repair FEC Payload ID     |
   +--------------------------------+
   |         Repair Symbol          |
   +--------------------------------+
```
      图7：具有修复FEC负载ID的FEC修复数据包结构

   更准确地说，修复FEC有效负载ID由源块编号、编码符号ID和源块长度组成。前两个字段的长度取决于m参数（在FFCI第5.1.1.2节中单独传输）：

   -  源块编号 (SBN) ((32-m)-bit field): 此字段标识此FEC修复数据包所属的源块.
   -  编码符号ID (ESI) (m-bit field): 此字段标识此FEC修复数据包中包含的修复符号。对于维修符号，该值为k<=ESI<=n-1.
   - 源块长度 (k) (16-bit field): 源块长度 (k) (16-bit field): 此字段提供此源块的源符号数，即k参数. 如果m<=8时16位太多，而在8<m<=16时需要16位。因此，我们提供了一个单一的通用格式，与m无关.
```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Source Block Number (24 bits)       | Enc. Symb. ID |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Source Block Length (k)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
   图8: m=8的修复FEC负载ID编码格式（默认）。

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Source Block Nb (16 bits)   |   Enc. Symbol ID (16 bits)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Source Block Length (k)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
        图9:m=16的修复FEC负载ID编码格式。

  图8和图9分别说明了m=8和m=16的修复FEC负载ID的格式。

## 5.2. 程序
以下程序适用：
- 源块创建必须遵循第4.3节中规定的程序。
- 对于ADU流的第一个块，SBN值必须以0开始，对于每个新的源块，SBN值必须增加1。在值2^^（32-m）-1之后，长会话将换行为零。
- 编码符号的ESI第一个符号必须以的0开始，并且必须按顺序管理。前k个值（0<=ESI<=k-1）标识源符号，而最后n-k个值（k<=ESI<=n-1）标识修复符号。
- FEC修复包的创建必须遵循第5.1.3节规定的程序。

## 5.3. FEC代码规范
本文件继承了[RFC5510]第8节“擦除信道的里德所罗门码规范”，即基于范德蒙矩阵的核心里德所罗门码规范。

# 6. 安全考虑

FECFRAME文档[RFC6363]提供了适用于FEC方案的安全注意事项的综合分析。因此，本节遵循[RFC6363]的安全注意事项部分，仅讨论特定于使用Reed-Solomon代码的主题。

## 6.1. 对数据流的攻击
### 6.1.1. 访问机密内容
本文件中规定的Reed-Solomon FEC方案不会改变[RFC6363]的建议。总之，如果涉及机密性，建议使用[RFC6363]中提到的解决方案之一，并特别考虑该解决方案应用于操作约束的方式（例如，在FEC保护之前或之后，在终端系统内或在中间盒中应用加密）（例如，在受保护的环境中执行FEC解码可能会很复杂，甚至不可能）并与威胁模型相关联。

### 6.1.2. 内容损坏
本文件中规定的Reed-Solomon FEC方案不会改变[RFC6363]的建议。总之，建议在FEC源和修复数据包上使用[RFC6363]中提到的解决方案之一。

## 6.2. 对FEC参数的攻击
本文档中指定的FEC方案定义了可作为多种攻击基础的参数。更具体地说，攻击者可以修改FFCI的以下参数（第5.1.1.2节）：
- FEC编码ID：改变这个参数导致接收器考虑不同的FEC方案，这使得攻击者能够创建拒绝服务（DoS）。
- 编码符号长度（E）：将此E参数设置为小于有效值的值可使攻击者创建DoS，因为修复符号和某些源符号将大于E，这对接收器来说是不一致的。当S=1时，将此E参数设置为大于有效值的值会产生类似的影响，因为收到的修复符号大小将小于预期值。相反，S=0时不会导致任何不一致，因为块的实际符号长度值由任何接收到的修复符号的大小决定，只要该值小于E。但是，将此E参数设置为较大的值可能会对预先分配内存空间以存储传入符号的接收器产生影响。
- 严格（S）标志：如果实际符号大小在不同的源块上不同，则将此S标志从0翻转到1（即，e现在被视为严格值）会使攻击者误导接收器。将该S标志从1翻转到0不会产生重大后果，除非接收器要求具有固定的E值（例如，因为接收器预先分配内存空间）。
- M参数：改变此参数触发DOS，因为接收方和发送方将考虑不同的代码，并且接收方将不同地解释显式源FEC有效载荷ID和修复FEC有效载荷ID。此外，如果尝试解码，则通过将该m参数增加到更大的值（当两个值在目标用例中都可能时，通常m＝16而不是8），将在接收器处创建额外的处理负载。

因此，建议按照[RFC6363]中的规定，采取安全措施以保证FFCI的完整性。如何实现这一点取决于FFCI从发送方到接收方的通信方式，本文件未对此进行规定。

类似地，针对显式源FEC有效负载ID和修复FEC有效负载ID的攻击也是可能的：通过修改源块编号（SBN）、编码符号ID（ESI）或源块长度（k），攻击者可以轻易损坏SBN标识的块。还可能发生其他依赖于用例和/或CDP的后果。因此，建议采取安全措施，以保证FEC源和修复[RFC6363]中所述的数据包。

## 6.3. 当要同时保护多个源流时
本文件中规定的Reed-Solomon FEC方案不会改变[RFC6363]的建议。

## 6.4. 基线安全帧操作
本文件中规定的Reed-Solomon FEC方案不会改变[RFC6363]关于将IPsec/ESP安全协议作为强制实施（但非强制使用）安全方案的建议。这非常适合于唯一不安全的域是FECFRAME操作的域的情况。

# 7. 业务和管理考虑
FEC框架文件[RFC6363]提供了适用于FEC方案的运营和管理考虑因素的综合分析。因此，本节仅讨论本文件中规定的特定于使用里德-所罗门代码的主题。
## 7.1. 操作建议：有限字段大小（m）
本文件要求有限域中元素的长度m（以位为单位）为2<=m<=16。然而，从实践的角度来看，并非所有的可能性都同样有趣。预计将主要使用默认值m=8，因为它提供了具有中小型源块和/或大量修复符号（即k<n<=255）的可能性。此外，有限域中的元素长度为8位，这使得在编码和解码期间读/写内存操作在字节上对齐。

已知仅使用非常小的源块时的备选方案是m＝4（即，k＜n＜15）。有限域中的元素长度为4位，因此，如果一次访问2个元素，则在编码和解码期间，读/写内存操作将按字节对齐。

当需要非常大的源块时，另一种选择是m=16（即k<n<=65535）。但是，此选择对处理负载有重大影响。例如，使用预先计算的表来加速有限域上的运算（如使用较小的有限域所做的），可能需要在嵌入式平台上使用大量的内存。在处理负载是一个问题且源块长度足够大的情况下，可优选替代的轻型解决方案（例如，LDPC FEC[RFC5170]）。

由于m参数可能有多个值，因此用例应该定义需要支持的一个或多个值。在未指定的情况下，必须使用默认的m=8值。

在任何情况下，任何兼容的实现都必须至少支持默认的m=8值。

# 8. IANA考虑
FEC编码ID的值受IANA注册的约束。[RFC6363]定义了IANA考虑事项的一般指南。特别是，它定义了“可靠多播传输（RMT）FEC编码ID和FEC实例ID”注册表的“FEC框架（FEC框架）FEC编码ID”子区，其注册过程为IETF Review。

本文件在“FEC框架（FEC框架）FEC编码ID”子区域中注册一个值，如下所示：

8指用于任意分组流的GF（2^^m）上的简单Reed-Solomon[RFC5510]FEC方案。

# 9.  Acknowledgments

   The authors want to thank Hitoshi Asaeda and Ali Begen for their
   valuable comments.


# 10.  References

## 10.1.  Normative References

   [RFC2119]      Bradner, S., "Key words for use in RFCs to Indicate
                  Requirement Levels", BCP 14, RFC 2119, March 1997.

   [RFC5052]      Watson, M., Luby, M., and L. Vicisano, "Forward Error
                  Correction (FEC) Building Block", RFC 5052,
                  August 2007.

   [RFC5510]      Lacan, J., Roca, V., Peltotalo, J., and S. Peltotalo,
                  "Reed-Solomon Forward Error Correction (FEC) Schemes",
                  RFC 5510, April 2009.

   [RFC6363]      Watson, M., Begen, A., and V. Roca, "Forward Error
                  Correction (FEC) Framework", RFC 6363, October 2011.

   [RFC6364]      Begen, A., "Session Description Protocol Elements for
                  the Forward Error Correction (FEC) Framework",
                  RFC 6364, October 2011.

## 10.2.  Informative References

   [Matsuzono10]  Matsuzono, K., Detchart, J., Cunche, M., Roca, V., and
                  H. Asaeda, "Performance Analysis of a High-Performance
                  Real-Time Application with Several AL-FEC Schemes",
                  35th Annual IEEE Conference on Local Computer
                  Networks (LCN 2010), October 2010.

   [RFC5053]      Luby, M., Shokrollahi, A., Watson, M., and T.
                  Stockhammer, "Raptor Forward Error Correction Scheme
                  for Object Delivery", RFC 5053, October 2007.

   [RFC5170]      Roca, V., Neumann, C., and D. Furodet, "Low Density
                  Parity Check (LDPC) Staircase and Triangle Forward
                  Error Correction (FEC) Schemes", RFC 5170, June 2008.

   [RFC5740]      Adamson, B., Bormann, C., Handley, M., and J. Macker,
                  "NACK-Oriented Reliable Multicast (NORM) Transport
                  Protocol", RFC 5740, November 2009.

   [RFC5775]      Luby, M., Watson, M., and L. Vicisano, "Asynchronous
                  Layered Coding (ALC) Protocol Instantiation",
                  RFC 5775, April 2010.

   [Rizzo97]      Rizzo, L., "Effective Erasure Codes for Reliable
                  Computer Communication Protocols", ACM SIGCOMM
                  Computer Communication Review, Vol.27, No.2, pp.24-36,
                  April 1997.

   [RS-codec]     Rizzo, L., "Reed-Solomon FEC codec (C language)",
                  original codec: http://info.iet.unipi.it/~luigi/vdm98/
                  vdm980702.tgz, improved codec: http://openfec.org/,
                  July 1998.

Authors' Addresses

   Vincent Roca
   INRIA
   655, av. de l'Europe
   Inovallee; Montbonnot
   ST ISMIER cedex  38334
   France

   EMail: vincent.roca@inria.fr
   URI:   http://planete.inrialpes.fr/people/roca/


   Mathieu Cunche
   INSA-Lyon/INRIA
   Laboratoire CITI
   6 av. des Arts
   Villeurbanne cedex  69621
   France

   EMail: mathieu.cunche@inria.fr
   URI:   http://mathieu.cunche.free.fr/


   Jerome Lacan
   ISAE, Univ. of Toulouse
   10 av. Edouard Belin; BP 54032
   Toulouse cedex 4  31055
   France

   EMail: jerome.lacan@isae.fr
   URI:   http://personnel.isae.fr/jerome-lacan/

   Amine Bouabdallah
   CDTA
   Center for Development of Advanced Technologies
   Cite 20 aout 1956, Baba Hassen
   Algiers
   Algeria

   EMail: abouabdallah@cdta.dz


   Kazuhisa Matsuzono
   Keio University
   Graduate School of Media and Governance
   5322 Endo
   Fujisawa, Kanagawa  252-8520
   Japan

   EMail: kazuhisa@sfc.wide.ad.jp

