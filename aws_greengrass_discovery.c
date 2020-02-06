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
 * Implementation for AWS Greengrass discovery
 *
 */

#include "aws_common.h"
#include "cy_result.h"
#include "linked_list.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define WICED_AWS_GG_DISCOVERY_PAYLOAD_LENGTH  (6000)

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

static uint8_t json_object_counter = 0;
static uint8_t gg_group_found;
cy_linked_list_t* group_list = NULL;
/******************************************************
 *               Function Definitions
 ******************************************************/

#ifdef AWS_GG_DEBUG

static void dump_connection( wiced_aws_greengrass_core_connection_t* connection )
{
    wiced_aws_greengrass_core_connection_info_t* info = NULL;

    if( !connection )
        return;

    info = &connection->info;

    WPRINT_APP_INFO(("\t ==== Core's Connection Information ====\n"));
    WPRINT_APP_INFO(("\t %s: %s\n", GG_HOST_ADDRESS, info->ip_address));
    WPRINT_APP_INFO(("\t %s: %s\n", GG_PORT, info->port));
    WPRINT_APP_INFO(("\t %s: %s\n", GG_METADATA, info->metadata));
    WPRINT_APP_INFO(("\t ==== End of Connection Information ====\n"));
}

static void dump_core( wiced_aws_greengrass_core_t* core )
{
    uint32_t i = 0;
    uint32_t count;
    wiced_aws_greengrass_core_info_t* info = NULL;
    cy_linked_list_t* connections = NULL;
    cy_linked_list_node_t* node = NULL;

    if( !core )
    {
        return;
    }

    info = &core->info;
    WPRINT_APP_INFO((" ==== Core/Group Information ====\n"));
    WPRINT_APP_INFO(("%s: %s\n", GG_GROUP_ID, info->group_id));
    WPRINT_APP_INFO(("%s: %s\n", GG_CORE_THING_ARN, info->thing_arn));
    WPRINT_APP_INFO(("%s: %s\n", GG_ROOT_CAS, info->root_ca_certificate));
    WPRINT_APP_INFO((" ==== End of Core/Group Information ====\n"));
    connections = &info->connections;
    count = connections->count;
    for( i = 0; i < count; i++ )
    {
        cy_linked_list_get_front_node( connections, &node);
        if( node )
        {
            dump_connection( node->data );
        }
    }
}

static void dump_group_list(void)
{
    uint32_t i = 0;
    cy_linked_list_node_t* node = NULL;
    uint32_t count = group_list->count;

    for( i = 0; i < count; i++ )
    {
        cy_linked_list_get_front_node(group_list, &node);
        if( node )
        {
            dump_core( node->data );
        }
    }
}

#endif



static int greengrass_initialize_group_list(void)
{
    group_list = malloc( sizeof(cy_linked_list_t) );
    if( !group_list )
        return 0;
    cy_linked_list_init(group_list);
    return 1;
}

/* Each Group has only one Core; But One core may have many connection endpoints */
static void greengrass_initialize_core_node( char* group_id, uint16_t length )
{
    aws_greengrass_core_info_t* info = NULL;
    aws_greengrass_core_t* core = NULL;

    if( !group_id || !length )
        return;

    core = malloc( sizeof(aws_greengrass_core_t) );
    if( !core )
    {
        return;
    }

    info = &core->info;

    cy_linked_list_init(&info->connections);

    info->group_id = malloc((size_t)length + 1);
    strncpy( info->group_id, group_id, length );
    info->group_id[length] = '\0';

    cy_linked_list_set_node_data( &core->node, (void *)core );

    cy_linked_list_insert_node_at_rear(group_list, &core->node);
    return;
}

static void greengrass_add_core_root_ca( char* root_ca, uint16_t length )
{
    char* src;
    char* dst;
    int i = 0;
    int count = 0;

    cy_linked_list_node_t* node = NULL;
    aws_greengrass_core_t* core = NULL;
    aws_greengrass_core_info_t* info = NULL;

    cy_linked_list_get_rear_node(group_list, &node);
    if( !node )
    {
        return;
    }

    core = (aws_greengrass_core_t*)node->data;
    info = &core->info;

    info->root_ca_certificate = malloc((size_t)length + 1);

    /* Remove '\' 'n' characters which is added by AWS in the root CA cert. */
    src = root_ca;
    dst = info->root_ca_certificate;
    for (i = 0; i < length; i++)
    {
        if ( ('\\' == *src) && ('n' == *(src + 1)) )
        {
            src += 2;
            *dst++ = '\n';
            i++;
            count++;
            continue;
        }
        *dst++ = *src++;
    }

    info->root_ca_certificate[length-count] = '\0';
    return;
}

static void greengrass_add_core_thing_arn( char* thing_arn, uint16_t length )
{
    cy_linked_list_node_t* node = NULL;
    aws_greengrass_core_t* core = NULL;
    aws_greengrass_core_info_t* info = NULL;

    cy_linked_list_get_rear_node(group_list, &node);
    if( !node )
    {
        return;
    }

    core = node->data;
    info = &core->info;

    info->thing_arn = malloc((size_t)length + 1);
    strncpy( info->thing_arn, thing_arn, length );
    info->thing_arn[length] = '\0';
    return;
}

