"""
在这个编程作业中，你将用 Py山on 语育开发一个简单的 Web 服务器，它仅能处理一个请求。具体而
育，你的 Web 服务器将: (1) 当一个客户(浏览器)联系时创建一个连接套接字 ; (2) 从这个连接接
收 HTTP 请求; (3) 解释该请求以确定所谓求的特定文件; (4) 从服务榕的文件系统获得请求的文件:
(5) 创建一个由请求的文件组成的 HTTP 响应报文，报文前面有首都行; (6) 经 TCP 连接向请求的浏览应用层 121
器发送响应 如果浏览器请求一个在该服务器中不存在的文件，服务然应当返回一个" 404 Not Found!"
差错报文
在配套网站中，孩们提供了用于该服务器的框架代码 ‘ 你的任务是完善该代码，运行服务器，涵过
在不同主机上运行的浏览器发送请求来测试该服务器 。 如果运行你版务部的主机上已经有-个 Web 服务
器在运行，你应当为该 Web 服务器使用一个不同于 80 揣口的其他端口
"""

#here HTTP relies on TCP
#This problem want me to create a HTTP server all by myself?!

#the sentence seems like a byte info(can decode to string)
"""

b'GET / HTTP/1.1\r\n
Host: 127.0.0.1:12000\r\n
Connection: keep-alive\r\n
Cache-Control: max-age=0\r\n
Upgrade-Insecure-Requests: 1\r\n
User-Agent: Mozilla/5.0(Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3\r\n
Accept-Encoding: gzip, deflate, br\r\n
Accept-Language: zh-CN,zh;q=0.9\r\n
\r\n'


"""

class HTTPINFO:
    rawstring = ""
    method = ""
    url = ""
    version = ""
    otherdicts = {}

    def __init__(self, bytesstr):
        self.rawstring = bytesstr.decode(encoding='utf-8')  #here sometimes rawstring 空字符串, 未处理
        print(self.rawstring)
        templists = self.rawstring.split("\r\n")

        #first line
        firstline = templists[0].split(' ')
        self.method = firstline[0]
        self.url = firstline[1]
        self.version = firstline[2]

        #other lines
        for otherline in templists[1:]:
            temploc = otherline.find(": ")  #find the first ": "
            if temploc == -1:
                continue
            self.otherdicts.update({otherline[0: temploc + 1]: otherline[temploc + 2: ]})

    def getres(self):
        return '1'


from socket import *
serverPort = 12000
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
serverSocket.listen(1)   #same time 1 connections
while True:
    connectionSocket, addr = serverSocket.accept()
    sentence = connectionSocket.recv(1024)
    httpinfo = HTTPINFO(sentence)
    
    #print(sentence)
    #extract these datas


    message = ('HTTP/1.1 404 NOT FOUND\r\n')
    message = bytes(message, encoding="utf8")
    connectionSocket.send(message)
    connectionSocket.close()

