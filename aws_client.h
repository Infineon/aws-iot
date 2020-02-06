/*
 * Copyright 2019-2020 Cypress Semiconductor Corporation
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

/**
********************************************************************************
* \mainpage Overview
*********************************************************************************
* Cypress AWS IoT library provides easy-to-use APIs for users to connect to AWS IoT cloud and perform MQTT publish and subscribe operations. The MQTT publish and subscribe operations can be performed either through AWS IoT cloud or through Greengrass core. The APIs provided in this library helps user to discover Greengrass core devices that are on the local area network, and allow them to exchange messages on the local area network without having to connect to the AWS IoT cloud.
* 
* This library provides following features:
*  * Single interface to communicate with AWS using different protocols:
*     - MQTT over TCP sockets (using Client Certificates)
*     - HTTP for Greengrass core discovery \n
*     NOTE: MQTT over Websockets and RESTful HTTPS are not supported currently
*  * Supports Quality of Service (QoS) levels 0 and 1
* 
* User can implement a AWS subscriber/publisher application by using the APIs provided in this library. To communicate with AWS IoT message broker, user should have a 'Thing', 'Policies', 'certificates' and unique client IDs for each AWS client instance. Refer to the section below for AWS IoT terminology.
*
***********************************************************************************
* \section sec_term AWS IoT terminology
***********************************************************************************
* 
* * <b>Thing :</b> A thing is a representation of a specific device or logical entity. It can be a physical device or sensor (for example, a light bulb or a switch on a wall). Things are identified by a name. Things can also have attributes, which are name-value pairs that can be used to store information about the thing, such as its serial number or manufacturer.
* 
* * <b>Client ID :</b> The Client Identifier (ClientID) identifies the Client to the Server. Each Client connecting to the Server has a unique ClientID. The ClientID MUST be used by Clients and by Servers to identify state that they hold relating to this MQTT Session between the Client and the Server.
* 
* * <b>Thing and Client ID association:</b> A typical device use case involves the use of the thing name as the default MQTT client ID. Although AWS does not enforce a mapping between a things registry name and its use of MQTT client IDs, certificates, or shadow state, they recommend that a thing name be used as the MQTT client ID for both the registry and the Device Shadow service. This provides organization and convenience without removing the flexibility of the underlying device certificate model or shadows.
* 
* * <b>Policies :</b> AWS IoT policies are JSON documents. They follow the same conventions as IAM policies. AWS IoT policies allow you to control access to the AWS IoT data plane. The AWS IoT data plane consists of operations that allow you to connect to the AWS IoT message broker, send and receive MQTT messages, and get or update a device's shadow.
* 
* * <b>Security and certificates :</b> Each connected device or client must have a credential to interact with AWS IoT. All traffic to and from AWS IoT is sent securely over Transport Layer Security (TLS). AWS cloud security mechanisms protect data as it moves between AWS IoT and other AWS services. Authentication is a mechanism where you verify the identity of a client (such as AWS IoT device) or a server (AWS IoT cloud). Server authentication is the process where devices or other clients ensure they are communicating with an actual AWS IoT endpoint. Client authentication is the process where devices or other clients authenticate themselves with AWS IoT. Certificates are used during the authentication process to validate the identify of the client and server. X.509 certificates are the digital certificates that are used by the client during AWS IoT authentication. The AWS IoT root CA certificate allows the devices to verify that they are communicating with AWS IoT Core and not another server impersonating AWS IoT Core. Refer to the following section in AWS documentation on Authentication and Key management: https://docs.aws.amazon.com/iot/latest/developerguide/authentication.html
* 
***********************************************************************************
* \section sec_add_info Additional resources
***********************************************************************************
* * <b>AWS IoT :</b> https://docs.aws.amazon.com/iot/latest/developerguide/what-is-aws-iot.html
* * <b>Greengrass :</b> https://docs.aws.amazon.com/greengrass/latest/developerguide/what-is-gg.html
* * <b>Authentication :</b> https://docs.aws.amazon.com/iot/latest/developerguide/authentication.html
* * <b>MQTT Protocol :</b> http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html
*
* \defgroup aws_iot AWS IoT library
* \defgroup aws_iot_macros AWS IoT and Greengrass macros
* @ingroup aws_iot
* \defgroup aws_iot_enums AWS IoT and Greengrass enumerated types
* @ingroup aws_iot
* \defgroup aws_iot_struct AWS IoT and Greengrass data structures
* @ingroup aws_iot
* \defgroup aws_iot_classes AWS IoT and Greengrass base class and sub-classes
* @ingroup aws_iot
*/