static void greengrass_add_connection_node_metadata( char* metadata, uint16_t length )
{
    cy_linked_list_node_t* node = NULL;
    aws_greengrass_core_info_t* core_info = NULL;
    cy_linked_list_t* connection_list = NULL;
    aws_greengrass_core_connection_info_t* connection = NULL;

    cy_linked_list_get_rear_node(group_list, &node);
    if( !node || !node->data )
    {
        return;
    }

    core_info = &( (aws_greengrass_core_t *)(node->data) )->info;

    connection_list = &core_info->connections;

    cy_linked_list_get_rear_node(connection_list, &node);
    if( !node || !node->data )
    {
        return;
    }

    connection = &( (aws_greengrass_core_connection_t *)node->data)->info;

    connection->metadata = malloc( (size_t)length + 1 );
    if( !connection->metadata )
        return;
    strncpy(connection->metadata, metadata, length);
    connection->metadata[length] = '\0';
    return;
}

static void greengrass_add_connection_node_port( char* port, uint16_t length )
{
    cy_linked_list_node_t* node = NULL;
    aws_greengrass_core_info_t* core_info = NULL;
    cy_linked_list_t* connection_list = NULL;
    aws_greengrass_core_connection_info_t* connection = NULL;

    cy_linked_list_get_rear_node(group_list, &node);
    if( !node || !node->data )
    {
        return;
    }

    core_info = &( (aws_greengrass_core_t *)(node->data) )->info;

    connection_list = &core_info->connections;

    cy_linked_list_get_rear_node(connection_list, &node);
    if( !node || !node->data )
    {
        return;
    }

    connection = &( (aws_greengrass_core_connection_t *)node->data)->info;

    connection->port = malloc( (size_t)length + 1 );
    if( !connection->port )
        return;
    strncpy(connection->port, port, length);
    connection->port[length] = '\0';
    return;
}

static void greengrass_initialize_connection_node( char* host_address, uint16_t length )
{
    cy_linked_list_node_t* node = NULL;
    aws_greengrass_core_info_t* core_info = NULL;

    cy_linked_list_t* connection_list = NULL;
    aws_greengrass_core_connection_info_t* info = NULL;
    aws_greengrass_core_connection_t* connection = NULL;

    cy_linked_list_get_rear_node(group_list, &node);
    if( !node || !node->data )
    {
        return;
    }

    core_info = &( (aws_greengrass_core_t *)(node->data) )->info;

    connection_list = &core_info->connections;

    /* hostAddress field indicates start of a new connection entry for this core */
    connection = malloc(sizeof(aws_greengrass_core_connection_t) );
    if( !connection )
    {
        return;
    }

    info = &connection->info;
    node = &connection->node;

    cy_linked_list_set_node_data( node, connection );

    /* Copy host-address */
    info->ip_address = malloc( (size_t)length + 1 );
    strncpy(info->ip_address, host_address, length);
    info->ip_address[length] = '\0';

    cy_linked_list_insert_node_at_rear( connection_list, node );
    return;
}

cy_rslt_t json_callback_for_discovery_payload (cy_JSON_object_t* json_object )
{
    /* Make sure that first JSON object is "GGGroups"; if we find it, all good; else it is probably not a valid json payload */
    if( json_object_counter == 0 )
    {
        if( strncmp( GG_GROUP_KEY, json_object->object_string, strlen(GG_GROUP_KEY) ) == 0 )
        {
            if( greengrass_initialize_group_list() )
            {
                /* if we only can create a linked list for collecting all groups */
                gg_group_found = 1;
            }
        }
        json_object_counter++;
        return CY_RSLT_SUCCESS;
    }

    /* Ignore JSON objects if 'GGGroups' object was not found earlier */
    if( json_object_counter && !gg_group_found )
    {
        json_object_counter++;
        return CY_RSLT_AWS_ERROR_GG_DISCOVERY_FAILED;
    }

    /* Here is a valid JSON Discovery Payload */

    /* First lookout for 'GGGroupID' and if found create a 'core' node corresponding to it */
    if( strncmp(GG_GROUP_ID, json_object->object_string, strlen(GG_GROUP_ID) ) == 0 )
    {
        greengrass_initialize_core_node(json_object->value, json_object->value_length );
        return CY_RSLT_SUCCESS;
    }

    /* store the 'ThingARN' for this group */
    if( strncmp(GG_CORE_THING_ARN, json_object->object_string, strlen(GG_CORE_THING_ARN) ) == 0 )
    {
        greengrass_add_core_thing_arn(json_object->value, json_object->value_length);
        return CY_RSLT_SUCCESS;
    }
    /* If 'HostAddress' is available, create a connection node */
    if( strncmp( GG_HOST_ADDRESS, json_object->object_string, strlen(GG_HOST_ADDRESS)  ) == 0 )
    {
        greengrass_initialize_connection_node(json_object->value, json_object->value_length);
        return CY_RSLT_SUCCESS;
    }
    /* fill 'PortNumber' to the connection node created earlier */
    if( strncmp(GG_PORT, json_object->object_string, strlen(GG_PORT) ) == 0 )
    {
        greengrass_add_connection_node_port(json_object->value, json_object->value_length);
        return CY_RSLT_SUCCESS;
    }
    /* And 'Metadata' to the connection node */
    if( strncmp(GG_METADATA, json_object->object_string, strlen(GG_METADATA) ) == 0 )
    {
        greengrass_add_connection_node_metadata(json_object->value, json_object->value_length);
        return CY_RSLT_SUCCESS;
    }

    if( strncmp(GG_ROOT_CAS, json_object->object_string, strlen(GG_ROOT_CAS) ) == 0 )
    {
        return CY_RSLT_SUCCESS;
    }

    if( strncmp( GG_BEGIN_CERTIFICATE, json_object->value, strlen(GG_BEGIN_CERTIFICATE) ) == 0 )
    {
        greengrass_add_core_root_ca( json_object->value, json_object->value_length );
        return CY_RSLT_SUCCESS;
    }
    return CY_RSLT_SUCCESS;
}
