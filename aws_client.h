/*
 * Copyright 2019 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file
 *  CY AWS IoT Client Library interface
 */
#ifndef AWSCLIENT_H
#define AWSCLIENT_H

#include "aws_common.h"
#include "NetworkInterface.h"
#include "MQTTClient.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"

using namespace MQTT;

/*****************************************************************************/
/**
 *
 * @defgroup aws_iot_client          CY AWS IoT Client Library
 *
 * Communication functions for AWS IoT & AWS Greengrass services
 *
 * AWS IoT provides secure, bi-directional communication between Internet-connected devices such as sensors,
 * actuators, embedded micro-controllers, or smart appliances and the AWS Cloud.
 *
 * AWS Greengrass is software that extends AWS cloud capabilities to local devices(typically Edge or Gateway devices),
 * making it possible for them to collect and analyze data closer to the source of information (Nodes, CY IoT Devices, Amazon FreeRTOS devices).
 * With AWS Greengrass, devices securely communicate on a local network and exchange messages with each other
 * without having to connect to the cloud. AWS Greengrass provides a local pub/sub message manager that can
 * intelligently buffer messages if connectivity is lost so that inbound and outbound messages to the cloud are preserved.
 *
 * This library provides application developers an easy-to-use, unified interface for quickly enabling
 * AWS communication in their applications. The library provides a single interface to communicate with AWS
 * using different protocols. Currently, only MQTT & HTTP ( using Client Certificates )
 * are supported. See <https://docs.aws.amazon.com/iot/latest/developerguide/protocols.html> for more details.
 *
 *  @{
 */
/*****************************************************************************/

/*******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct MQTT::MessageData  aws_iot_message_t;
typedef struct MQTT::Message      aws_message_t;

typedef void (*aws_greengrass_callback)( aws_greengrass_discovery_callback_data_t* );

typedef void (*subscriber_callback)( aws_iot_message_t& );

class AWSIoTEndpoint
{

private:
    aws_iot_transport_type_t transport;
    const char* root_ca;
    uint16_t root_ca_length;
    TLSSocket* socket;
    char* uri;
    uint16_t port;

    friend class AWSIoTClient; /* AWSClient can access private members of AWSEndpooint */
};

class AWSIoTClient {

public:

    AWSIoTClient();

    /** Initializes AWS IoT client library
     *
     * @param[in] network             : Network interface (WiFI, Ethernet etc )
     * @param[in] thing_name          : Name of the IoT thing
     * @param[in] private_key         : Private key of device/thing
     * @param[in] key_length          : Length of private key of device/thing
     * @param[in] certificate         : Certificate of device/thing
     * @param[in] certificate_length  : Length of certificate of device/thing
     *
     * @return @ref cy_rslt_t
     *
     */
    AWSIoTClient ( NetworkInterface* network, const char* thing_name, const char* private_key, uint16_t key_length, const char* certificate,uint16_t certificate_length );

    /** Creates endpoint instance using the information provided to connect to server.
     *
     * @param[in] transport           : AWS transport to be used
     * @param[in] uri                 : URI of the AWS endpoint
     * @param[in] port                : Port of AWS endpoint
     * @param[in] root_ca             : RootCA certificate
     * @param[in] rootca_length       : Length of RootCA certificate
     *
     * @return @ref cy_rslt_t
     *
     */
    AWSIoTEndpoint* create_endpoint( aws_iot_transport_type_t transport, const char* uri, int port, const char* root_ca, uint16_t root_ca_length);

    /** Set command timeout for AWS IoT client library commands ex. connect, publish, subscribe and unsubscribe.
     *  Default value is set to 5000ms. 
     *  Use this API to change the timeout value. This API needs to be called after creation of AWS endpoint
     *  and before the MQTT connect operation.
     *
     * @param[in] command_timeout     : timeout in milliseconds
     *
     */
    void set_command_timeout( int command_timeout );

    /** Discovers Greengrass cores(groups) of which this 'Thing' is part of.
     *
     * @param[in] transport           : AWS transport to be used
     * @param[in] uri                 : URI of the AWS endpoint
     * @param[in] root_ca             : RootCA certificate
     * @param[in] rootca_length       : Length of RootCA certificate
     * @param[in] gg_cb               : Greengrass discovery payload callback - Notifies application of Greengrass Cores information
     *
     * @return @ref cy_rslt_t
     *
     */
    cy_rslt_t discover( aws_iot_transport_type_t transport, const char* uri, const char* root_ca, uint16_t root_ca_length, aws_greengrass_callback gg_cb );


    /** Establishes connection to an AWS IoT or Greengrass core
     * This API is blocking and shall return when CONACK is received from server or timeout occurs
     *
     * @param[in] ep              : AWS endpoint object
     * @param[in] conn_params     : Connection parameters
     *
     * @return @ref cy_rslt_t
     *
     */
    cy_rslt_t connect( AWSIoTEndpoint* ep, aws_connect_params conn_params);


    /** Publishes message to user defined topic on AWS cloud
     * This API is blocking and shall return when PUBACK is received from server or timeout occurs
     *
     *
     * @param[in] ep              : AWS endpoint object
     * @param[in] topic           : Contains the topic to which the message is to be published
     * @param[in] data            : Pointer to the message to be published
     * @param[in] length          : Length of the message pointed by 'message'
     * @param[in] pub_params      : Publish paramaters
     *
     * @return @ref cy_rslt_t
     */
    cy_rslt_t publish( AWSIoTEndpoint* ep, const char* topic, const char* data, uint32_t length, aws_publish_params pub_params );


    /** Subscribes to the user defined topic on AWS cloud 
     * This API is blocking and shall return when SUBACK is received from server or timeout occurs
     *
     *
     * @param[in] ep              : AWS endpoint object
     * @param[in] topic           : Contains the topic to be subscribed to
     * @param[in] qos             : QoS level to be used for receiving the message on the given topic
     * @param[in] cb              : Subscriber callback for the topic to receive the messages
     *
     * @return @ref cy_rslt_t
     */
    cy_rslt_t subscribe( AWSIoTEndpoint* ep, const char* topic, aws_iot_qos_level_t qos, subscriber_callback cb );

    /** Unsubscribes from the topic that has been previously subscribed to on AWS cloud
     *
     * @param[in] aws             : Connection handle
     * @param[in] topic           : Contains the topic to be unsubscribed from
     *
     * @return @ref cy_rslt_t
     */
    cy_rslt_t unsubscribe( AWSIoTEndpoint* ep, char* topic );

    /** A call to this API must be made within the keepAlive interval to keep the MQTT connection alive.
     *  This API can be invoked if no other MQTT operation is needed. 
     *  This will also allow messages to be received.
     *
     *  @param[in] timeout_ms     : the time to wait, in milliseconds. Recommend threshold timeout value 1000ms in order to allow adequate time for the system to receive and decode data
     *
     *  @return @ref cy_rslt_t
     */
    cy_rslt_t yield( unsigned long timeout_ms = 1000L );

    /** Disconnects from AWS IoT endpoint
     *
     * @param[in] ep              : AWS endpoint object
     * @param[in] command_timeout : Time to wait for the CONACK from server
     *
     * @return @ref cy_rslt_t
     *
     */
    cy_rslt_t disconnect( AWSIoTEndpoint* ep );


private:
    NetworkInterface* network;
    const char* thing_name;
    const char* private_key;
    uint16_t key_length;
    const char* certificate;
    uint16_t certificate_length;
    int command_timeout;
    MQTT::Client<MQTTNetwork, Countdown> *mqtt_obj;
    MQTTNetwork *mqttnetwork;
    mqtt_security_flag flag;
};

#endif
