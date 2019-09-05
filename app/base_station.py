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
from threading import Thread, Lock

import serial


class RPiBaseStation(object):
    debug = True

    def __init__(self, port):
        self._ctx = {}
        self._dev = serial.Serial(port=port)
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
                try:
                    self._parse(resp)
                except BaseException as e:
                    print('error: ', e, 'resp', resp, len(resp))

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

        if self.debug:
            os.system('clear')
            print('--------- {}'.format(ts))
            print(''.join(['{:<10s}'.format(h) for h in header]))
            print(''.join(['{:<10s}'.format(str(r)) for r in rdata]))
            print(''.join(['{:<10s}'.format(f) for f in fdata]))

    def _recv(self, timeout=4):
        # resp = self._dev.readline()
        resp = b''
        st = time.time()
        while 1:
            inw = self._dev.inWaiting()
            if inw:
                resp += self._dev.read(inw)
            if resp.endswith('\r\n'):
                break
            if time.time() - st > timeout:
                break
            time.sleep(0.1)

        return resp.decode('utf8')


PORT = '/dev/ttyAMA0'
BaseStation = RPiBaseStation(port=PORT)

# ============= EOF =============================================
