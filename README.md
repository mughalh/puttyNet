# puttyNet
A Linux based Advanced Networking Decentralized Messaging and Audio call networking application written in c/c++

**Hardware Requirements:**

* A laptop with a Wi-Fi card that you want to program
* A development environment (e.g., Linux, Windows, or macOS)
* A C++ compiler (e.g., GCC or Clang)
* The Wi-Fi card's device driver code

**Software Development Process:**

1. **Familiarize yourself with the laptop's hardware and firmware:** Research the laptop's hardware components, including the Wi-Fi card, its drivers, and firmware.
2. **Understand the Wi-Fi card's protocol stack:** Study the Wi-Fi card's protocol stack, including the 802.11 protocols (e.g., 802.11a/b/g/n/ac/ax).
3. **Choose a programming framework:** Select a suitable programming framework for your project, such as Linux's `netlink` API or Windows' `WinAPI`.
4. **Obtain the Wi-Fi card's device driver code:** Acquire the device driver code for the Wi-Fi card from the laptop manufacturer or through reverse engineering.
5. **Write the custom firmware:** Develop a custom firmware that communicates with the Wi-Fi card using the obtained device driver code and your chosen programming 
framework.
6. **Compile and load the firmware:** Compile the custom firmware and load it into the Wi-Fi card using a tool like `fwtool` or `dfu-util`.
7. **Test and refine the firmware:** Test the custom firmware with various scenarios, such as connecting to Wi-Fi networks, adjusting channel settings, and measuring 
performance.

**Programming Languages:**

Some popular programming languages for Wi-Fi card development include:

* C (or C++)
* Assembly language (e.g., x86 or ARM)
* Rust
* Python (using libraries like `scapy` or `pyWiFi`)

**Device-Specific APIs:**

To interact with the Wi-Fi card, you'll need to use device-specific APIs. Some examples include:

* Linux's `netlink` API
* Windows' `WinAPI`
* macOS's `IOKit`

**Important Considerations:**

* **Security:** Be aware of security implications when modifying or programming a laptop's Wi-Fi card.
* **Compatibility:** Ensure that your custom firmware is compatible with the laptop's operating system and other devices on the network.
* **Reverse engineering:** You may need to reverse engineer the Wi-Fi card's device driver code, which can be a complex task.

**Dependencies:**

* `libpcap`
* `libelf`

**Node structure:**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Node structure
typedef struct {
    uint32_t node_id;
    char node_name[256];
    struct timespec timestamp;
} Node;

// Function to generate a random node ID
uint32_t generate_node_id() {
    return (random() % 65536) * 65536 + (random() % 65536);
}

// Function to broadcast packets to other nodes
void broadcast_packets(uint8_t* packet, uint16_t length) {
    // Get the IP address and port of this node
    struct ifaddrs *ifa;
    struct interface_attr *ia;
    char ip[256];
    char port[256];

    // ...

    // Create a UDP socket to send packets to other nodes
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // Set the IP address and port of this node as the broadcast address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "255.255.255.255", &addr.sin_addr);
    strcpy(addr.sin_port, port);

    // Broadcast packets to other nodes
    sendto(sock, packet, length, 0, (struct sockaddr *)&addr, sizeof(addr));
}

