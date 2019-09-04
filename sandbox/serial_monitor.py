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
import serial
import os
import time
import matplotlib.pyplot as plt
from numpy import array


# paces, 5, 5, 10, 20, 20, 40
# last 90

def main():
    port = 'USA19H1427P1.1'
    port = 'usbmodem14601'
    bd = 9600
    dev = serial.Serial('/dev/tty.{}'.format(port), baudrate=bd)

    measure(dev)


def analyze():
    with open('/Users/ross/Sandbox/irrigation_sensor/range_test-001.txt', 'r') as rfile:
        xs, rssi, vbat, errs = [], [], [], []
        cnt = 1
        ts = []
        for line in rfile:
            d = parse(line, delimiter=',')
            if d:
                t = d['time']
                x = d['count']
                x = float(x)
                if x != cnt:
                    print(x, cnt)
                    errs.append(1)
                    # print('sd', x - cnt)
                    # cnt += (x-cnt%255)

                else:
                    errs.append(0)

                ts.append(t)
                # print(x, cnt)
                # xs.append(cnt)
                cnt += 1
                if cnt > 255:
                    cnt = 0
                # print('asfasfa', cnt)
                rssi.append(float(d['RSSI']))
                vbat.append(float(d['battery voltage']))

        # ploterrors(xs, rssi, errs)
        ts = array(ts)
        ts = ts - ts[0]
        plot(ts, rssi, errs)
        plt.show()


def plot(xs, y1, y2):
    fig, ax1 = plt.subplots()

    ax2 = ax1.twinx()
    ax1.plot(xs, y1, 'g-')
    ax2.plot(xs, y2, 'b-')

    ax1.set_xlabel('Time')
    ax1.set_ylabel('RSSI', color='g')
    ax2.set_ylabel('VBatt', color='b')


def unique_path(root, base, extension='.txt'):
    """

    """
    if extension:
        if not extension.startswith('.'):
            extension = '.{}'.format(extension)
    else:
        extension = ''

    p = os.path.join(root, '{}-001{}'.format(base, extension))
    cnt = 1
    i = 2
    while os.path.isfile(p):
        p = os.path.join(root, '{}-{:03d}{}'.format(base, i, extension))
        i += 1
        cnt += 1

    return p, cnt


def get_path():
    root = '/Users/ross/Sandbox/irrigation_sensor'
    base = 'range_test'
    if not os.path.isdir(root):
        os.mkdir(root)

    p, _ = unique_path(root, base)
    return p


def measure(dev):
    newpath = get_path()
    st = time.time()
    duration = 12 * 3600
    while 1:
        ct = time.time()
        if ct - st > duration:
            break

        with open(newpath, 'a') as wfile:
            resp = read(dev)
            # print(resp)
            if resp:
                data = parsebuf(resp)
                data.insert(0, time.time())
                wfile.write('{}\n'.format(','.join(data)))

        time.sleep(0.9)


def parsebuf(resp):
    """
    //0 1  2 3     4 5        6 7   8 9   10 11 12 13  14 15  16 17 18 19  20 21
    //@@   nodeid  packetnum  temp  hum   ain          state  vbatt        rssi

    :param resp:
    :return:
    """

    resp = resp.strip()

    nodid = resp[2:4]
    packetnum = resp[4:6]
    temp = resp[6:8]
    hum = resp[8:10]
    ain = resp[10:14]
    state = resp[14:16]
    vraw = resp[16:20]
    rssi = resp[20:22]
    int = fform = resp[22:24]

    header = ['NODE', 'PACKET', 'TEMP', 'HUM', 'Ain', 'STATE', 'VBATT', 'RSSI' 'FROM']

    rdata = [nodid, packetnum, temp, hum, ain, state, rssi, fform]
    fdata = [str(int(d, 16)) for d in rdata]

    vbatt = vraw * 3.3 * 2 / 1024
    rdata.append(vraw)
    fdata.append('{:0.2f}'.format(vbatt))

    print(''.join(['{:<10s}'.format(h) for h in header]))
    print(''.join(['{:<10s}'.format(r) for r in rdata]))
    print(''.join(['{:<10s}'.format(f) for f in fdata]))
    return rdata


def parse(resp, delimiter='\r\n'):
    # print(resp.strip())
    args = resp.split(delimiter)

    # print('aragsd', args)
    d = {}
    for a in args:
        try:
            k, v = a.split(':')
            d[k.strip()] = v.strip()
        except BaseException:
            pass

    # d['node'] = float(args[0])
    hkeys = ['Cnt', 'State', 'Temp', 'Hum', 'RSSI', 'VBatt']
    keys = ['count', 'State', 'temp', 'hum', '|\nRSSI', 'battery voltage']
    try:
        header = ['{:<10s}'.format(k.lower().strip()) for k in hkeys]
        data = ['{:<10s}'.format(d[k].strip()) for k in keys]

        print('|'.join(header))
        print('|'.join(data))
        return d
    except KeyError as e:
        print('asd', e)
        pass


def read(dev, verbose=False):
    terminator = b'\r\n'
    # terminator = b'|\n'

    resp = b''
    for i in range(150):
        inw = dev.inWaiting()
        if verbose:
            print('try:{} inwaiting: {}'.format(i, inw))
        if inw:
            resp += dev.read(inw)
            if resp.endswith(terminator):
                break

        time.sleep(0.01)

    resp = resp.decode('utf8')
    return resp


if __name__ == '__main__':
    main()
    # analyze()
# ============= EOF =============================================
