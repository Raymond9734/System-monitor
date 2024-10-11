#include "header.h"


std::vector<NetworkInterface> getNetworkInfo() {
    std::vector<NetworkInterface> interfaces;
    std::ifstream netdev("/proc/net/dev");
    std::string line;

    // Skip the first two lines (headers)
    std::getline(netdev, line);
    std::getline(netdev, line);

    while (std::getline(netdev, line)) {
        std::istringstream iss(line);
        NetworkInterface iface;
      

        iss >> iface.name
            >> iface.rx.bytes >> iface.rx.packets >> iface.rx.errs >> iface.rx.drop
            >> iface.rx.fifo >> iface.rx.frame >> iface.rx.compressed >> iface.rx.multicast
            >> iface.tx.bytes >> iface.tx.packets >> iface.tx.errs >> iface.tx.drop
            >> iface.tx.fifo >> iface.tx.colls >> iface.tx.carrier >> iface.tx.compressed;

        iface.name.pop_back(); // Remove the colon
        interfaces.push_back(iface);
    }

    // Get IPv4 addresses
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, host, NI_MAXHOST);

            for (auto& iface : interfaces) {
                if (iface.name == ifa->ifa_name) {
                    iface.ipv4 = host;
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);

    return interfaces;
}