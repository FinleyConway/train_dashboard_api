# https://stackoverflow.com/a/74633230

import sys
import time
import signal
import socket
from zeroconf import ServiceInfo, Zeroconf

class dns_broadcaster:
    def __init__(self, hostname, port):
        ip = self._get_ip()

        self._service = ServiceInfo(
            "_mytcp._tcp.local.",
            f"{hostname.replace(".local", "")}._mytcp._tcp.local.",
            addresses=[socket.inet_aton(ip)],
            port=int(port),
            server=f"{hostname}."
        )

        self._zeroconf = Zeroconf()

        signal.signal(signal.SIGINT, self._shutdown)
        signal.signal(signal.SIGTERM, self._shutdown)

    def run(self):
        # broadcast "{name}.local" as server ip
        self._zeroconf.unregister_all_services()
        self._zeroconf.register_service(self._service)
        
        # make the program sleep
        while True:
            time.sleep(1)

    def _shutdown(self, signum, frame):
        # clean up when signals were called
        self._zeroconf.unregister_service(self._service)
        self._zeroconf.close()
        sys.exit(0)

    # bit of a hack
    # https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
    def _get_ip(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip


def main():
    hostname = sys.argv[1]
    port = sys.argv[2]

    dns = dns_broadcaster(hostname, port)
    dns.run()

if __name__ == "__main__":
    main()