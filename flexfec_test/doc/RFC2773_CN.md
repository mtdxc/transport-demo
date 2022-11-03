<h1>前向纠错码(FEC)的RTP荷载格式</h1>

# 本备忘录状态

本文档讲述了一种Internet通信的标准Internet跟踪协议,并对其改进提出了讨论和建议。请参考最新版本的"Internet Official Protocol Standards"(STD1)来获得本协议的标准化进程和状态，此备忘录的发布不受任何限制。

#  摘要
本文档规定了一般性的前向纠错的媒体数据流的RTP打包格式。这种格式针对基于异或操作的FEC算法进行了特殊设计，它允许终端系统使用任意长度的纠错码，并且可以同时恢复出荷载数据和RTP头中的关键数据。由于FEC作为一个分离的数据流进行传送，这种方案可以向后兼容那些没有实现FEC解码器的接收终端。对于那样的终端来说，可简单地将FEC数据丢掉。

# 1 简介

在Internet上用分组传送话音，质量不好的一个重要原因是较高的丢包率。尤其在广域网中，这个问题相当突出。不幸的是，实时多媒体业务对于时延要求相当严格，因此不大可能通过重传来解决丢包问题。正是出于这个原因，大家提出用前向纠错（FEC）来解决Internet上的丢包问题[1] [2]。尤其是对于传统纠错码如校验码、RS码、汉明码等的使用引起了很多人的注意。为了能够更好地应用这些纠错码，必须有相关的协议来支持。

本文档定义了一种RTP的荷载格式，允许对于实时媒体流进行一般性的前向纠错。在这里“一般性”指的是
- 与被保护的媒体类型无关，即音频、视频或其它；
- 足够灵活，能够支持多种FEC机制；
- 自适应性，可以方便的修改FEC方案而不需要带外的信令支持；
- 支持若干种不同的FEC包的传输机制。

# 2 术语
本文档中使用了下面这些术语：

- 媒体荷载：一段待传输的未加保护的用户数据。媒体荷载放在一个RTP包的内部。
- 媒体头：包含媒体荷载的包的RTP头
- 媒体包：媒体荷载与媒体头合起来称作媒体包
- FEC包：发送端将媒体包作为前向纠错算法的输入，输出除了这些媒体包之外，还有一些新的数据包称作FEC包。FEC包的格式在本文档中进行说明。
- FEC头：FEC包的头信息称作FEC头。
- FEC荷载：FEC荷载是FEC包中的荷载。
- 关联的：如果在这个FEC包的产生过程中，这几个媒体包用作FEC算法的输入，则称这几个包是关联的

关键词“必须”，“必须不”，“要求的”，“会“，”不会“，“应该”，“不应该”，“建议的”，“或许”，“可选的”在RFC2119[4]中解释。

# 3 基本操作

这里描述的荷载格式用于一个RTP会话中的某一端想要用FEC来保护它所传送的媒体数据流的情况。这种格式所支持的FEC是基于简单异或校验的纠错算法。 发送端从媒体数据流中取出若干个包，并对它们整个施以异或操作，包括RTP头。基于这样一个过程，可以得到一个包含FEC信息的RTP包。这个包可以被接收端用来恢复任何一个用来产生它的包。本文档中并未规定多少个媒体包合起来产生一个FEC包。不同参数的选取会导致在负载: overhead，延时和恢复能力之间的一个不同的折中方案。第4节给出了一些可能的组合。

发送端需要告诉接收端哪些媒体包被用来产生了一个FEC包，这些信息都包含在荷载信息中。每个FEC包中包含一个24比特的mask，如果mask的第i个比特为1，序号为N＋i的媒体包就参与了这个FEC包的生成。N称作基序号，也在FEC包中传送。通过这样一种方案就可以以相当小的overhead，来封装任意的FEC纠错方案恢复丢失的数据包。

本文档也描述了如何使接收端在不了解具体纠错码细节的情况下使用FEC的方法。这就给了发送端更大的灵活性，它可以根据网络状态而自适应选择纠错码，而接收端仍能够正确解码并恢复丢失的包。

发送端生成FEC包之后，就把它们发给接收端，同时，发送端也照常发送原来的媒体数据包，就好像没有FEC一样。这样对于没有FEC解码能力的接收端，媒体流也照常可以接收并解码。然而，对于某些纠错码来说，原始的媒体数据包是不需要传输的，仅靠FEC包就足以恢复丢失的包了。这类码就具有一个很大的缺点，就是要求所有的接收者都具有FEC解码能力。这类码在本文档中也是支持的。

