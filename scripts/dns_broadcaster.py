# https://stackoverflow.com/a/74633230

import sys
import time
import signal
import socket
from zeroconf import ServiceInfo, Zeroconf

class dns_broadcaster:
    def __init__(self, ip, port):
        self._info = ServiceInfo(
            "_mytcp._tcp.local.",
            "esp_server._mytcp._tcp.local.",
            addresses=[socket.inet_aton(ip)],
            port=int(port)
        )
        self._zeroconf = Zeroconf()

        signal.signal(signal.SIGINT, self._shutdown)
        signal.signal(signal.SIGTERM, self._shutdown)

    def run(self):
        # broadcast "esp_server.local" as server ip
        self._zeroconf.register_service(self._info)
        
        # make the program sleep
        while True:
            time.sleep(1)

    def _shutdown(self, signum, frame):
        # clean up when signals were called
        self._zeroconf.unregister_service(self._info)
        self._zeroconf.close()
        sys.exit(0)


def main():
    ip = sys.argv[1]
    port = sys.argv[2]

    dns = dns_broadcaster(ip, port)
    dns.run()

if __name__ == "__main__":
    main()