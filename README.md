# AWS IoT library

AWS IoT provides secure, bi-directional communication between Internet-connected devices such as sensors, actuators, embedded micro-controllers, or smart appliances and the AWS Cloud.

AWS Greengrass is software that extends AWS cloud capabilities to local devices(typically Edge or Gateway devices), making it possible for them to collect and analyze data closer to the source of information (Nodes, CY IoT Devices, Amazon FreeRTOS devices).
With AWS Greengrass, devices securely communicate on a local network and exchange messages with each other without having to connect to the cloud. AWS Greengrass provides a local pub/sub message manager that can intelligently buffer messages if connectivity is lost so that inbound and outbound messages to the cloud are preserved.

This library provides application developers an easy-to-use, unified interface for quickly enabling AWS communication in their applications. The library provides a single interface to communicate with AWS using different protocols. Currently, only MQTT ( using Client Certificates ) is supported. HTTP is used for Greengrass core discovery. See <https://docs.aws.amazon.com/iot/latest/developerguide/protocols.html> for more details.

This repository contains the AWS IoT client library code. AWS code examples download this library automatically, so you don't need to. 

AWS code examples, for both AWS IoT client and Greengrass nodes, are available for Mbed OS.

Note: Applications that need Greengrass support, should ensure that adequate heap memory (at least 8KB) is available for HTTP client library on memory constrained platforms (such as CY8CKIT_062_WIFI_BT)

## Features
* Supports AWS IoT client APIs to connect, publish and subscribe to topics on the AWS IoT cloud
* Supports AWS Greengrass core discovery and connection to Greengrass cores
* Built on top of Eclipse PAHO MQTT client library
* Designed to work with Cypress' PSoC platforms running ARM Mbed OS 5.15.0

## Supported platforms
This middleware library and it's features are supported on following Cypress platforms:
* [PSoC6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC6 WiFi-BT Pioneer Kit (CY8CKIT-062-WiFi-BT)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit-cy8ckit-062-wifi-bt)
* CY8CKIT-062S2-43012

## Dependencies
This section provides the list of dependency libraries required for this middleware library to work.
* [ARM Mbed OS stack version 5.15.0](https://os.mbed.com/mbed-os/releases)
* [Cypress Connectivity Utilities Library](https://github.com/cypresssemiconductorco/connectivity-utilities)

## Additional Information
* [AWS IoT RELEASE.md](./RELEASE.md)
* [AWS IoT API reference guide](https://cypresssemiconductorco.github.io/aws-iot/api_reference_manual/html/index.html)
* [AWS IoT version](./version.txt)
