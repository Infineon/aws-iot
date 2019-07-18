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
 *  Home for common defines & configuration for Wiced AWS
 */
#pragma once

#include "aws_error.h"
#include "JSON.h"
#include "linked_list.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define AWS_MAX_CONNECTIONS                   (2)
#define AWS_MQTT_KEEP_ALIVE_TIMEOUT           (5)         //in seconds
#define AWS_DEFAULT_CONNECTION_RETRIES        (3)
#define AWS_DEFAULT_DNS_TIMEOUT               (10000)     // milliseconds
#define AWS_REQUEST_TIMEOUT                   (5000)      // milliseconds
#define AWS_IOT_DEFAULT_MQTT_PORT             (8883)

#define GREENGRASS_DISCOVERY_HTTP_REQUEST_URI_PREFIX  "/greengrass/discover/thing/"
#define AWS_GG_HTTPS_CONNECT_TIMEOUT          (2000)
#define AWS_GREENGRASS_DISCOVERY_TIMEOUT      (5000)

#define AWS_GG_METADATA_MAX_LENGTH            (128)
#define AWS_GG_THING_ARN_MAX_LENGTH           (128)
#define AWS_GG_GROUP_ID_MAX_LENGTH            (64)
#define AWS_GG_ROOT_CA_MAX_LENGTH             (2000)
#define AWS_GG_MAX_CONNECTIONS                (1)

#define AWS_GG_HTTPS_SERVER_PORT              (8443)

#define GG_GROUP_ID                                 "GGGroupId"
#define GG_GROUP_KEY                                "GGGroups"
#define GG_HOST_ADDRESS                             "HostAddress"
#define GG_PORT                                     "PortNumber"
#define GG_METADATA                                 "Metadata"
#define GG_ROOT_CAS                                 "CAs"
#define GG_CORE_THING_ARN                           "thingArn"
#define GG_BEGIN_CERTIFICATE                        "-----BEGIN CERTIFICATE-----"
#define GG_REQUEST_PROTOCOL                         "https://"

#define verify_aws_type(aws, type)   ( (aws->transport == type) ? 1 : 0 )

#define AWS_LIBRARY_INFO( x )   printf x
#define AWS_LIBRARY_DEBUG( x )  //printf x
#define AWS_LIBRARY_ERROR( x )  //printf x

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
/**
 * List of AWS IoT protocols
 */

typedef enum
{
    AWS_TRANSPORT_MQTT_NATIVE = 0,                /**< MQTT-native i.e. MQTT over TCP sockets */
    AWS_TRANSPORT_MQTT_WEBSOCKET,                 /**< MQTT over Websockets */
    AWS_TRANSPORT_RESTFUL_HTTPS,                  /**< AWS RESTful HTTPS APIs */
    AWS_TRANSPORT_INVALID,                        /**< Invalid transport type */
} aws_iot_transport_type_t;

/**
 * QoS types for publish/subscribe messages over MQTT
 */
typedef enum
{
    AWS_QOS_ATMOST_ONCE    = 0x00,                /**< QoS level 0 */
    AWS_QOS_ATLEAST_ONCE   = 0x01,                /**< QoS level 1 */
    AWS_QOS_EXACTLY_ONCE   = 0x02,                /**< QoS level 2 */
    AWS_QOS_INVALID        = 0x80,                /**< Invalid QoS level */
} aws_iot_qos_level_t;

/**
 * List of AWS events
 */
typedef enum
{
    AWS_EVENT_CONNECTED,                          /**< The connection has been accepted by remote */
    AWS_EVENT_DISCONNECTED,                       /**< Disconnection received from peer or some other network failure */
    AWS_EVENT_PUBLISHED ,                         /**< Publication event; Either accepted by remote( received an acknowledgment ) or timed-out */
    AWS_EVENT_SUBSCRIBED,                         /**< Subscription event; Either acknowledged by remote or timed-out */
    AWS_EVENT_UNSUBSCRIBED,                       /**< Unsubscription event; acknowledged by remote or timed-out */
    AWS_EVENT_PAYLOAD_RECEIVED,                   /**< Data received event */
} aws_iot_event_type_t;


 /******************************************************
 *                    Structures
 ******************************************************/

/**
 *  Connection endpoint information of a Greengrass core device. A greengrass core can have one or more endpoints.
 */
typedef struct
{
    char*                                       metadata;               /**< Any metadata associated with this Connection - Example - Interface name etc. */
    char*                                       ip_address;             /**< IP-Address of the Core */
    char*                                       port;                   /**< Port number of the Core */
} aws_greengrass_core_connection_info_t;

/**
 * Node Wrapper structure for Connection Information of a greengrass core.
 */
typedef struct
{
    aws_greengrass_core_connection_info_t info;
    cy_linked_list_node_t                 node;
} aws_greengrass_core_connection_t;

