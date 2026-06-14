# https://stackoverflow.com/a/74633230

import sys
import time
import signal
import socket
from dataclasses import dataclass
from zeroconf import ServiceInfo, Zeroconf


@dataclass
class service:
    service_type: str
    hostname: str
    port: str


class dns_broadcaster:
    def __init__(self, services):
        self._zeroconf = Zeroconf()
        self._services = []
        ip = self._get_ip()

        for svc in services:
            service_type = svc.service_type
            hostname = svc.hostname.replace(".local", "")
            port = int(svc.port)

            info = ServiceInfo(
                type_=f"_{service_type}._tcp.local.",
                name=f"{hostname}._{service_type}._tcp.local.",
                addresses=[socket.inet_aton(ip)],
                port=port,
                server=f"{hostname}.local."
            )

            self._services.append(info)

        signal.signal(signal.SIGINT, self._shutdown)
        signal.signal(signal.SIGTERM, self._shutdown)

    def run(self):
        # broadcast "{name}.local" as ip
        for service in self._services:
            self._zeroconf.register_service(service)
        
        # make the program sleep
        while True:
            time.sleep(1)

    def _shutdown(self, signum, frame):
        # clean up when signals were called
        for service in self._services:
            self._zeroconf.unregister_service(service)

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
    args = sys.argv[1:]

    if len(args) % 3 != 0:
        raise ValueError("Expected triplets: service_type hostname port")

    services = [
        service(
            service_type = args[i],
            hostname = args[i + 1],
            port = args[i + 2]
        )
        for i in range(0, len(args), 3)
    ]

    dns = dns_broadcaster(services)
    dns.run()

if __name__ == "__main__":
    main()