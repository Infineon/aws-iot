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
 *  Enumeration of AWS and other internal error codes
 */
#pragma once
#include "cy_result_mw.h"

/**
 * @defgroup aws_iot_defines AWS IoT library error codes
 * @ingroup aws_iot
 *
 * AWS IoT and Greengrass preprocessor directives such as results and error codes
 *
 * Cypress middleware APIs return results of type cy_rslt_t and comprise of three parts:
 * - module base
 * - type
 * - error code
 *
 * \par Result Format
 *
   \verbatim
              Module base         Type    Library specific error code
      +-------------------------+------+------------------------------+
      | CY_RSLT_MODULE_AWS_BASE | 0x2  |           Error Code         |
      +-------------------------+------+------------------------------+
                14-bits          2-bits            16-bits

   Refer to the macro section of this document for library specific error codes.
   \endverbatim
 *
 * The data structure cy_rslt_t is part of cy_result.h in Mbed OS PSoC6 target platform, located in <mbed-os/targets/TARGET_Cypress/TARGET_PSOC6/psoc6csp/core_lib/include>
 *
 * Module base: This base is derived from CY_RSLT_MODULE_MIDDLEWARE_BASE (defined in cy_result.h) and is an offset of the CY_RSLT_MODULE_MIDDLEWARE_BASE
 *              The details of the offset and the middleware base are defined in cy_result_mw.h, that is part of [Github connectivity-utilities] (https://github.com/cypresssemiconductorco/connectivity-utilities)
 *              For instance, AWS uses CY_RSLT_MODULE_AWS_BASE as the module base
 *
 * Type: This type is defined in cy_result.h and can be one of CY_RSLT_TYPE_FATAL, CY_RSLT_TYPE_ERROR, CY_RSLT_TYPE_WARNING or CY_RSLT_TYPE_INFO. AWS library error codes are of type CY_RSLT_TYPE_ERROR
 *
 * Library specific error code: These error codes are library specific and defined in macro section
 *
 * Helper macros used for creating the library specific result are provided as part of cy_result.h
 *
 *  @{
 */
/** AWS error code start */
#define CY_RSLT_MODULE_AWS_ERR_CODE_START       (0)

/** AWS base error code */
#define CY_RSLT_AWS_ERR_BASE                        CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_AWS_BASE, CY_RSLT_MODULE_AWS_ERR_CODE_START)

/** Connection to MQTT broker failed */
#define CY_RSLT_AWS_ERROR_CONNECT_FAILED            (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 1)

/** Loading Root CA certificate failed */
#define CY_RSLT_AWS_ERROR_INVALID_ROOTCA            (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 2)

/** Loading client private key failed  */
#define CY_RSLT_AWS_ERROR_INVALID_CLIENT_KEY        (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 3)

/** Publishing to MQTT broker on topic failed */
#define CY_RSLT_AWS_ERROR_PUBLISH_FAILED            (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 4)

/** Subscribing to MQTT broker on topic failed */
#define CY_RSLT_AWS_ERROR_SUBSCRIBE_FAILED          (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 5)

/** Un-subscribing to MQTT broker on topic failed */
#define CY_RSLT_AWS_ERROR_UNSUBSCRIBE_FAILED        (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 6)

/** Disconnection to MQTT broker failed */
#define CY_RSLT_AWS_ERROR_DISCONNECT_FAILED         (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 7)

/** Invalid yield timeout provided */
#define CY_RSLT_AWS_ERROR_INVALID_YIELD_TIMEOUT     (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 8)

/** Failed to send HTTP request  */
#define CY_RSLT_AWS_ERROR_HTTP_FAILURE              (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 9)

/** AWS Greengrass discovery failed */
#define CY_RSLT_AWS_ERROR_GG_DISCOVERY_FAILED       (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 10)

/** Disconnected from AWS */
#define CY_RSLT_AWS_ERROR_DISCONNECTED              (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 11)

/** Buffer overflow while receiving packet */
#define CY_RSLT_AWS_ERROR_BUFFER_OVERFLOW           (cy_rslt_t)(CY_RSLT_AWS_ERR_BASE + 12)

/**
 * @}
 */