/**
 * @addtogroup aws_iot_struct
 *
 * AWS IoT and Greengrass data structures and type definitions
 *
 * @{
 */

/*******************************************************
 *                 Type Definitions
 ******************************************************/

/** AWS IoT Message payload structure using Eclipse MQTTClient MQTT::MessageData. The structure of MessageData extracted from MQTTClient.h:
 *
 * @code
 * struct MessageData
 * {
 *    struct Message &message;
 *    MQTTString &topicName;
 * };
 * @endcode
 *
 * aws_iot_message_t structure is not allocated internally in MQTT library.
 * The application callback should copy the content prior to return.
 *
 * */
typedef struct MQTT::MessageData  aws_iot_message_t;

/** AWS IoT message structure using Eclipse MQTTClient MQTT::Message. The structure of Message extracted from MQTTClient.h:
 *
 * @code
 * struct Message
 * {
 *    enum QoS qos;
 *    bool retained;
 *    bool dup;
 *    unsigned short id;
 *    void *payload;
 *    size_t payloadlen;
 * };
 * @endcode
 *
 * aws_message_t structure is not allocated internally in MQTT library.
 * The application callback should copy the content prior to return.
 * 
 * */
typedef struct MQTT::Message      aws_message_t;

/** AWS Greengrass discovery callback that will be invoked in response to discover API (@ref AWSIoTClient::discover) */
typedef void (*aws_greengrass_callback)( aws_greengrass_discovery_callback_data_t* cb_data);

/** AWS IoT client subscriber callback that will be invoked whenever a message is received for the subscribed topic */
typedef void (*subscriber_callback)( aws_iot_message_t& message);

/**
 * @}
 */

/**
 * @addtogroup aws_iot_macros
 *
 * AWS IoT and Greengrass macros
 * @{
 */
 /******************************************************
 *                    Macros
 ******************************************************/

/** Threshold yield timeout (in ms).
 * Recommended threshold timeout value is 1000ms in order to allow adequate time for the system to receive and decode data.
 * Recommended Yield timeout has been arrived at by considering different networks having different speed.
 */
#define THRESHOLD_YIELD_TIMEOUT 1000

/** Default command_timeout (in ms).
 * Wait time (in ms) before terminating the attempt to execute a command and generating an error.
 */
#define DEFAULT_COMMAND_TIMEOUT 5000

/** Maximum MQTT packet size including MQTT header and payload. */
#define AWS_MAX_PACKET_SIZE 100

/** Maximum number of message handlers.
 * AWS_MAX_MESSAGE_HANDLERS 5 - It means application can register 5 different callback functions for 5 different subscribed topics.
 */
#define AWS_MAX_MESSAGE_HANDLERS 5

/**
 * @}
 */

/*****************************************************************************/
/**
 *
 * @addtogroup aws_iot_classes
 *
 * Base class and communication functions for AWS IoT & AWS Greengrass services
 *
 *  @{
 */
/*****************************************************************************/
/** AWS IoT endpoint information class */
class AWSIoTEndpoint
{

private:
    aws_iot_transport_type_t transport; /**< AWS IoT transport for communication */
    const char* root_ca;                /**< TLS root certificate */
    uint16_t root_ca_length;            /**< Length of the TLS root certificate */
    TLSSocket* socket;                  /**< Pointer to MBED TLS socket object */
    char* uri;                          /**< AWS IoT endpoint URI */
    uint16_t port;                      /**< AWS IoT endpoint port number */

