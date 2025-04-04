#!/usr/bin/env python3
"""
License: MIT License
Copyright (c) 2023 Miel Donkers

Very simple HTTP server in python for logging requests
Usage::
    ./server.py [<port>]
"""
from http.server import BaseHTTPRequestHandler, HTTPServer
from time import sleep
from random import randint
import logging

current_channel = 5
max_channel = 16


class S(BaseHTTPRequestHandler):

    def _set_response(self, code, type="text/html"):
        self.send_response(code)
        self.send_header('Content-type', type)
        self.end_headers()
    
    
    def do_GET(self):
        global current_channel

        logging.info("GET request,\nPath: %s\nHeaders:\n%s\n", str(self.path), str(self.headers))
        
        if self.path == "/":
            res = ""
            with open("./data/index.html", 'r') as f:
                res = f.read()
            self._set_response(200)
            self.wfile.write(res.encode('utf-8'))
        elif self.path == "/style.css":
            res = ""
            with open("./data/style.css", 'r') as f:
                res = f.read()
            self._set_response(200, "text/css")
            self.wfile.write(res.encode('utf-8'))
        elif self.path == "/channel_names.js":
            res = ""
            with open("./data/channel_names.js", 'r') as f:
                res = f.read()
            self._set_response(200, "text/javascript")
            self.wfile.write(res.encode('utf-8'))
        elif self.path == "/get_channel":
            logging.info(f" Get channel: {current_channel}")
            self._set_response(200)
            self.wfile.write(str(current_channel).encode('utf-8'))
        elif self.path.startswith("/up?channel=") or self.path.startswith("/down?channel="):
            channel = int(self.path.split("=")[-1])
            logging.info(f" Received channel: {channel}")
            sleep(randint(2, 3)) # random sleep added to simulate the remote
            current_channel = channel
            self._set_response(200)
        else:
            self._set_response(200)
            self.wfile.write("GET request for {}".format(self.path).encode('utf-8'))


def run(server_class=HTTPServer, handler_class=S, port=8080):
    logging.basicConfig(level=logging.INFO)
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    logging.info('Starting httpd...\n')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    logging.info('Stopping httpd...\n')

if __name__ == '__main__':
    from sys import argv

    if len(argv) == 2:
        run(port=int(argv[1]))
    else:
        run()
