# AWS IoT library

AWS IoT provides secure, bi-directional communication between Internet-connected devices such as sensors, actuators, embedded micro-controllers, or smart appliances and the AWS Cloud.

AWS Greengrass is software that extends AWS cloud capabilities to local devices(typically Edge or Gateway devices), making it possible for them to collect and analyze data closer to the source of information (Nodes, CY IoT Devices, Amazon FreeRTOS devices).
With AWS Greengrass, devices securely communicate on a local network and exchange messages with each other without having to connect to the cloud. AWS Greengrass provides a local pub/sub message manager that can intelligently buffer messages if connectivity is lost so that inbound and outbound messages to the cloud are preserved.

This library provides application developers an easy-to-use, unified interface for quickly enabling AWS communication in their applications. The library provides a single interface to communicate with AWS using different protocols. Currently, only MQTT & HTTP ( using Client Certificates ) are supported. See <https://docs.aws.amazon.com/iot/latest/developerguide/protocols.html> for more details.

This repository contains the AWS IoT client library code. AWS code examples download this library automatically, so you don't need to. 

AWS code examples, both AWS IoT client and Greengrass nodes, are available for mbed-os.

Note: Applications that need Greengrass support, should ensure that adequate heap memory (at least 8KB) is available for HTTP client library on memory constrained platforms (such as CY8CKIT_062_WIFI_BT)
