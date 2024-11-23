#! /bin/env python3

"""
This is a test server to work on developing the HTML/CSS
without uploading the littleFS to the ESP everytime
"""

import http.server
from urllib.parse import unquote

settings = {
    'hostname': 'emon-XXXX',
    'api_key': '',
    'channel': '',
    'calibration': '62.75',
    'nominal_voltage': '120.0',
    'time_zone': 'PST8PDT,M3.2.0,M11.1.0',
    'ntp_server': 'us.pool.ntp.org'
}

emon_values = {
    'daily_kWh': '12.34',
    'monthly_kWh': '1234.56',
    'watts': '123.45'
}

class RequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        global settings

        if self.path == '/favicon.ico':
            self.send_response(200)
            self.send_header('content-type', 'image/x-icon')
            self.end_headers()

            with open('favicon.ico', 'rb') as favicon:
                self.wfile.write(favicon.read())
                return

        # Handle JS/CSS content
        if self.path == '/static/main.js':
            self.send_response(200)
            self.send_header('content-type', 'text/javascript')
            self.end_headers()

            with open('main.js') as mainJs:
                self.wfile.write(mainJs.read().encode('utf8'))
                return

        if self.path == '/static/bootstrap.min.css':
            self.send_response(200)
            self.send_header('content-type', 'text/css')
            self.send_header("Content-Encoding", "gzip")
            self.end_headers()

            with open('bootstrap.min.css.gz', 'rb') as minCss:
                self.wfile.write(minCss.read())
                return

        if self.path == '/static/bootstrap.min.css.map':
            self.send_response(200)
            self.send_header('content-type', 'text/json')
            self.send_header("Content-Encoding", "gzip")
            self.end_headers()

            with open('bootstrap.min.css.map.gz', 'rb') as minCssMap:
                self.wfile.write(minCssMap.read())
                return

        response = 'not found'
        if self.path == '/':
            self.send_response(200)
            self.send_header('content-type', 'text/html')
            self.end_headers()
            with open('index.html') as indexHtml:
                indexHtmlText = indexHtml.read()
                indexHtmlText = indexHtmlText.replace(r'%daily_kWh%', emon_values['daily_kWh'])
                indexHtmlText = indexHtmlText.replace(r'%monthly_kWh%', emon_values['monthly_kWh'])
                indexHtmlText = indexHtmlText.replace(r'%watts%', emon_values['watts'])
                response = indexHtmlText

        elif self.path == '/settings':
            self.send_response(200)
            self.send_header('content-type', 'text/html')
            self.end_headers()
            with open('settings.html') as settingsHtml:
                settingsHtmlText = settingsHtml.read()
                settingsHtmlText = settingsHtmlText.replace(r'%hostname%', settings['hostname'])
                settingsHtmlText = settingsHtmlText.replace(r'%api_key%', settings['api_key'])
                settingsHtmlText = settingsHtmlText.replace(r'%channel%', settings['channel'])
                settingsHtmlText = settingsHtmlText.replace(r'%calibration%', settings['calibration'])
                settingsHtmlText = settingsHtmlText.replace(r'%nominal_voltage%', settings['nominal_voltage'])
                settingsHtmlText = settingsHtmlText.replace(r'%time_zone%', settings['time_zone'])
                settingsHtmlText = settingsHtmlText.replace(r'%ntp_server%', settings['ntp_server'])
                response = settingsHtmlText

        elif '/settings?' in self.path:
            settings = {arg.split('=')[0]: arg.split('=')[1] for arg in unquote(self.path.split('/settings?')[1]).split('&')}
            self.send_response(302)
            self.send_header('Location','/')
            self.end_headers()

        elif self.path == '/reset':
            response = 'Settings have been reset'

        elif self.path == '/upgrade':
            response = 'Software upgrade initiated...'

        self.wfile.write(response.encode('utf8'))

def main():
    server_address = ('0.0.0.0', 8000)
    server = http.server.HTTPServer(server_address, RequestHandler)
    print('Server listening on port 8000')
    server.serve_forever()

if __name__ == '__main__':
    main()
