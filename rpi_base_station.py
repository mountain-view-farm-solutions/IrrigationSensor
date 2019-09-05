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
from http.server import HTTPServer, BaseHTTPRequestHandler


from app.base_station import BaseStation


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.end_headers()

        ctx = BaseStation.get_context()
        with open('home.html', 'r') as rfile:
            template = rfile.read()

        if ctx:
            template = template.format(**ctx)
        self.wfile.write(template.encode('utf8'))


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
    main()
    # test()

# ============= EOF =============================================
