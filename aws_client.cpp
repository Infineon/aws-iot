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
 *
 * Implementation for AWS IoT client library
 *
 */
#include "aws_client.h"
#include "https_request.h"

static aws_greengrass_discovery_callback_data_t discovery_data;
extern cy_linked_list_t* group_list;

AWSIoTClient::AWSIoTClient()
{
    /* Assign thing name and credentials to AWS client members */
    AWSIoTClient::thing_name = NULL;
    AWSIoTClient::private_key = NULL;
    AWSIoTClient::key_length = 0;
    AWSIoTClient::certificate = NULL;
    AWSIoTClient::certificate_length = 0;

    AWSIoTClient::command_timeout = DEFAULT_COMMAND_TIMEOUT;
    AWSIoTClient::network = NULL;
    AWSIoTClient::flag = SECURED_MQTT;
    AWSIoTClient::mqttnetwork = NULL;
    AWSIoTClient::mqtt_obj = NULL;
    AWSIoTClient::ep = NULL;
};

AWSIoTClient::AWSIoTClient(NetworkInterface* network, const char* thing_name, const char* private_key, uint16_t key_length, const char* certificate, uint16_t certificate_length)
{
    /* Assign thing name and credentials to AWS client members */
    AWSIoTClient::thing_name = thing_name;
    AWSIoTClient::private_key = private_key;
    AWSIoTClient::key_length = key_length;
    AWSIoTClient::certificate = certificate;
    AWSIoTClient::certificate_length = certificate_length;

    AWSIoTClient::command_timeout = DEFAULT_COMMAND_TIMEOUT;
    AWSIoTClient::network = network;
    AWSIoTClient::flag = SECURED_MQTT;
    AWSIoTClient::mqttnetwork = NULL;
    AWSIoTClient::mqtt_obj = NULL;
    AWSIoTClient::ep = NULL;
}

void AWSIoTClient::set_command_timeout( int command_timeout )
{
    AWSIoTClient::command_timeout = command_timeout;
}

AWSIoTEndpoint* AWSIoTClient::create_endpoint(aws_iot_transport_type_t transport, const char* uri, int port, const char* root_ca, uint16_t root_ca_length)
{
    AWSIoTEndpoint* ep = NULL;

    if(uri == NULL || root_ca == NULL || root_ca_length == 0) {
        AWS_LIBRARY_ERROR (("Invalid End point parameters\n"));
        goto exit;
    }

    switch (transport)
    {
        case AWS_TRANSPORT_MQTT_NATIVE:
        {
            break;
        }
        case AWS_TRANSPORT_RESTFUL_HTTPS:
        case AWS_TRANSPORT_INVALID:
        default:
        {
            goto exit;
        }
    }

    ep = new AWSIoTEndpoint();
    ep->root_ca = root_ca;
    ep->root_ca_length = root_ca_length;
    ep->transport = transport;
    ep->uri = (char*) uri;
    ep->port = port;

exit:
    return ep;
}

void AWSIoTClient::free_endpoint(AWSIoTEndpoint* ep)
{
    if(ep!=NULL) {
        delete ep;
        ep = NULL;
        AWS_LIBRARY_DEBUG (("AWSIoTEndpoint Freed \n"));
    } else {
        AWS_LIBRARY_DEBUG (("AWSIoTEndpoint is not valid \n"));
    }
}