int main() {
    // Generate a random node ID and name
    uint32_t node_id = generate_node_id();
    char node_name[256];
    sprintf(node_name, "Node-%u", node_id);

    // Initialize the Node structure
    Node node;
    node.node_id = node_id;
    strcpy(node.node_name, node_name);
    clock_gettime(CLOCK_REALTIME, &node.timestamp);

    // Open the WiFi adapter for packet capture
    pcap_t *handle;
    struct pcap_pkthdr hdr;
    char* errbuf;

    handle = pcap_open_live("/dev/wlan0", 1024, 1, 1000, errbuf);
    if (handle == NULL) {
        perror(errbuf);
        exit(1);
    }

    // Print the IP address and port of this node
    struct ifaddrs *ifa;
    struct interface_attr *ia;
    char ip[256];
    char port[256];

    // ...

    while (1) {
        // Capture a packet from the network interface
        pcap_pkthdr pkthdr;
        const u_char* packet = pcap_next(handle, &pkthdr);
        if (packet == NULL)
            break;

        // Process the packet (e.g., extract IP address and port)
        // ...

        // Broadcast the packet to other nodes
        broadcast_packets(packet, pkthdr.len);

        // Print the timestamp of this node
        printf("%u.%06d %s\n", node.timestamp.tv_sec, node.timestamp.tv_nsec/1000000, node.node_name);
    }

    return 0;
}
```
**Explanation:**

1. The code generates a random node ID and name using the `generate_node_id` function.
2. It initializes the Node structure with the generated node ID and name.
3. It opens the WiFi adapter for packet capture using `pcap_open_live`.
4. It enters an infinite loop, capturing packets from the network interface using `pcap_next`.
5. For each captured packet, it processes the packet (e.g., extracts IP address and port) and broadcasts it to other nodes using the `broadcast_packets` function.
6. It prints the timestamp of this node to the console.

**Note:** This code is a basic example and does not include error checking or security features that would be necessary in a production system. Additionally, it assumes 
that the WiFi adapter is connected to a network with IP addresses that are within the broadcast range (255.255.255.255).

To compile this code, you'll need to install `libpcap` and `libelf`. You can do this using the following commands:
```bash
sudo apt-get install libpcap-dev libelf-dev
```
Then, compile the code using:
```bash
gcc -o node node.c -lpcap -lef
```
This will generate an executable file called `node` that you can run on your Linux system.


1. **Radar-based approach:** This method uses a radar antenna to emit radio waves and measure the time it takes for them to bounce back from objects in the environment.
2. **Ultrasonic-based approach:** This method uses high-frequency ultrasonic waves (typically above 20 kHz) to detect the distance of objects.
3. **RFID-based approach:** This method uses Radio Frequency Identification (RFID) tags to track the movement and proximity of devices.

Here's an example of how you could use a Wi-Fi-based approach to detect the distance of other devices in the same room on Windows:

**Method 1: Using Wi-Fi Beacon Technology**

Wi-Fi beacon technology involves transmitting and receiving packets from nearby devices. The distance between two devices can be estimated based on the strength of the 
received signal.

To implement this method, you'll need:

* A device with a Wi-Fi adapter (e.g., laptop or desktop)
* A software library for Wi-Fi packet capturing and analysis (e.g., `libpcap`)

Here's an example C code snippet that demonstrates how to detect nearby devices using Wi-Fi beacon technology:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Structure for storing device information
typedef struct {
    uint32_t mac_address[6];
    float distance; // estimated distance from the current device
} DeviceInfo;

// Function to detect nearby devices using Wi-Fi beacon technology
void detect_nearby_devices() {
    pcap_t *handle;
    struct pcap_pkthdr hdr;
    char* errbuf;

    handle = pcap_open_live("/dev/wlan0", 1024, 1, 1000, errbuf);
    if (handle == NULL) {
        perror(errbuf);
        return;
    }

    while (1) {
        pcap_pkthdr pkthdr;
        const u_char* packet = pcap_next(handle, &pkthdr);

        // Process the received packet
        // ...

        // Estimate the distance from the current device based on the packet strength
        float signal_strength = calculate_signal_strength(packet);
        float estimated_distance = estimate_distance(signal_strength);

        // Store the device information
        DeviceInfo* info = malloc(sizeof(DeviceInfo));
        memcpy(info->mac_address, packet, 6 * sizeof(uint8_t));
        info->distance = estimated_distance;
        device_list.push_back(info);
    }
}

// Function to calculate signal strength from a received packet
float calculate_signal_strength(const u_char* packet) {
    // Implement your own signal strength calculation algorithm here
    return packet[0] + packet[1];
}

// Function to estimate distance based on signal strength
float estimate_distance(float signal_strength) {
    // Implement your own distance estimation algorithm here
    return (signal_strength * 10.0) / 100.0;
}
```
This code snippet uses `libpcap` to capture Wi-Fi packets and estimates the distance of nearby devices based on the signal strength.

**Method 2: Using Ultrasonic Sensors**

Ultrasonic sensors can be used to detect objects in a room by emitting high-frequency sound waves and measuring the time it takes for them to bounce back.

To implement this method, you'll need:

* An ultrasonic sensor module (e.g., HC-SR04)
* A microcontroller or single-board computer (e.g., Arduino, Raspberry Pi)

Here's an example code snippet that demonstrates how to use an ultrasonic sensor to detect the distance of nearby devices:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure for storing device information
typedef struct {
    uint32_t mac_address[6];
    float distance; // estimated distance from the current device
} DeviceInfo;

// Function to detect nearby devices using ultrasonic sensors
void detect_nearby_devices() {
    int ultrasonic_pin = 12; // Pin connected to the ultrasonic sensor module
    float max_distance = 10.0f; // Maximum detected distance

    while (1) {
        // Trigger the ultrasonic sensor and measure the time it takes for the sound wave to bounce back
        float measured_time = measure_sound_wave(ultrasonic_pin, max_distance);

        // Calculate the estimated distance based on the measured time
        float estimated_distance = calculate_distance(measured_time);

        // Store the device information
        DeviceInfo* info = malloc(sizeof(DeviceInfo));
        memcpy(info->mac_address, "00:11:22:33:44:55", 6 * sizeof(uint8_t)); // Replace with actual MAC address
        info->distance = estimated_distance;
        device_list.push_back(info);
    }
}

// Function to measure the time it takes for the sound wave to bounce back
float measure_sound_wave(int ultrasonic_pin, float max_distance) {
    // Implement your own sound wave measurement algorithm here
    return measured_time; // Replace with actual measured time
}