FEC包与媒体包并不在同一个RTP流中传输，而是在一个独立的流中传输，或作为冗余编码(redundant encoding）中的次编码 （secondary encoding）来传输[5]。当在另一个流中传输时，FEC包有自己的序号空间。FEC包的时间戳是从对应的媒体包中得来的，同样是单调递增。因此，这样的FEC包可以很好地应用于任何具有固定差值的包头压缩方案。本文并没有规定何为“一个独立的流”，而把它留给上层协议和具体应用去定义。对于多播，“一个独立的流”可通过不同的多播组来实现，或是同一个组的不同端口，或者同样的组和端口不同的SSRC。对于单播的情况，可使用不同的端口或者不同的SSRC。

这些方法都各有其优缺点，选用哪一种取决于具体的应用。接收端收到FEC包和媒体包之后，先判断是否有媒体包丢失。如果没有，FEC包就直接被丢弃。如有丢包，就使用接收到的FEC包和媒体包来进行丢包的重建。这样一个重建过程是很精确的，荷载以及包头的大部分数据都可以完全恢复出来。

按照本协议来进行打包的RTP包，可以使用一个动态RTP荷载类型号来通知接收端。

# 4 监督码

我们定义f(x,y,..)为数据包x,y,…等的异或，这个函数的输出也是一个数据包，称作监督包。为简单起见，我们假定监督包就是输入的各个包的对应比特异或得到。详细的过程描述在第6节中给出。

用监督码来恢复数据包是通过对一组数据包生成一个或多个监督包来完成的。为了提高编码的效率，这些监督包必须是数据包的线性无关的组合生成的。某一个特定的组合就称为一个监督码。对于k个一组的数据包，生成n-k个监督包，这样的监督码认为是同一类监督码。对于给定的n,k，可能的监督码有很多。荷载格式并未要求使用某个特定的监督码。

举个例子，考虑这样一种监督码，输入为两个数据包，输出为1一个监督包。如果原始媒体数据包是a,b,c,d，发送端送出的包如下所示：
```
   a        b        c        d               <-- media stream
              f(a,b)            f(c,d)        <-- FEC stream
```
时间从左向右递增。在这个例子里，纠错码带来了50%的overhead。但是如果b丢失了，用a和f(a,b)就可以恢复出b。
下面还给出了一些其它的监督码。其中每一个的原始媒体数据流都包含数据包a,b,c,d等等。

方案1
--------
这个方案与上面的例子很类似。区别在于，并不是发送b之后再发送f(a,b)。f(a,b)在b之前就发送出去了。显然这样做会给发送端带来额外的延时，但是用这种方案能够恢复出突发的连续两个丢包。发送端送出的包如下所示：
```
   a        b        c        d        e        <-- media stream
     f(a,b)   f(b,c)   f(c,d)   f(d,e)          <-- FEC stream
```

方案2
--------
事实上，并没有严格规定一定要传送原始媒体数据流。在这个方案中，只传送FEC数据包。这种方案能够恢复出所有单个的丢包和某些连续的丢包，而且overhead比方案1略小一些。发送端送出的数据包如下所示：
```
 f(a,b)  f(a,c)  f(a,b,c)  f(c,d)  f(c,e)  f(c,d,e)  <-- FEC stream
```

方案3
--------
这种方案要求接收端在恢复原始媒体数据包时等待4个数据包间隔的时间，也就是在接收端引入了4个数据包的延时。它的优越性在于它能够恢复出单个丢包，连续两个丢包乃至连续三个丢包。发送端送出的数据包如下所示：
```
   a         b          c                    d     <-- media stream
               f(a,b,c)    f(a,c,d) f(a,b,d)       <-- FEC stream
```

# 5 RTP媒体数据包结构
媒体数据包的结构并不受FEC的影响。
- 如果FEC包作为一个独立的流进行传输，媒体数据包的传输就与不使用FEC时完全没有区别。
- 如果FEC作为一个冗余编码（redundant codec）来进行传输，媒体数据包就作为RFC2198[5]中定义的主编码（primary codec）。
这样一种编码方式是非常高效的。如果只使用了极少量的FEC，大部分数据包都是媒体数据包，这意味着overhead代表着FEC的冗余数据量。

# 6 FEC包结构
一个FEC包就是把一个FEC包头和FEC包的荷载放进RTP包的荷载中组成的，如图1所示。
```
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         RTP Header                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         FEC Header                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         FEC Payload                           |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
   Figure 1: FEC Packet Structure

## 6.1 FEC包的RTP包头

在FEC包的RTP包头中：
- ver设定为2
- 填充位通过保护操作计算得到，具体方法在后面给出。
- 扩展位也是通过保护操作计算得到的。
- 一般情况下，SSRC 的值应当与它所保护的媒体数据包的SSRC值一致。如果FEC流通过SSRC值来进行解复用的话，SSRC的值就有可能会不同。
- CC的值也是在保护操作中 计算出来。不管CC域为何值，CSRC列表永远不存在。
- 不管X比特为何值，头扩展部分永远不存在。
- 标记位的值通过保护操作计算得出。
- 对于系列号有一个标准的定义：当前包的次序号必须比前一个包的次序号大1。
- 时间戳必须设定为当前这个FEC包发送时对应的媒体流的RTP时钟的值。不管使用何种FEC方案，FEC包头中的TS值是单调递增的。
- FEC包的荷载类型是动态确定的，即在带外通过信令来协商确定的。

按照RFC1889[3]，RTP会话的某一方如果不能识别收到的RTP包的荷载类型的话，就必须将其丢弃。这样就很自然地提供了向后兼容的能力。FEC机制可以用在一个多播的环境中，某些接收端具有FEC解码能力，而某些不具有。

## 6.2 FEC头
FEC头的长度为12个字节，其格式如图2所示。它包含一个SN基数字段，长度恢复，E，PT恢复，mask以及TS恢复字段。
```
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      SN base                  |        length recovery        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |E| PT recovery |                 mask                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                          TS recovery                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
   Figure 2: Parity Header Format

- 长度恢复字段用来确定待恢复的数据包的长度。它的值是当前组中被保护的媒体数据包的长度（以字节为单位）的二进制和（逐位异或），用一个网络序的16比特无符号整数表示。在这里，媒体数据包的长度包括CSRC列表、扩展部分和填充的比特。这样即使在媒体数据包长度不一致的情况下，也一样可以使用FEC。举个例子，假定要用两个媒体数据包的异或来产生一个FEC包，这两个媒体数据包的长度分别为3（0b011）和5（0b101）个字节，那么长度恢复字段的值就 是0b011 xor 0b101 = 0b110。
- 比特E指示是否存在一个扩展部分。在当前版本中，这个比特必须设置为0。
- PT recovery：是通过对相关的媒体数据包的荷载类型的值进行异或操作得到的，从而能够用来恢复丢失包的荷载类型。
- Mask域长度为24个比特，如果其中的第i个比特设置为1，那么序号为N+i的媒体数据包就与当前FEC包相关联。其中N是SN Base的值。最低位（LSB）对应于i＝0，最高位（MSB）对应于i＝23。
- SN Base的值必须设置为与当前FEC包相关的媒体数据包中的最小的序号。这样一个FEC包最多可以与连续24个媒体数据包相关联。
- TS恢复域是通过计算相关联的媒体数据包的时间戳的异或得到的。这样就可以恢复出丢掉的数据包的时间戳。
- FEC Payload 是对相关联的媒体数据包的CSRC列表、RTP头部扩展、媒体包荷载以及填充比特连在一起进行异或操作得到的。

值得注意的是FEC包的长度有可能会比它保护的媒体数据包的长度大一点，因为多了一个FEC头。如果FEC包的长度超出了下层协议允许的最大包长，这可能会给传输带来很大的问题。

# 7 保护操作
保护操作涉及到将RTP头中的某些字段与媒体包的荷载级联起来，再加上填充比特，然后对这些序列计算它们的异或值。得到的比特序列就成为FEC包的某个组成部分。对于每一个要保护的媒体包，按照下面的次序将各个数据域级联起来生成一个比特序列，如果中间还插入了其它操作的话，最终结果必须与下面所描述的一致：
- 填充位（1比特）
- 扩展位（1比特）
- CC（4比特）
- 标记位（1比特）
- 荷载类型（7比特）
- 时间戳（32比特）
- 长度恢复域的值（网络次序16比特无符号整数），也就是各个媒体数据包的荷载长度、CSRC列表长度以及填充比特长度的和异或的结果
- 如果CC不为零，这里是CSRC列表（变长）
- 如果扩展位为1，这里是头部扩展（变长）
- 荷载（变长）
- 填充，如果存在的话（变长）

需要注意的是最前面的填充位是整个比特序列的最重要比特（MSB）。

如果各个媒体数据包的到的比特序列长度不相等，那么每一个都必须填充到最长的序列那么长。填充值可以是任意的，但必须填充在整个比特序列的最后。对所有这 些比特序列进行对位异或操作，就可以得到一个监督比特序列，这个监督比特序列就可以用来生成FEC包。称这个比特序列为FEC比特序列。FEC比特序列中 的第一个比特填入FEC包的填充位，第二个比特填入FEC包的扩展位，接下来的四个比特填入FEC包的CC域，再下来的一个比特填入FEC包的标记位，然 后的七个比特写入FEC包头的PT恢复域，再后面的32个比特写入FEC包头的TS恢复域，再接下来的16个比特写入FEC包头中的长度恢复域。最后剩下 的比特就作为FEC包的荷载。

# 8 恢复过程
FEC包能够使终端系统有能力恢复出丢失的媒体数据包。丢失包的的包头中的所有数据域，包括CSRC列表，扩展位，填充位，标记位以及荷载类型，都是可以恢复的。这一节描述进行恢复的过程。

恢复过程中包含两个不同的操作。第一个是确定需要用哪些包（包括媒体包和FEC包）来恢复一个丢失的包。这一步完成之后，第二步就是重建丢失的包。第二步 必须按照下面的规定来进行。第一步可以由实现者选择任意的算法来完成。不同的算法会导致在复杂度和恢复丢包能力之间的一个不同的折中。

## 8.1 重建
设T为可用来恢复媒体数据包xi的一组包（包括FEC包和媒体包），重建过程如下所述：
1. 对于T中的媒体数据包，按照上一节保护操作中所规定的那样计算它们的比特序列。
2. 对于T中的FEC包，基本以同样的方式来计算比特序列，不同点仅在于用PT恢复域的值取代荷载类型，用TS恢复域的值取代时间戳，并将CSRC列表、扩展位和填充位都设为null。
3. 如果某个媒体数据包生成的比特序列比FEC包生成的比特序列短，就把它填充到域FEC包生成的比特序列一样长度。填充部分必须加在比特序列的最后，可以为任意值。
4. 对这些比特序列进行按位异或操作，得到一个恢复出的比特序列。
5. 创建一个新的数据包，12个字节的标准RTP头，没有荷载。
6. 将这个新包的版本域设为2。
7. 将新包的填充位设为恢复出的比特序列的第一个比特。
8. 将新包的扩展位设为恢复出的比特序列的第二个比特。
9. 将新包的CC域设为恢复出的比特序列的接下来的4个比特。
10. 将新包的标记为设为恢复出的比特序列的接下来的一个比特。
11. 将新包的荷载类型设为恢复出的比特序列的接下来的7个比特。
12. 将新包的SN域设定为xi。
13. 将新包的TS域的值设定为恢复出的比特序列的接下来的32个比特。
14. 从恢复出的比特序列中取出接下来的16个比特，将其作为一个网络序的无符号整数，然后从恢复出的比特序列中取出这个整数那么多的字节，添加在新包之后，这代表新包的CSRC列表、扩展、荷载和填充。
15. 将新包的SSRC域设定为它所保护的媒体流的SSRC值。
    上面的这个过程能够完全恢复出一个丢失的RTP包的包头和荷载。

## 8.2 何时进行恢复
前面一节讨论了当要恢复一个序号为xi的包时，所有需要的包都可用时，如何来进行恢复。而并未涉及如何决定是否去试图恢复某个包xi，以及如何确定是否有 足够的数据来恢复这个包。这些问题将留给实现者去灵活设计，但我们将在本节给出一个可用于解决这些问题的简单算法。
下面的这个算法是用C语言写的。代码中假定已经存在了几个函数。recover_packet()的参数是一个包的序号和一个FEC包。用这个FEC包和 以前收到的数据包，这个函数能够恢复出指定序号的数据包。add_fec_to_pending_list()将一个给定的FEC包加入到一个存放尚未用 于恢复操作的FEC包的链表中去。wait_for_packet()等待从网络上发来的一个包，

FEC包或者是数据包。remove_from_pending_list()将一个FEC包从链表中删除。结构体packet包含一个布尔变量 fec，当这个包为FEC包时fec为真，否则为假。当它是一个FEC包时，成员变量mask和snbase存放着FEC包头中对应值；当它是一个媒体数 据包时，sn变量存放着包的序号。全局数组A用于指示出哪些媒体数据包已经收到了，而哪些还没有。它是以包的序号为索引的。

函数fec_recovery给出了这个算法的实现。它等待网络上发来的数据包，当它收到一个FEC包时，就调用recover_with_fec()， 即尝试用它来恢复。如果恢复操作不可能（数据不足），就把这个FEC包存起来备用。如果收到的包是一个媒体数据包，就记录下它已经收到了，然后检查以前的 FEC包是否现在可以进行恢复了。我们对待恢复出来的包就象从网络上收到这个包一样，并会引发进一步的恢复尝试。

一个实际的实现需要用一个循环缓冲区来代替数组A，以避免数组缓冲区溢出。并且，下面的代码中并没有释放已经没有任何用处的FEC包。一般来说，对FEC 包的释放操作可以基于一个时间限制（playtime），这个时间限制取决于发送端以多少个包为一组进行保护操作。当一个FEC包所保护的数据包的 playtime已经过去的时候，这个FEC包就不再有用了。
```c++
typedef struct packet_s {
  BOOLEAN fec;               /* FEC or media */
  int sn;                    /* SN of the packet, for media only */
  BOOLEAN mask[24];          /* Mask, FEC only */
  int snbase;                /* SN Base, FEC only */
  struct packet_s *next;
} packet;



BOOLEAN A[65535];
packet *pending_list;

packet *recover_with_fec(packet *fec_pkt) {
  packet *data_pkt;
  int pkts_present,  /* number of packets from the mask that are present */
    pkts_needed,    /* number of packets needed is the number of ones
                        in the mask minus 1 */
    pkt_to_recover, /* sn of the packet we are recovering */
    i;

  pkts_present = 0;

  /* The number of packets needed is the number of ones in the mask
     minus 1.  The code below increments pkts_needed by the number
     of ones in the mask, so we initialize this to -1 so that the
     final count is correct */
  pkts_needed = -1;

  /* Go through all 24 bits in the mask, and check if we have
     all but one of the media packets */
  for(i = 0; i < 24; i++) {

     /* If the packet is here and in the mask, increment counter */
     if(A[i+fec_pkt->snbase] && fec_pkt->mask[i]) pkts_present++;

     /* Count the number of packets needed as well */
     if(fec_pkt->mask[i]) pkts_needed++;

     /* The packet to recover is the one with a bit in the
        mask that's not here yet */
     if(!A[i+fec_pkt->snbase] && fec_pkt->mask[i])
       pkt_to_recover = i+fec_pkt->snbase;
   }

   /* If we can recover, do so. Otherwise, return NULL */

   if(pkts_present == pkts_needed) {
     data_pkt = recover_packet(pkt_to_recover, fec_pkt);
   }  else {
     data_pkt = NULL;
   }

   return(data_pkt);
 }


 void fec_recovery() {

   packet *p,    /* packet received or regenerated */
       *fecp,    /* fec packet from pending list */
       *pnew;    /* new packets recovered */

   while(1) {
     p = wait_for_packet();    /* get packet from network */
     while(p) {

       /* if it's an FEC packet, try to recover with it. If we can't,
          store it for later potential use. If we can recover, act as
          if the recovered packet is received and try to recover some
          more.  Otherwise, if it's a data packet, mark it as received,
          and check if we can now recover a data packet with the list
          of pending FEC packets */

       if(p->fec == TRUE) {
          pnew = recover_with_fec(p);

          if(pnew)
            A[pnew->sn] = TRUE;
          else
            add_fec_to_pending_list(p);

          /* We assign pnew to p since the while loop will continue
             to recover based on p not being NULL */
          p = pnew;
       } else {

         /* Mark this data packet as here */
         A[p->sn] = TRUE;

         free(p);
         p = NULL;

         /* Go through pending list. Try and recover a packet using
            each FEC. If we are successful, add the data packet to
            the list of received packets, remove the FEC packet from
            the pending list, since we've used it, and then try to
            recover some more */
         for(fecp = pending_list; fecp != NULL; fecp = fecp->next) {
           pnew = recover_with_fec(fecp);
           if(pnew) {
             /* The packet is now here, as we've recovered it */
             A[pnew->sn] = TRUE;

             /* One FEC packet can only be used once to recover,
                so remove it from the pending list */
             remove_fec_from_pending_list(fecp);

             p = pnew;
             break;
           }
         } /*for*/
       } /*p->fec was false */
     } /* while p*/
   } /* while 1 */

 }
```
# 9 例子

考虑这样的情况，有两个媒体数据包x和y要从SSRC2发送出去。它们的序号分别为8和9，时间戳分别为3和5。x的荷载类型为11，y的荷载类型为18。x有十个字节的荷载，y有11个字节的荷载。y的标记位为1。x和y的RTP包头分别如图3和图4所示。

媒体数据包x
```
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |1 0|0|0|0 0 0 0|0|0 0 0 1 0 1 1|0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

      Version:   2
      Padding:   0
      Extension: 0
      Marker:    0
      PTI:       11
      SN:        8
      TS:        3
      SSRC:      2
```
      图3:媒体包x的RTP头

媒体包y
```
 0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |1 0|0|0|0 0 0 0|1|0 0 1 0 0 1 0|0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

      Version:   2
      Padding:   0
      Extension: 0
      Marker:    1
      PTI:       18
      SN:        9
      TS:        5
      SSRC:      2
```
图4:媒体包y的RTP头

    由这两个包生成一个FEC包。我们假定用荷载类型值127来指示一个FEC包。得到的FEC包的RTP包头如图5所示。
```
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |1 0|0|0|0 0 0 0|1|1 1 1 1 1 1 1|0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

      Version:   2
      Padding:   0
      Extension: 0
      Marker:    1
      PTI:       127
      SN:        1
      TS:        5
      SSRC:      2
```
图5:x,y的FEC包的RTP头

FEC包的FEC头如图6所示。
```
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0|0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0|0 0 1 1 0 0 1|0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

      SN base:   8    [min(8,9)]
      len. rec.: 1    [8 xor 9]
      E:         0
      PTI rec.:  25   [11 xor 18]
      mask:      3
      TS rec.:   6    [3 xor 5]

      The payload length is 11 bytes.
```
图6:FEC包的FEC头

# 10 冗余编码中使用FEC的用法
我们可以把一个FEC包看成是对媒体的一个冗余编码（redundantencoding）。这样一来，[5]中所描述的冗余音频数据的荷载格式就可以用 于FEC数据的打包。这个过程如下所述：上面的FEC操作作用于一个RTP媒体数据包组成的流上。这个流也就是RFC2198[5]中进行封装之前的流。 换句话说，将待保护的媒体数据流封装在标准的RTP媒体包中，然后进行前面定义过的FEC操作（有一点小的改动），生成一个FEC数据包的流。这里与前面 不同的一点在于：在进行FEC操作之前，必须把待保护的RTP媒体数据包的RTP头扩展、填充部分以及CSRC列表去掉，并且CC域、填充位、扩展位必须 设置为零，用这些修改之后的RTP媒体数据包去生成FEC包。需要注意的是发送端要发送出去的仍然是原来没有修改的媒体数据包，只是在计算FEC包的时候 才去除掉这些域。

一旦生成了FEC包，将媒体数据包中的荷载提取出来，作为RFC2198中定义的主编码(primary encoding），将FEC包中的FEC头和荷 载提取出来，作为RFC2198中定义的冗余编码（redundant encoding）。除了FEC之外，还可以向包中加入额外的冗余编码，但这些信息 不受FEC的保护。

主编码（primary codec）的冗余编码头（redundant encodings header）按照RFC2198中的定义进行设置。下面给出 FEC数据的冗余编码头的设置。块荷载类型域（block PT）设置为与FEC格式相关联的动态PT值，块长度设置为FEC头和FEC荷载的长度之和。时 间戳偏移（timestamp offset）应当设置为0。次编码（secondary codec）的荷载包括FEC头和FEC荷载。

在接收端，主编码和所有的次编码都作为不同的RTP包提取出来。这是通过把冗余编码包中的RTP头的次序号、SSRC、标记位、CC域、RTP版本号和扩 展位等拷贝到每一个提取出的包的RTP头中去。如果次编码中包含FEC，FEC包的RTP头中的CC域、扩展位、填充位都必须设置为零。提取出的包的荷载 类型码是从冗余编码头中的块荷载类型域复制过来。提取出的包的时间戳是RTP头中的时间戳与块头中的时间戳偏移之差。提取包的荷载就是块中的数据部分。这 样一来，FEC流和媒体数据流就被分别提取出来了。

要利用FEC包和已接收到的媒体数据包来恢复丢失的包，必须把媒体数据包中的CSRC列表、扩展头、填充部分去掉，并把CC域，扩展位、填充位都设置为 零。用这些修改之后的媒体包，加上FEC包，基于第8节中的步骤，就可以恢复出丢失的包。恢复出的包将肯定没有扩展部分、填充部分和CSRC列表部分。在 具体实现中，如果其它包中存在这些部分，可以从其它包中将这些部分拷贝过来。

使用冗余编码的荷载格式，有可能不能正确恢复出标记位。在使用RFC2198来进行FEC封装的应用程序中，必须把恢复出的媒体数据包的标记位设置为零。相对于发送完整的FEC包，这种方法的优点在于它能够减少overhead。

# 11 在SDP中表示FEC
FEC包的RTP荷载类型值是一个动态类型值。FEC包可以发到与媒体包不同的多播组或者不同的端口。如果使用[5]中定义的冗余编码荷载格式，FEC数 据甚至可以放在媒体包中一起传输。这些配置选项必须在带外明确指示出来。这一节讲述如何用RFC2327[6]中定义的会话描述协议 (Session Description Protocol,SDP）来完成这一工作。

## 11.1 FEC作为独立的流传输
在第一种情况中，FEC包是作为一个独立的流来进行传输的。这可能意味着它们被发往与媒体包不同的端口或不同的多播组。这时，必须传达以下几条消息：
- FEC包被发往的地址和端口
- FEC包的荷载类型号
- 它保护的是哪个媒体数据流

FEC的荷载类型号是在它所保护的媒体的m行（译者注：媒体描述行，mediadescription line，见RFC2327）中传送的。由于没有为FEC分配静态的荷载类型值，因此必须使用动态的荷载类型值。这个值的绑定是通过一个rtpmap属性 来指示的，绑定中所使用的名称为"parity fec"。
FEC的荷载类型号出现在它所保护的媒体的m行中并不意味着FEC包要发送到相同的地址和端口。事实上，FEC包的端口和地址信息是通过fmtp属性行来传递的。在媒体的m行中出现FEC荷载类型指示为了指出FEC保护的是哪个媒体流。
FEC的fmtp行的格式如下所示：
a=fmtp:<荷载类型号><端口><网络类型><地址类型><连接地址>
其中“荷载类型号”就是在m行中出现的荷载类型号。“端口”是FEC包将要发送的端口。其余的三项，网络类型，地址类型和连接地址与SDP的c行中的语法 语义是相同的。这样fmtp行可以部分地用与c行相同的解析器来进行解析。需要注意的是由于FEC不能够分级编码，<地址数量 （numberofaddresses）>参数必须不出现在连接地址中。
下面是一个FEC包的SDP例子：
```
   v=0
   o=hamming 2890844526 2890842807 IN IP4 126.16.64.4
   s=FEC Seminar
   c=IN IP4 224.2.17.12/127
   t=0 0
   m=audio 49170 RTP/AVP 0 78
   a=rtpmap:78 parityfec/8000
   a=fmtp:78 49172 IN IP4 224.2.17.12/127
   m=video 51372 RTP/AVP 31 79
   a=rtpmap:79 parityfec/8000
   a=fmtp:79 51372 IN IP4 224.2.17.13/127
```
在上面的SDP描述中出现两个m行是因为其中存在两个媒体流，一个音频流和一个视频流。媒体格式为0代表用PCM编码的音频，它被荷载类型号为78的 FEC流保护。FEC流被发往与音频相同的多播组，TTL参数也相同，但端口号大2（49172）。视频流被荷载类型号为79的FEC流保护，这个FEC 流的端口号是一样的，但是多播地址不一样。

## 11.2 在冗余编码中使用FEC
当FEC流以冗余编码格式作为一个次编码来发送的时候，必须通过SDP通知对方。为了做到这一点，使用RFC2198中定义的步骤来通知对方使用了冗余编 码。FEC的荷载类型就象其它任意一个次编码那样表示出来。必须用一个rtpmap属性行来指示出FEC包的动态荷载类型号。FEC必须只保护主编码。这 时，FEC的fmtp属性必须不出现。
举个例子:
```
   m=audio 12345 RTP/AVP 121 0 5 100
   a=rtpmap:121 red/8000/1
   a=rtpmap:100 parityfec/8000
   a=fmtp:121 0/5/100
```
这个SDP例子指示有一个单一的音频流，由PCM格式（媒体格式0）和DVI格式（媒体格式5）组成，一个冗余编码（用媒体格式121表示，在 rtpmap属性中绑定为red），以及一个FEC（媒体格式100，在rtpmap属性中绑定为parityfec）。尽管FEC格式是作为媒体流的一 个可能编码来描述的，但它必须不单独传送。它出现在m行中只是因为按照RFC2198，非主编码（non-primarycodec）都必须在这里列出 来。fmtp属性指出冗余编码格式可以这样使用：DVI作为一个次编码（secondarycoding），而FEC作为第三编码 （tertiaryencoding）。

## 11.3 在RTSP中的用法
RTSP[7]可以用来请求将FEC包作为一个独立的流进行传输。当在RTSP中使用SDP时，每个流的会话描述中并不包含连接地址和端口号。作为代替，RTSP使用了控制URL（Control URL）的概念。SDP中以两种不同的方式来使用控制URL。
1. 所有的流使用一个控制URL。这被称为“集中控制”。在这种情况下，FEC流的fmtp行被省略了。
2. 每一个流都有一个控制URL。这被称为“非集中控制”。在这种情况下，FEC流的fmtp行指定它的控制URL。这个URL可以被用在RTSP客户端的SETUP命令中。
非集中控制的FEC流用RTSP时，它的fmtp行的格式为：
a=fmtp:<荷载类型号><控制URL>
    其中荷载类型号就是出现在m行中的荷载类型号。控制URL就是用于控制这个FEC流的URL。
    需要注意的是控制URL并不一定是一个绝对URL。将一个相对的控制URL转化为一个绝对的控制URL的规则在RFC2326的C.1.1中给出。

# 12 安全性问题
使用FEC对于加密时密钥的用法和更改有一定的影响。由于FEC包组成了一个独立的流，在加密的用法上可以有几种不同的排列组合：
- 可以对FEC流加密，而不对媒体流加密
- 可以对媒体流加密，而不对FEC流加密
- 可以对媒体流和FEC流同时加密，但使用不同的密钥
- 可以对媒体流和FEC流同时加密，但使用相同的密钥

前面三种要求应用层的信令协议知道使用了FEC，并因此对媒体流和FEC流分别进行用法协商，并分别交换密钥。在最后一种情况里，并不需要有这些机制，只 要想对待媒体包一样去对待FEC包就可以了。前面两种情况可能会引起分层冲突，因为实际上不应该将FEC包与其它媒体包区别对待，并且只对其中一个流加密 会带来明文攻击的危险。出于这些考虑，使用了加密的应用就应当对两个流都进行加密。
然而，在密钥的变更中会出现一些问题。例如，如果发送了两个数据包a和b，还有一个FEC包f(a,b)。如果a和b所使用的密钥是不同的，那么应该用哪一个密钥来解密f(a,b)？

一般说来，用过的密钥都会被缓存起来，这样当媒体流的密钥更改之后，旧的密钥被保留下来备用，直到它发现FEC包的密钥也发生更改了。

使用FEC的另一个问题是它对于网络拥塞的影响。面对网络丢包越来越多的情况加入FEC不是一个好办法，它可能会导致拥塞更加严重并最终崩溃。因此，实现者在网络丢包增加的情况下必须不大量增加FEC冗余数据，以保证整个网络的性能。

# 13 致谢
这份方案是基于Budge和Mackenzie于1997年提交的一个早期FEC草案，在此表示感谢。我们感谢 SteveCasner,MarkHandley,OrionHodson和ColinPerkins对我们工作所提出的批评和建议，并感谢 AndersKlemets写了“在RTSP中的用法”这一节。

# 14 作者地址
   Jonathan Rosenberg
   dynamicsoft
   200 Executive Drive
   Suite 120
   West Orange, NJ 07046

   Email: jdrosen@dynamicsoft.com


   Henning Schulzrinne
   Columbia University
   M/S 0401, 1214 Amsterdam Ave.
   New York, NY 10027-7003

   EMail: schulzrinne@cs.columbia.edu

# 15 参考书目
   [1] J.C. Bolot and A. V. Garcia, "Control mechanisms for packet audio
       in the internet," in Proceedings of the Conference on Computer
       Communications (IEEE Infocom) , (San Francisco, California), Mar.
       1996.

   [2] Perkins, C. and O. Hodson, "Options for Repair of Streaming
       media", RFC 2354, June 1998.

   [3] Schulzrinne, H., Casner, S., Frederick, R. and V. Jacobson, "RTP:
       A Transport Protocol for Real-Time Applications", RFC 1889,
       January 1996.

   [4] Bradner, S., "Key words for use in RFCs to indicate requirement
       levels", BCP 14, RFC 2119, March 1997.

   [5] Perkins, C., Kouvelas, I., Hodson, O., Hardman, V., Handley, M.,
       Bolot, J.C., Vega-Garcia, A. and S. Fosse-Parisis, "RTP Payload
       for Redundant Audio Data", RFC 2198, September 1997.

   [6] Handley, M. and V. Jacobson, "SDP: Session Description Protocol",
       RFC 2327, April 1998.

   [7] Schulzrinne, H., Rao, A. and R. Lanphier, "Real Time Streaming
       Protocol (RTSP)", RFC 2326, April 1998.

# 16 版权声明
版权归Internet协会所有（1999）。保留所有权利。
本文及其译本可以提供给其他任何人，可以准备继续进行注释，可以继续拷贝、出版、发布，无论是全部还是部分，没有任何形式的限制，不过要在所有这样的拷贝 和后续工作中提供上述声明和本段文字。无论如何，本文档本身不可以做任何的修改，比如删除版权声明或是关于Internet协会、其他的Internet 组织的参考资料等。除了是为了开发Internet标准的需要，或是需要把它翻译成除英语外的其他语言的时候，在这种情况下，在Internet标准程序 中的版权定义必须被附加其中。
上面提到的有限授权允许永远不会被Internet协会或它的继承者或它的下属机构废除。
本文档和包含在其中的信息以"Asis"提供给读者，Internet社区和Internet工程任务组不做任何担保、解释和暗示，包括该信息使用不破坏任何权利或者任何可商用性担保或特定目的。