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

import time
from flask import Flask, render_template
from base_station import BaseStationCTX

bsapp = Flask(__name__)


@bsapp.route('/')
def index():
    ctx = BaseStationCTX()
    # ctx['STATE']= bool(int(ctx.get('STATE', int(time.time())%2 )))
    t = int(time.time())
    # ctx['STATE']=c=bool(t%2)
    ctx['bgcolor'] = '#8DEEF1' if ctx.get('STATE') else '#F0C4C4'
    ctx['update_timestamp'] = t
    ctx['alarm_source'] = '/static/sounds/tornado_siren.mp3'
    return render_template('index.html', **ctx)


if __name__ == '__main__':

    bsapp.run(debug=True, host='0.0.0.0')

# ============= EOF =============================================