cy_rslt_t AWSIoTClient::connect(aws_connect_params_t conn_params,aws_endpoint_params_t endpoint_params)
{
    int rc = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    ep = create_endpoint(endpoint_params.transport, endpoint_params.uri, endpoint_params.port, endpoint_params.root_ca, endpoint_params.root_ca_length);
    if (ep == NULL) {
        AWS_LIBRARY_ERROR (("Error in creating endpoint\n"));
        result = CY_RSLT_AWS_ERROR_CONNECT_FAILED;
        goto exit;
    }

    mqttnetwork = new MQTTNetwork(AWSIoTClient::network, flag);
    if (mqttnetwork == NULL) {
        result = CY_RSLT_AWS_ERROR_CONNECT_FAILED;
        goto exit;
    }

    rc = mqttnetwork->set_root_ca_certificate(ep->root_ca);
    if (rc != 0) {
        AWS_LIBRARY_ERROR (("Error in setting root CA certificate \n"));
        result = CY_RSLT_AWS_ERROR_INVALID_ROOTCA;
        goto exit;
    }

    rc = mqttnetwork->set_client_cert_key(AWSIoTClient::certificate,
            AWSIoTClient::private_key);
    if (rc != 0) {
        AWS_LIBRARY_ERROR (("Error in setting client certificate and private key\n"));
        result = CY_RSLT_AWS_ERROR_INVALID_CLIENT_KEY;
        goto exit;
    }

    rc = mqttnetwork->connect(ep->uri, ep->port, (char*) conn_params.peer_cn);
    if (rc != 0) {
        AWS_LIBRARY_ERROR (("TLS connection to MQTT broker failed \n"));
        result = CY_RSLT_AWS_ERROR_CONNECT_FAILED;
        goto exit;
    } else {
        AWS_LIBRARY_DEBUG(("TLS connection to AWS endpoint established \n"));
        mqtt_obj = new MQTT::Client<MQTTNetwork, Countdown, AWS_MAX_PACKET_SIZE, AWS_MAX_MESSAGE_HANDLERS>(*mqttnetwork,
                AWSIoTClient::command_timeout);

        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        data.MQTTVersion = 4;
        data.clientID.cstring = (char*) conn_params.client_id;
        data.username.cstring = (char*) conn_params.username;
        data.password.cstring = (char*) conn_params.password;
        data.keepAliveInterval = conn_params.keep_alive;

        AWS_LIBRARY_DEBUG(("Send MQTT connect frame \n"));
        if ((rc = mqtt_obj->connect(data)) != 0) {
            AWS_LIBRARY_ERROR(("MQTT connect failed : %d\r\n", rc));
            delete mqtt_obj;
            mqtt_obj = NULL;
            result = CY_RSLT_AWS_ERROR_CONNECT_FAILED;
            goto exit;
        } else {
            AWS_LIBRARY_DEBUG(("MQTT connect is successful %d\r\n", rc));
        }
        return CY_RSLT_SUCCESS;
    }

exit:
    if(mqttnetwork != NULL) {
        delete mqttnetwork;
        mqttnetwork = NULL;
    }
    if(ep != NULL) {
        free_endpoint(ep);
        ep = NULL;
    }
    return result;
}