    friend class AWSIoTClient;          /**< AWSIoTClient can access private members of AWSIoTEndpoint */
};

/** AWS IoT client class */
class AWSIoTClient {

public:
    /** Default constructor of AWSIoTClient class
     * Initializes AWSIoTClient class members to default values
     */
    AWSIoTClient();

    /** Initializes AWSIoTClient class members
     *
     * @param[in] network             : Network interface (Wi-Fi, Ethernet etc ) defined by Mbed OS
     * @param[in] thing_name          : Name of the IoT thing
     * @param[in] private_key         : Private key of device/thing
     * @param[in] key_length          : Length of private key of device/thing
     * @param[in] certificate         : Certificate of device/thing
     * @param[in] certificate_length  : Length of certificate of device/thing
     *
     */
    AWSIoTClient ( NetworkInterface* network, const char* thing_name, const char* private_key, uint16_t key_length, const char* certificate,uint16_t certificate_length );

    /** Set command timeout for AWS IoT client library commands such as connect, publish, subscribe and unsubscribe.
     *  Default value is set to 5000ms.
     *  Use this API to change the timeout value. This API needs to be called before the MQTT connect operation.
     *
     * @param[in] command_timeout     : timeout in milliseconds
     *
     */
    void set_command_timeout( int command_timeout );

    /** Discovers Greengrass cores(groups) of which this 'Thing' is part of.
     *
     * @param[in] transport           : AWS transport to be used
     * @param[in] uri                 : URI of the AWS endpoint
     * @param[in] root_ca             : Root CA certificate
     * @param[in] root_ca_length      : Length of Root CA certificate
     * @param[in] gg_cb               : Greengrass discovery payload callback - Notifies application of Greengrass Cores information
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success,
     *                                  CY_RSLT_AWS_ERROR_INVALID_CLIENT_KEY, CY_RSLT_AWS_ERROR_INVALID_ROOTCA,
     *                                  CY_RSLT_AWS_ERROR_CONNECT_FAILED, CY_RSLT_AWS_ERROR_HTTP_FAILURE - On error ( @ref aws_iot_defines )
     *
     */
    cy_rslt_t discover( aws_iot_transport_type_t transport, const char* uri, const char* root_ca, uint16_t root_ca_length, aws_greengrass_callback gg_cb );


    /** Establishes connection to an AWS IoT or Greengrass core
     * This API is blocking and shall return when CONACK is received from server or timeout occurs
     *
     * @param[in] conn_params     : Connection parameters
     * @param[in] endpoint_params : AWS IoT Endpoint parameters
     *
     * @return cy_rslt_t          : CY_RSLT_SUCCESS - On success
     *                              CY_RSLT_AWS_ERROR_CONNECT_FAILED, CY_RSLT_AWS_ERROR_INVALID_ROOTCA,
     *                              CY_RSLT_AWS_ERROR_INVALID_CLIENT_KEY - On error ( @ref aws_iot_defines )
     *
     */
    cy_rslt_t connect( aws_connect_params_t conn_params,aws_endpoint_params_t endpoint_params);


    /** Publishes message to user defined topic on AWS cloud
     * This API is blocking and shall return when PUBACK is received from server or timeout occurs
     *
     *
     * @param[in] topic           : Contains the topic to which the message is to be published
     * @param[in] data            : Pointer to the message to be published
     * @param[in] length          : Length of the message pointed by 'message'
     * @param[in] pub_params      : Publish parameters
     *
     * @return cy_rslt_t          : CY_RSLT_SUCCESS - on success,
     *                              CY_RSLT_AWS_ERROR_PUBLISH_FAILED - On error ( @ref aws_iot_defines )
     *
     */
    cy_rslt_t publish( const char* topic, const char* data, uint32_t length, aws_publish_params_t pub_params );


