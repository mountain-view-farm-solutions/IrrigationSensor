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
from gpiozero import LED, TonalBuzzer
from gpiozero.tones import Tone
import serial
import pickle

# Configuration
# =================================================
PORT = '/dev/ttyAMA0'
BAUDRATE = 19200
BATT_LED_PIN = 'GPIO23'
BUZZER_PIN = 18
STATE_LED_PIN = 'GPIO24'
CONTEXT_PATH = '/tmp/context.ctx'


# =================================================

class BaseStationContext(object):
    def __call__(self):
        if os.path.isfile(CONTEXT_PATH):
            with open(CONTEXT_PATH, 'rb') as rfile:
                return pickle.load(rfile)
        else:
            return {}

    def dump(self, ctx):
        with open(CONTEXT_PATH, 'wb') as wfile:
            pickle.dump(ctx, wfile)


BaseStationCTX = BaseStationContext()


class RPiBaseStation(object):
    debug = False

    def __init__(self):
        self._ctx = {}
        BaseStationCTX.dump({})
        self._dev = serial.Serial(port=PORT, baudrate=BAUDRATE)
        self._batt_led = LED(BATT_LED_PIN)
        self._buzzer = TonalBuzzer(BUZZER_PIN)
        self._state_led = LED(STATE_LED_PIN)

    def run(self):

        t = Thread(target=self._loop)
        t.start()
        t.join()

    def _loop(self):
        while 1:
            try:
                resp = self._recv()
            except BaseException:
                continue

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

        ctx = {k: v for k, v in zip(header, fdata)}
        ctx['update_timestamp'] = ts = datetime.now().isoformat()
        
        for k in ('TOP', 'MID', 'GND'):
            ok = 'TEMP{}'.format(k)
            t = float(ctx[ok])/10.
            ctx['TEMP{}F'.format(k)] = '{:0.1f}'.format(t*9/5.+32)
            ctx[ok] = '{:0.1f}'.format(t)

        self._ctx = ctx
        BaseStationCTX.dump(ctx)

        if self.debug:
            os.system('clear')
            print('--------- {}'.format(ts))
            print(''.join(['{:<10s}'.format(h) for h in header]))
            print(''.join(['{:<10s}'.format(str(r)) for r in rdata]))
            print(''.join(['{:<10s}'.format(f) for f in fdata]))

    def _recv(self):
        resp = self._dev.readline()
        return resp.decode('utf8')


BaseStation = RPiBaseStation()
# ============= EOF =============================================