cy_rslt_t AWSIoTClient::disconnect()
{
    int rc = 0;

    if( mqtt_obj == NULL ) {
        AWS_LIBRARY_ERROR(("Device not connected to MQTT broker \n"));
        return CY_RSLT_AWS_ERROR_DISCONNECT_FAILED;
    }

    AWS_LIBRARY_DEBUG(("Send MQTT dis-connect frame \n"));
    rc = mqtt_obj->disconnect();
    if (rc != 0) {
        AWS_LIBRARY_ERROR(("MQTT dis-connect failed : %d\r\n", rc));
        return CY_RSLT_AWS_ERROR_DISCONNECT_FAILED;
    } else {
        AWS_LIBRARY_DEBUG(("MQTT dis-connect is successful %d\r\n", rc));
    }

    delete mqtt_obj;
    mqtt_obj = NULL;
    mqttnetwork->disconnect();

    delete mqttnetwork;
    mqttnetwork = NULL;

    if(ep != NULL) {
        free_endpoint(AWSIoTClient::ep);
        ep = NULL;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t AWSIoTClient::publish(const char* topic, const char* data, uint32_t length, aws_publish_params_t pub_params )
{
    int rc = 0;

    MQTT::Message message;
    message.qos = (MQTT::QoS) pub_params.QoS;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)data;
    message.payloadlen = length;

    if( pub_params.QoS != AWS_QOS_ATMOST_ONCE && pub_params.QoS != AWS_QOS_ATLEAST_ONCE ) {
        AWS_LIBRARY_ERROR(("QoS value not supported\n"));
        return CY_RSLT_AWS_ERROR_PUBLISH_FAILED;
    }

    if( mqtt_obj == NULL ) {
        AWS_LIBRARY_ERROR(("Device not connected to MQTT broker \n"));
        return CY_RSLT_AWS_ERROR_PUBLISH_FAILED;
    }

    rc = mqtt_obj->publish(topic, message);
    if ( rc != 0 ) {
        AWS_LIBRARY_ERROR(("Publish to AWS endpoint failed  : %d \n", rc ));
        return CY_RSLT_AWS_ERROR_PUBLISH_FAILED;
    }

    AWS_LIBRARY_DEBUG(("Published to AWS endpoint successfully \n"));

    return CY_RSLT_SUCCESS;
}

cy_rslt_t AWSIoTClient::subscribe(const char* topic, aws_iot_qos_level_t qos, subscriber_callback cb)
{
    int rc = 0;

    if( mqtt_obj == NULL ) {
        AWS_LIBRARY_ERROR(("Device not connected to MQTT broker \n"));
        return CY_RSLT_AWS_ERROR_SUBSCRIBE_FAILED;
    }

    rc = mqtt_obj->subscribe(topic, (MQTT::QoS)qos, cb);
    if (rc != 0) {
        AWS_LIBRARY_ERROR(("MQTT subscribe failed %d\r\n", rc));
        return CY_RSLT_AWS_ERROR_SUBSCRIBE_FAILED;
    } else {
        AWS_LIBRARY_DEBUG(("MQTT subscribtion successful %d\r\n", rc));
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t AWSIoTClient::unsubscribe(char* topic )
{
    int rc = 0;

    if( mqtt_obj == NULL ) {
        AWS_LIBRARY_ERROR(("Device not connected to MQTT broker \n"));
        return CY_RSLT_AWS_ERROR_UNSUBSCRIBE_FAILED;
    }

    rc = mqtt_obj->unsubscribe(topic);
    if (rc != 0) {
        AWS_LIBRARY_ERROR(("MQTT unsubscribe failed %d\r\n", rc));
        return CY_RSLT_AWS_ERROR_UNSUBSCRIBE_FAILED;
    } else {
        AWS_LIBRARY_DEBUG(("MQTT unsubscribe successful %d\r\n", rc));
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t AWSIoTClient::yield(unsigned long timeout_ms)
{
    int rc = 0;

    if ( timeout_ms < THRESHOLD_YIELD_TIMEOUT ) {
        AWS_LIBRARY_INFO(("Recommend threshold timeout value 1000ms in order to allow adequate time for the system to receive and decode data \n"));
        return CY_RSLT_AWS_ERROR_INVALID_YIELD_TIMEOUT;
    }

    if( mqtt_obj == NULL ) {
        AWS_LIBRARY_ERROR(("Device not connected to MQTT broker \n"));
        return CY_RSLT_AWS_ERROR_DISCONNECTED;
    }

    rc = mqtt_obj->yield( timeout_ms );
    if( rc == -1 ) {
        /* Send disconnect frame to broker */
        mqtt_obj->disconnect();

        delete mqtt_obj;
        mqtt_obj = NULL;
        mqttnetwork->disconnect();

        delete mqttnetwork;
        mqttnetwork = NULL;

        return CY_RSLT_AWS_ERROR_DISCONNECTED;
    }

    return CY_RSLT_SUCCESS;
}

void dump_response(HttpResponse* res)
{
    AWS_LIBRARY_DEBUG(("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str()));

    AWS_LIBRARY_DEBUG(("Headers:\n"));
    for (size_t ix = 0; ix < res->get_headers_length(); ix++)
    {
        AWS_LIBRARY_DEBUG(("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str()));
    }

    AWS_LIBRARY_DEBUG(("\nBody (%lu bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str()));

}

cy_rslt_t AWSIoTClient::discover(aws_iot_transport_type_t transport, const char* uri, const char* root_ca, uint16_t root_ca_length, aws_greengrass_callback gg_cb)
{
    nsapi_error_t result;
    TLSSocket* socket = new TLSSocket();
    SocketAddress address;
    char* discovery_uri = NULL;

    result = socket->set_client_cert_key(AWSIoTClient::certificate, AWSIoTClient::private_key);
    if (result != 0) {
        AWS_LIBRARY_ERROR((" Error in initializing client certificate and key : %d \n", result));
        return CY_RSLT_AWS_ERROR_INVALID_CLIENT_KEY;
    }

    /* Initialize RootCA certificate */
    result = socket->set_root_ca_cert((const char*) root_ca);
    if (result != 0) {
        printf (" Error in initializing rootCA certificate \n");
        return CY_RSLT_AWS_ERROR_INVALID_ROOTCA;
    }

    /* Resolve hostname address */
    result = network->gethostbyname(uri, &address);

    AWS_LIBRARY_INFO((" IP address of server : %s \n", address.get_ip_address()));

    /* set GreenGrass port */
    address.set_port( AWS_GG_HTTPS_SERVER_PORT);

    result = socket->open(AWSIoTClient::network);

    /* set peer common name */
    socket->set_hostname(uri);

    socket->set_timeout(5000);

    result = socket->connect(address);
    if (result != 0) {
        AWS_LIBRARY_ERROR((" TLS connection to server failed : %d \n", result));
        return CY_RSLT_AWS_ERROR_CONNECT_FAILED;
    }

    AWS_LIBRARY_DEBUG((" TLS connection to server established. Connected to server \n"));

    /* Register callback for AWS discovery payload */
    cy_JSON_parser_register_callback( json_callback_for_discovery_payload );

    /* form discovery URI to send to AWS cloud */
    uint8_t discovery_uri_length = (strlen(thing_name) + strlen(GG_REQUEST_PROTOCOL) + strlen(uri) + strlen(GREENGRASS_DISCOVERY_HTTP_REQUEST_URI_PREFIX) + 1 );

    discovery_uri = (char *) malloc( discovery_uri_length);
    memset( discovery_uri, 0, discovery_uri_length);

    /* example : https://a38td4ke8seeky-ats.iot.us-east-1.amazonaws.com/greengrass/discover/thing/wiced_ggd_1 */
    strcat( discovery_uri, GG_REQUEST_PROTOCOL );
    strcat( discovery_uri, uri );
    strcat( discovery_uri, GREENGRASS_DISCOVERY_HTTP_REQUEST_URI_PREFIX );
    strcat( discovery_uri, thing_name );

    AWS_LIBRARY_DEBUG(("[AWS-Greengrass] Discovery URI is: %s (len:%d)\n", discovery_uri, (int) strlen(discovery_uri)));

    HttpsRequest* get_req = new HttpsRequest(socket, HTTP_GET, discovery_uri);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
        AWS_LIBRARY_ERROR(("HttpRequest failed (error code %d)\n", get_req->get_error()));
        return CY_RSLT_AWS_ERROR_HTTP_FAILURE;
    }

    AWS_LIBRARY_DEBUG(("\n----- HTTPS GET response -----\n"));
    dump_response(get_res);

    result = cy_JSON_parser( (const char*)get_res->get_body_as_string().c_str(), (uint32_t) get_res->get_body_length() );
    if( result != CY_RSLT_SUCCESS ) {
        AWS_LIBRARY_ERROR(("[AWS-Greengrass] JSON parser error\n"));
        return CY_RSLT_AWS_ERROR_HTTP_FAILURE;
    }

    discovery_data.groups = group_list;

    if( gg_cb ) {
        gg_cb( &discovery_data );
    }

    delete get_req;
    delete socket;
    free(discovery_uri);
    return CY_RSLT_SUCCESS;
}
