"""
CLIENT

在这个编程作业中，你将用 Python 编写一个客户 ping 湿序 【 该客户将发送一个简单的 pmg 报文，接
收一个从服务器返间的对应 pong 报文，并确定从该客户发送 ping 报文到接收到 pong 报文为止的时延 。
该时延称为往返时延 (RTI) Q Fh该客户和服务器提供的功能类似于在现代操作系统中可用的标准 pLng 程
序 然而，标准的 ping 使用互联网控制报文协议 (IC!l1P) (我们将在第 4 ì言巾学习 ICMP) 。 此时我们将
创建一个非标准(但简单)的基于 UDP 的 ping 瞿序 。
你的 pi吨程序经 UDP 向日标服务然发送 10 个 ping 报文 。 对于每个报文，当对应的 [>ong 报文逅同
时，你的客户要确定和打印 Rγr。 例为 UDP 是一个不可靠的协议，巾客户发送的分组可能会丢失 。 为
此.客户不能无限期地等待对 ping 报文的回答。 客户等待服务器 InJ 答:的时间至多为 l 秒;如果没有收到
问答，客户假定该分组丢失Jt相应地打印一条报文 。
"""

from socket import *
import time

#serverName = 'doudou.ac.cn'
serverName = '18.222.252.3'
serverPort = 9090
clientSocket = socket(AF_INET, SOCK_DGRAM)
clientSocket.settimeout(5)   #1s timeout
while True:
    choose = input('Input order: ping means run, end means exit, other dump')
    if choose == 'ping':
        for i in range(10):  #send 10 messages in sequence
            message = ('times:%s' % str(i))
            message = bytes(message, encoding="utf8")
            starttime = time.time()
            clientSocket.sendto(message, (serverName, serverPort))
            try:
                response = clientSocket.recvfrom(2048)  #may timeout
                endtime = time.time()
                print("time: %d ms" % ((endtime - starttime)*1000))
            except Exception as e:
                print(e)
                print("Timeout or other fuck problem")

    elif choose == 'end':
        break
    else:
        continue