    /** Subscribes to the user defined topic on AWS cloud 
     * This API is blocking and shall return when SUBACK is received from server or timeout occurs
     *
     *
     * @param[in] topic           : Contains the topic to be subscribed to
     * @param[in] qos             : QoS level to be used for receiving the message on the given topic
     * @param[in] cb              : Subscriber callback for the topic to receive the messages
     *
     * @return cy_rslt_t         : CY_RSLT_SUCCESS - on success
     *                             CY_RSLT_AWS_ERROR_SUBSCRIBE_FAILED - On error ( @ref aws_iot_defines )
     *
     */
    cy_rslt_t subscribe( const char* topic, aws_iot_qos_level_t qos, subscriber_callback cb );

    /** Unsubscribes from the topic that has been previously subscribed to on AWS cloud
     *
     * @param[in] topic           : Contains the topic to be unsubscribed from
     *
     * @return cy_rslt_t         : CY_RSLT_SUCCESS - on success
     *                             CY_RSLT_AWS_ERROR_UNSUBSCRIBE_FAILED - On error ( @ref aws_iot_defines )
     *
     */
    cy_rslt_t unsubscribe( char* topic );

    /** A call to this API must be made within the keepAlive interval to keep the MQTT connection alive.
     *  This API can be invoked if no other MQTT operation is needed. 
     *  This will also allow messages to be received.
     *
     *  AWSIoTClient is implemented on top of Eclipse MQTT library which are single threaded.
     *  Calling publish and yield simultaneously can lead to a crash as the underlying Eclipse Paho MQTT library APIs are not thread safe.
     *  Therefore, publish and subscribe should be handled by a single thread.
     *  For better efficiency, the application should typically be in yield, except when publishing.
     *
     *  For simultaneous publish and subscribe, Async mode communication should be supported in MQTT core library.
     *  But in current Eclipse MQTT library version only Sync mode is supported.
     *
     *  @param[in] timeout_ms     : Time to wait, in milliseconds. Recommend threshold timeout value 1000ms in order to allow adequate time for the system to receive and decode data.
     *                              Once Yield starts receiving data, it will not return even if timer(timeout_ms) expires.
     *                              It reads complete data and returns.
     *
     *  @return cy_rslt_t         : CY_RSLT_SUCCESS - on success
     *                              CY_RSLT_AWS_ERROR_INVALID_YIELD_TIMEOUT,CY_RSLT_AWS_ERROR_DISCONNECTED - On error ( @ref aws_iot_defines )
     */
    cy_rslt_t yield( unsigned long timeout_ms = 1000L );

    /** Disconnects from AWS IoT endpoint
     *
     *
     * @return cy_rslt_t         : CY_RSLT_SUCCESS - on success
     *                             CY_RSLT_AWS_ERROR_DISCONNECT_FAILED - On error ( @ref aws_iot_defines )
     *
     */
    cy_rslt_t disconnect();


private:
    NetworkInterface* network;
    const char* thing_name;
    const char* private_key;
    uint16_t key_length;
    const char* certificate;
    uint16_t certificate_length;
    int command_timeout;
    MQTT::Client<MQTTNetwork, Countdown, AWS_MAX_PACKET_SIZE, AWS_MAX_MESSAGE_HANDLERS> *mqtt_obj;
    MQTTNetwork *mqttnetwork;
    mqtt_security_flag flag;
    AWSIoTEndpoint *ep;

    /** Creates endpoint instance using the information provided to connect to server.
     *
     * @param[in] transport           : AWS transport to be used
     * @param[in] uri                 : URI of the AWS endpoint
     * @param[in] port                : Port of AWS endpoint
     * @param[in] root_ca             : Root CA certificate
     * @param[in] root_ca_length      : Length of Root CA certificate
     *
     */
    AWSIoTEndpoint* create_endpoint( aws_iot_transport_type_t transport, const char* uri, int port, const char* root_ca, uint16_t root_ca_length);

    /** Frees endpoint instance which is earlier used to connect to server.
     *
     * @param[in] ep                  : AWS endpoint instance
     *
     */
    void free_endpoint(AWSIoTEndpoint* ep);

};

/**
 * @}
 */

#endif