// Function to calculate distance based on measured time
float calculate_distance(float measured_time) {
    // Implement your own distance calculation algorithm here
    return (measured_time * 343.0) / 1000.0;
}
```
This code snippet uses an ultrasonic sensor module to detect the distance of nearby devices and estimates the MAC address from a received signal.

1. **Wi-Fi-based Distance Estimation**: This method uses the Received Signal Strength Indicator (RSSI) to estimate the distance between two Wi-Fi devices. The RSSI value 
decreases with increasing distance.
2. **Wi-Fi-based Time-Domain Reflectometry (TDR)**: This method uses the time it takes for a Wi-Fi signal to bounce back from an object to estimate its distance.

Here's an example of how you could use Wi-Fi-based Distance Estimation on a laptop:

**Method 1: Using Wi-Fi RSSI**

To implement this method, you'll need:

* A laptop with a built-in Wi-Fi card
* A software library for accessing the Wi-Fi card's RSSI values (e.g., `libwlan` or `lwres）

Here's an example Python code snippet that demonstrates how to use Wi-Fi RSSI to detect nearby devices:
```python
import wlan

# Get the Wi-Fi card's RSSI value
rssi_value = wlan.get_rssi()

# Calculate the estimated distance based on the RSSI value
estimated_distance = calculate_distance(rssi_value)

def calculate_distance(rssi_value):
    # Implement your own distance calculation algorithm here
    # For example:
    return (rssi_value * 100) / 2000.0

print("Estimated distance:", estimated_distance, "m")
```
This code snippet uses the `libwlan` library to access the Wi-Fi card's RSSI value and estimates the distance of nearby devices based on that value.

**Method 2: Using Wi-Fi-based TDR**

To implement this method, you'll need:

* A laptop with a built-in Wi-Fi card
* A software library for accessing the Wi-Fi card's transmit and receive times (e.g., `libwlan` or `lwres`)
* An additional sensor (e.g., a microphone) to detect the time it takes for a Wi-Fi signal to bounce back from an object

Here's an example Python code snippet that demonstrates how to use Wi-Fi-based TDR to detect nearby devices:
```python
import wlan
import pyaudio

# Get the Wi-Fi card's transmit and receive times
transmit_time = wlan.get_transmit_time()
receive_time = wlan.get_receive_time()

# Calculate the estimated distance based on the transmit and receive times
estimated_distance = calculate_distance(transmit_time, receive_time)

def calculate_distance(transmit_time, receive_time):
    # Implement your own distance calculation algorithm here
    # For example:
    return ((transmit_time + receive_time) * 1000) / 200.0

print("Estimated distance:", estimated_distance, "m")
```
This code snippet uses the `libwlan` library to access the Wi-Fi card's transmit and receive times and estimates the distance of nearby devices based on those values.

Here's how it works:

1. **RSSI Measurement**: The laptop's Wi-Fi card measures the RSSI value of nearby Wi-Fi networks, including the unknown device.
2. **Spatial Triangulation**: By taking into account the RSSI values at multiple points around the laptop (e.g., top, bottom, left, right, front, and back), you can use 
spatial triangulation to estimate the bearing of the unknown device.
3. **Azimuth Calculation**: Using the measured RSSI values and spatial triangulation, you can calculate the azimuth (bearing) of the unknown device in degrees.

The process is similar to how GPS devices work, where multiple satellites are used to determine the user's location by measuring the time delay between signal transmission 
and reception.

**Challenges and Limitations**

While it's possible to estimate the direction of another Wi-Fi-laptop device using an laptop's Wi-Fi card, there are some challenges and limitations to consider:

* **Interference**: Other wireless devices can cause interference, reducing the accuracy of RSSI measurements.
* **Multipath Effects**: Wi-Fi signals can bounce off surfaces, causing multipath effects that affect RSSI measurements.
* **Device Movement**: The unknown device's movement can cause changes in its position and bearing, affecting the accuracy of direction estimation.
* **Saturation Points**: RSSI values may saturate at certain distances or angles, limiting the accuracy of direction estimation.

**Implementation**

To implement this technique, you'll need:

1. A laptop with a Wi-Fi card
2. A programming language (e.g., Python) and libraries for accessing the Wi-Fi card's RSSI values
3. A software framework for processing and analyzing the RSSI data

Some popular libraries for accessing Wi-Fi RSSI values on Linux and Windows include:

* `libwlan` (Linux)
* `lwres` (Windows)
* `wlan0` (Python)

For processing and analyzing the RSSI data, you can use libraries like:

* `scipy` (Python) for signal processing
* `numpy` (Python) for numerical computations

**Example Code**

Here's a simplified example of how to estimate the direction of another Wi-Fi-laptop device using an laptop's Wi-Fi card:
```python
import wlan
import numpy as np

# Set up Wi-Fi interface
wlan_interface = wlan.wlan0()

# Measure RSSI values at multiple points around the laptop (e.g., top, bottom, left, right)
rssi_values = []
for i in range(8):
    rssi_value = wlan_interface.get_rssi(i * 90)  # measure every 45 degrees
    rssi_values.append(rssi_value)

# Calculate bearings using spatial triangulation
bearings = np.arctan2(np.diff(rssi_values), np.sum(rssi_values))

print("Bearing of unknown device:", bearings)
```
