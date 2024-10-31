#include "header.h"


/**
 * Retrieves detailed network interface information from the system
 * Reads interface statistics from /proc/net/dev and IPv4 addresses using getifaddrs
 * 
 * Collects the following metrics for each interface:
 * - RX (receive) stats: bytes, packets, errors, drops, fifo, frame errors, compressed, multicast
 * - TX (transmit) stats: bytes, packets, errors, drops, fifo, collisions, carrier errors, compressed
 * - IPv4 address
 *
 * @return Vector of NetworkInterface objects containing stats for each interface
 */
std::vector<NetworkInterface> getNetworkInfo() {
    std::vector<NetworkInterface> interfaces;
    std::ifstream netdev("/proc/net/dev");
    std::string line;

    // Skip the first two lines (headers)
    std::getline(netdev, line);
    std::getline(netdev, line);

    // Parse each line from /proc/net/dev which contains interface statistics
    while (std::getline(netdev, line)) {
        std::istringstream iss(line);
        NetworkInterface iface;
      
        // Read all interface statistics in order they appear in /proc/net/dev
        // Format: name rx_bytes rx_packets rx_errs rx_drop rx_fifo rx_frame rx_compressed rx_multicast 
        //             tx_bytes tx_packets tx_errs tx_drop tx_fifo tx_colls tx_carrier tx_compressed
        iss >> iface.name
            >> iface.rx.bytes >> iface.rx.packets >> iface.rx.errs >> iface.rx.drop
            >> iface.rx.fifo >> iface.rx.frame >> iface.rx.compressed >> iface.rx.multicast
            >> iface.tx.bytes >> iface.tx.packets >> iface.tx.errs >> iface.tx.drop
            >> iface.tx.fifo >> iface.tx.colls >> iface.tx.carrier >> iface.tx.compressed;

        iface.name.pop_back(); // Remove the colon from interface name
        interfaces.push_back(iface);
    }

    // Get IPv4 addresses for each interface using getifaddrs
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    // Get linked list of interfaces
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Iterate through linked list of interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        // Skip if no address assigned
        if (ifa->ifa_addr == NULL)
            continue;

        int family = ifa->ifa_addr->sa_family;

        // Process only IPv4 addresses
        if (family == AF_INET) {
            // Convert IP address to string representation
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, host, NI_MAXHOST);

            // Find matching interface in our vector and assign IPv4
            for (auto& iface : interfaces) {
                if (iface.name == ifa->ifa_name) {
                    iface.ipv4 = host;
                    break;
                }
            }
        }
    }

    // Clean up ifaddrs linked list
    freeifaddrs(ifaddr);

    return interfaces;
}