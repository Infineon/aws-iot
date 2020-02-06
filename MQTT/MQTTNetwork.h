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
/** file
 *
 * MQTT wrapper for MQTT client library
 */
#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "mbed.h"

#define MQTT_NETWORK_DEBUG( x )  //printf x
#define MQTT_NETWORK_ERROR( x )  printf x
#define MQTT_NETWORK_INFO( x )   printf x


typedef enum {
    SECURED_MQTT,
    NON_SECURED_MQTT
} mqtt_security_flag;


class MQTTNetwork {
public:
    MQTTNetwork(NetworkInterface* aNetwork, mqtt_security_flag is_security =
            NON_SECURED_MQTT) :
            network(aNetwork) {
        is_security_enabled = is_security;

        is_security_enabled = is_security;

        if (is_security_enabled == SECURED_MQTT) {
            TLSSocket *socket;
            socket = new TLSSocket;
            socket_context = socket;

        } else {
            TCPSocket *socket;
            socket = new TCPSocket;
            socket_context = socket;
        }
    }

    ~MQTTNetwork() {

        if (is_security_enabled == SECURED_MQTT) {
            TLSSocket *socket;

            socket = (TLSSocket *) socket_context;
            delete socket;

        } else {
            TCPSocket *socket;

            socket = (TCPSocket *) socket_context;
            delete socket;

        }

    }

    int read(unsigned char* buffer, int len, int timeout) {
        int bytes_read = 0;
        int total_bytes = len;
        int ret = 0;
        Timer timer;
        if (is_security_enabled == SECURED_MQTT) {
            TLSSocket *socket;

            socket = (TLSSocket *) socket_context;

            socket->set_timeout(timeout);
            /* Consider negative timeout value as blocking call and wait till all the expected bytes available and then return. If timeout is positive value then wait for the timeout to get the expected data */
            if ( timeout < 0 )
            {
                do {
                    ret = socket->recv(buffer + bytes_read,
                            total_bytes - bytes_read);

                    if (ret < 0) {
                        if (ret != NSAPI_ERROR_WOULD_BLOCK) {
                            MQTT_NETWORK_ERROR((" Socket receive error : %d \n", ret));
                            return -1;
                        }
                    } else {
                        bytes_read += ret;
                    }

                } while (bytes_read < total_bytes);

                return bytes_read;
            }
            else {
                timer.reset();
                timer.start();

                do {

                    ret = socket->recv(buffer + bytes_read,
                            total_bytes - bytes_read);

                    if (ret < 0) {
                        if (ret != NSAPI_ERROR_WOULD_BLOCK) {
                            MQTT_NETWORK_ERROR((" Socket receive error : %d \n", ret));
                            timer.stop();
                            return -1;
                        }
                    } else {
                        bytes_read += ret;
                    }
                } while (bytes_read < total_bytes && timer.read_ms() < timeout);

                timer.stop();

                return bytes_read;
            }

        } else {
            TCPSocket *socket;

            socket = (TCPSocket *) socket_context;
            do {
                ret = socket->recv(buffer + bytes_read,
                        total_bytes - bytes_read);
                if (ret < 0)
                    return -1;
                bytes_read += ret;
            } while (bytes_read < total_bytes);
            return bytes_read;
        }

    }


    int write(unsigned char* buffer, int len, int timeout) {

        if (is_security_enabled == SECURED_MQTT) {
            TLSSocket *socket;

            socket = (TLSSocket *) socket_context;
            return socket->send(buffer, len);

        } else {
            TCPSocket *socket;

            socket = (TCPSocket *) socket_context;
            return socket->send(buffer, len);
        }

    }

    int set_root_ca_certificate(const char* root_ca_certifcate) {
        TLSSocket *socket = NULL;
        socket = (TLSSocket *) socket_context;

        if (root_ca_certifcate == NULL)
        {
            MQTT_NETWORK_INFO(("[MQTT INFO] : ROOT CA CERTIFICATE IS IGNORED\r\n"));
            return 0;
        }
        return socket->set_root_ca_cert(root_ca_certifcate);
    }

    int set_client_cert_key(const char* client_cert, const char* client_key) {
        TLSSocket *socket = NULL;
        socket = (TLSSocket *) socket_context;
        if (client_cert == NULL || client_key == NULL) {
            MQTT_NETWORK_ERROR(("[MQTT ERROR] : PASS VALID client certificate and client private key\r\n"));
            return -1;
        }
        return socket->set_client_cert_key(client_cert, client_key);
    }

    int connect(const char* hostname, int port, const char* peer_cn) {

        if (is_security_enabled == SECURED_MQTT) {
            TLSSocket *socket;
            nsapi_error_t rc = NSAPI_ERROR_OK;

            socket = (TLSSocket *) socket_context;

            rc = socket->open(network);
            if (rc != NSAPI_ERROR_OK) {
                MQTT_NETWORK_ERROR(
                        ("[MQTT ERROR] : TLS SOCKET OPEN FAILED\r\n"));
               return ((int)rc);
            }

            MQTT_NETWORK_DEBUG(("[MQTT INFO] : hostname set : %s \n", peer_cn ));
            socket->set_hostname(peer_cn);

            rc = network->gethostbyname(hostname, &address, NSAPI_UNSPEC, NULL);
            if (rc != NSAPI_ERROR_OK) {
                MQTT_NETWORK_ERROR(
                        ("[MQTT ERROR] : GET HOST BY NAME FAILED\r\n"));
                return ((int)rc);
            }
            address.set_port(port);

            return socket->connect(address);

        } else {
            TCPSocket *socket;
            nsapi_error_t rc = NSAPI_ERROR_OK;
            socket = (TCPSocket *) socket_context;

            rc = socket->open(network);
            if (rc != NSAPI_ERROR_OK) {
                MQTT_NETWORK_ERROR(
                        ("[MQTT ERROR] :  TCP SOCKET OPEN FAILED\r\n"));
                return ((int)rc);
            }

            rc = network->gethostbyname(hostname, &address, NSAPI_UNSPEC, NULL);
            if (rc != NSAPI_ERROR_OK) {
                MQTT_NETWORK_ERROR(
                        ("[MQTT ERROR] : GET HOST BY NAME FAILED\r\n"));
                return ((int)rc);
            }
            address.set_port(port);

            return socket->connect(address);
        }

    }

    int disconnect() {

        int ret;
        if (is_security_enabled == SECURED_MQTT) {
            TLSSocket *socket;

            socket = (TLSSocket *) socket_context;
            ret = socket->close();
            delete socket;
            socket = NULL;
            socket_context = NULL;
            return ret;

        } else {
            TCPSocket *socket;

            socket = (TCPSocket *) socket_context;
            ret = socket->close();
            delete socket;
            socket = NULL;
            socket_context = NULL;
            return ret;
        }

    }

private:
    NetworkInterface* network;
    void* socket_context;
    mqtt_security_flag is_security_enabled;
    SocketAddress address;
};

#endif // _MQTTNETWORK_H_
