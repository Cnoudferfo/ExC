# Concept of Muliple IoT Simulator Design
## The process of Installation of IoT devices, in Real World
:::mermaid
graph TD
Prep_a_NICed_host-->Bind_server_to_hostNIC-->Prep_an_IOT_Device-->Launch_firmware_on_this_device -- more devices --> Prep_an_IOT_Device
Launch_firmware_on_this_device-->Server_mcast_ip&port-->IOT_clients_regiter_to_server
:::

## Definition of terms
- System : a system contains ONE host and MULTIPLE devices, it is supposed that one instance of the simulator can have more than one system.
- Host : the entity on which a server program runs.
- Device : the entity on which a client program runs.

## Entity Relationships
:::mermaid
---
title: Host-Devices Relationships
---
classDiagram
    class Host{
        my NIC info
        List of devices
        Thread mcast tx() always sending NIC info
        Thread tcp listener() always
        Thread operation interface()
    }
    class Device{
        my NIC info
        host NIC info
        Thread mcast rx() until host ip received
        Register my NIC() to Host
        Thread host cmd listener()
    }
    Host <|-- Device
    Device <|-- Host
:::

## List of functionalities of each entity
### System
- A system has a determined number of devices and one host
### Host
- A host has an unique id to tell what is the type of this host.
- Always sending multicast(mcast) messages periodically to notify this host's existence, this mcast message contains: host id and host NIC information.
- Always listening(at a tcp socket) for registration request from devices, a registration request of a device contains: device NIC information(ipv4 address:port of device's listening socket), device name.
- Base process of the system to manage the interaction between the devices. For example, to collect status from each device, realize the situation of each device, give operation command to a certain device, etc...
- Always listening(at a tcp socket) for operator's command and response to that command, and this functionality can  be realized as a user interface.
### Device
- A device can only connect to one host with corresponding id during this device's on time.
- A device will first listen to mcast message from host after this device got launched.
- When a device received a mcast message from a host, first to check the host id. If this host is a corresponding one, send a tcp request to this host to register the device itself, then stop listening to mcast message, because a connection with a corresponding host has been established. Otherwise, keep listening to mcast message until a corresponding host is found.
- According to some processing logic of the system, listen and receive commands from host and react on them, and send necessary information to host.

# Suggestion from MS copilot
## Definition of Terms
- System: Consider specifying the maximum number of systems an instance of the simulator can handle, if applicable.
- Host: Clarify if the server program on the host is responsible for any specific tasks beyond managing devices.
- Device: Mention if there are different types of devices or if all devices are treated equally.
## List of Functionalities of Each Entity
### System
- System Composition: You might want to specify if the number of devices in a system is fixed or can be dynamically adjusted.
### Host
- Unique ID: Explain what the unique ID represents (e.g., type, version, etc.).
- Multicast Messages: Specify the frequency of these messages and any protocol details.
- Registration Requests: Clarify if there are any security measures for registration (e.g., authentication).
- Base Process: Consider detailing how the host manages device interactions (e.g., polling intervals, data aggregation methods).
- Operator Commands: Describe the types of commands an operator can issue and any constraints or permissions.
### Device
- Host Connection: Mention if a device can switch hosts if the current host becomes unavailable.
- Multicast Listening: Specify the timeout period for listening to multicast messages before retrying or failing.
- Command Processing: Detail the types of commands a device can receive and any specific actions it can perform.
## Additional Considerations
- Error Handling: Outline how the system handles errors or failures (e.g., lost connections, unresponsive devices).
- Scalability: Discuss how the system scales with an increasing number of devices or hosts.
- Security: Include any security protocols or measures to protect data and communication between hosts and devices.
