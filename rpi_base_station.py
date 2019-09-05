# ===============================================================================
# Copyright 2019 ross
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ===============================================================================
import os
import time
from datetime import datetime
from http.server import HTTPServer, BaseHTTPRequestHandler
from threading import Thread, Lock

import serial


class RPiBaseStation(object):
    def __init__(self, port):
        self._ctx = None
        # self._dev = serial.Serial(port=port)
        self._lock = Lock()

    def run(self):
        t = Thread(target=self._loop)
        t.setDaemon(True)
        t.start()

    def get_context(self):
        with self._lock:
            return self._ctx

    def _loop(self):
        while 1:
            resp = self._recv()
            if resp:
                self._parse(resp)

    def _parse(self, resp):
        """
        :param resp:
        :return:
        """
        headert = (('NODE', 2),
                   ('PACKET', 2),
                   ('TEMPGND', 4),
                   ('TEMPMID', 4),
                   ('TEMPTOP', 4),
                   ('Ain', 4),
                   ('STATE', 1),
                   ('VBATT*', 4),
                   ('RSSI', 2),
                   ('FROM', 2))

        resp = resp.strip()

        rdata = []
        cbit = 0
        header, lens = zip(*headert)
        header = list(header)
        for k, l in headert:
            rdata.append(resp[cbit:cbit + l])
            cbit += l

        data = [int(d, 16) for d in rdata]
        fdata = [str(d) for d in data]

        vraw = data[7]
        vbatt = vraw * 3.3 * 2 / 1024

        header.append('VBATT')
        rdata.append('')
        fdata.append('{:0.2f}'.format(vbatt))

        with self._lock:
            self._ctx = {k: v for k, v in zip(header, fdata)}
            self._ctx['update_timestamp'] = ts = datetime.now().isoformat()

        os.system('clear')
        print('--------- {}'.format(ts))
        print(''.join(['{:<10s}'.format(h) for h in header]))
        print(''.join(['{:<10s}'.format(str(r)) for r in rdata]))
        print(''.join(['{:<10s}'.format(f) for f in fdata]))

    def _recv(self):
        resp = self._dev.readline()
        return resp.decode('utf8')


PORT = '/dev/tty'
BaseStation = RPiBaseStation(port=PORT)


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.end_headers()

        ctx = BaseStation.get_context()
        with open('home.html', 'r') as rfile:
            template = rfile.read()

        if ctx:
            template = template.format(**ctx)
        self.wfile.write(template)


def main():
    run()


def test():
    x = '01080010001e00120000002990000'
    BaseStation._parse(x)


def run():
    BaseStation.run()
    server_klass = HTTPServer
    handler_klass = Handler

    server_address = ('', 8000)
    httpd = server_klass(server_address, handler_klass)
    httpd.serve_forever()


if __name__ == '__main__':
    # main()
    test()

# ============= EOF =============================================