/**
 * Greengrass Core Information retrieved from AWS. A device can be part of multiple groups and can select
 * to which Group's core it wants to connect to.
 * Note: Each group have only one core.
 */
typedef struct
{
    char*                                       group_id;               /**< Group-ID */
    char*                                       thing_arn;              /**< Amazon resource name of the'Core' device of this Group */
    char*                                       root_ca_certificate;    /**< Root CA certificate for this 'Core' */
    uint16_t                                    root_ca_length;         /**< Length of the certificate */
    cy_linked_list_t                            connections;            /**< A linked-list to store all Connection endpoints( @ref wiced_aws_greengrass_core_connection_t ) available for this core.For example: A core can have multiple network Interfaces. */
} aws_greengrass_core_info_t;

/**
 * Node Wrapper structure for a greengrass core Information
 */

typedef struct
{
    aws_greengrass_core_info_t            info;
    cy_linked_list_node_t                 node;
} aws_greengrass_core_t;

/** @} */

/**
 * @addtogroup wiced_aws Greengrass
 *
 * Wiced Amazon Web services library - A one-stop library for communicating with AWS services( AWS IoT or AWS Greengrass ).
 *
 * Discovery callback returns a list of all Groups the thing is part of along with Connection Informations of the cores of these groups.
 * Application upon receiving this information can select which group/core it wants to connect to.
 *
 * Note that each Greengrass group can have only one Greengrass core.
 * However, each greengrass core can have one or more Connections endpoints.
 *                             Greengrass Discovery Payload architecture
 *                             =========================================
 *
 *
 *
 *             groups
 *               +
 *               |
 *               |     +--------+---------+-------+-----------+----+      +--------+---------+-------+-----------+----+
 *               |     |        |         |       |           |    |      |        |         |       |           |    |
 *               +---->|Group-ID|Thing ARN|Root CA|connections|next+----->|Group-ID|Thing ARN|Root CA|connections|next+----->NULL
 *                     |        |         |       |           |    |      |        |         |       |           |    |
 *                     +--------+---------+-------+----+------+----+      +--------+---------+-------+----+------+----+
 *                                                     |
 *                                                     |
 *                                                     |
 *                                                     v
 *                                                   +----------+-----------+--------+----+      +----------+-----------+--------+----+
 *                                                   |          |           |        |    |      |          |           |        |    |
 *                                                   |IP-address|Port-Number|Metadata|next+----->|IP-address|Port-Number|Metadata|next+----->NULL
 *                                                   |          |           |        |    |      |          |           |        |    |
 *                                                   +----------+-----------+--------+----+      +----------+-----------+--------+----+
 *
 *  @{
 */

typedef struct
{
    cy_linked_list_t* groups;
} aws_greengrass_discovery_callback_data_t;

typedef struct
{
    cy_rslt_t status;
} aws_iot_callback_connection_data_t;

typedef struct
{
    cy_rslt_t status;
} aws_iot_callback_disconnection_data_t;

typedef struct
{
    cy_rslt_t status;
} aws_iot_callback_publish_data_t;

typedef struct
{
    cy_rslt_t status;
} aws_iot_callback_subscribe_data_t;

typedef struct
{
    cy_rslt_t status;
} aws_iot_callback_unsubscribe_data_t;

typedef struct
{
    cy_rslt_t                                   status;
    uint8_t*                                    data;
    uint32_t                                    data_length;
    uint8_t*                                    topic;
    uint32_t                                    topic_length;
} aws_iot_callback_message_t;

typedef union
{
    aws_iot_callback_connection_data_t        connection;
    aws_iot_callback_disconnection_data_t     disconnection;
    aws_iot_callback_publish_data_t           publish;
    aws_iot_callback_subscribe_data_t         subscribe;
    aws_iot_callback_unsubscribe_data_t       unsubscribe;
    aws_iot_callback_message_t                message;
} wiced_aws_callback_data_t;

typedef struct aws_connect_params_s
{
    uint16_t    keep_alive;                                     /* Indicates keep alive interval to Broker */
    uint8_t     clean_session;                                  /* Indicates if the session to be cleanly started */
    uint8_t*    username;                                       /* User name to connect to Broker */
    uint8_t*    password;                                       /* Password to connect to Broker */
    uint8_t*    alpn_string;                                    /* If trying to connect with port 443 to AWS, It is mandatory to pass ALPN extension */
    uint8_t*    peer_cn;
    uint8_t*    client_id;                                      /* Application must pass the client ID information */
} aws_connect_params;

typedef struct aws_publish_params_s
{
    aws_iot_qos_level_t QoS;
} aws_publish_params;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
cy_rslt_t json_callback_for_discovery_payload (cy_JSON_object_t* json_object );

#ifdef __cplusplus
} /*extern "C" */
#endif
