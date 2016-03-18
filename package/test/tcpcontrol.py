# client  

import socket
import struct
import string, threading, time
import time


class tcpcontrol:

    def __init__(self):
        self.address = ('127.0.0.1', 4721)

        self.request_data =  [[0] * 4 for row in range(100)]
        self._init_request_data(self.request_data)
        pass

    def __del__(self):
        pass

    def _init_request_data(self, data):
        i = 0
        while i < 100:
            data[i][0] = str(i + 1000)
            data[i][1] = '127.0.0.1'
            data[i][2] = str(i + 1000)
            data[i][3] = 'PCMU'
            i += 1
        pass

    def _reqString(self, data):
        str2 = '<?xml version="1.0" encoding="UTF-8"?>' + \
            '<RegisterTerminalRequest>' +\
                '<device typeOfNumber="other" mediaClass="notKnown" bitRate="constant">' + data[0] + '</device>' +\
                '<localMediaInfo>' +\
                    '<rtpAddress>' +\
                        '<address>' + data[1] + '</address>' +\
                        '<port>' + data[2] + '</port>' +\
                    '</rtpAddress>' +\
                    '<rtcpAddress>' +\
                        '<address>' + data[1] + '</address>' +\
                        '<port>' + data[2] + '</port>' +\
                    '</rtcpAddress>' +\
                    '<codecs>' + data[3] + '</codecs>' +\
                    '<packetSize>20</packetSize>' +\
                    '<encryptionList>none</encryptionList>' +\
                '</localMediaInfo>' +\
            '</RegisterTerminalRequest>'
        a_version = socket.htons(1)
        b_len = socket.htons(len(str2) + 8)
        print 'header len = ' + str(len(str2))
        c_InvokeID = socket.htonl(100)
        str1 = struct.pack("HHI", a_version, b_len, c_InvokeID)
        return str1 +  str2

    def _heartbeat(self, data):
        str2 = '<ResetApplicationSessionTimer >' +\
                    '<requestedSessionDuration>' + '60' + '</requestedSessionDuration>'\
               '</ResetApplicationSessionTimer>'
        a_version = socket.htons(1)
        b_len = socket.htons(len(str2) + 8)
        print 'header len = ' + str(len(str2))
        c_InvokeID = socket.htonl(100)
        str1 = struct.pack("HHI", a_version, b_len, c_InvokeID)
        return str1 +  str2

    def _unregister(self, data):
        str2 = '<?xml version="1.0" encoding="UTF-8"?>' + \
            '<UnRegisterTerminalRequest>' +\
                '<device typeOfNumber="other" mediaClass="notKnown" bitRate="constant">' + data[0] + '</device>' +\
            '<UnRegisterTerminalRequest>'
        a_version = socket.htons(1)
        b_len = socket.htons(len(str2) + 8)
        print 'header len = ' + str(len(str2))
        c_InvokeID = socket.htonl(100)
        str1 = struct.pack("HHI", a_version, b_len, c_InvokeID)
        return str1 +  str2

    def GetDate(self, i, t):
        if t == 'register':
            data = self.request_data[i]
            str = self._reqString(data)
            print str
            return str
        elif t == 'heartbeat':
            data = self.request_data[i]
            str = self._heartbeat(data)
            print str
            return str
        elif t == 'unregister':
            data = self.request_data[i]
            str = self._unregister(data)
            print str
            return str
        else:
            print 'nothing'



    def test_one(self, i):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(self.address)
        str1 = self.GetDate(i, 'register')
        s.send(str1)

        for i in range(1, 100):
            str2 = self.GetDate(i, 'heartbeat')
            s.send(str2)
            time.sleep(0.1)

        str3 = self.GetDate(i, 'unregister')
        s.send(str3)
        time.sleep(1)

        s.close()
        pass

    def test(self, m):
        threads = []
        i = 1
        while i <= m:
            threads.append(threading.Thread(target=self.test_one, args=(i,)))
            i += 1
        for t in threads:
            t.start()
        for t in threads:
            t.join()



if __name__ == "__main__":
    i = 1
    #tcpcontorltmp = tcpcontrol()
    #tcpcontorltmp.test_one()
    while i <= 1:
        tcpcontorltmp = tcpcontrol()
        tcpcontorltmp.test(40)
        time.sleep(0.1)
        #i += 1
