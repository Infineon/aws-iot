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
 *  Enumeration of AWS and other internal error codes
 */
#pragma once
#include "cy_result_mw.h"

#define CY_RSLT_AWS_ERROR_CONNECT_FAILED            (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 1)
#define CY_RSLT_AWS_ERROR_INVALID_ROOTCA            (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 2)
#define CY_RSLT_AWS_ERROR_INVALID_CLIENT_KEY        (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 3)
#define CY_RSLT_AWS_ERROR_PUBLISH_FAILED            (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 4)
#define CY_RSLT_AWS_ERROR_SUBSCRIBE_FAILED          (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 5)
#define CY_RSLT_AWS_ERROR_UNSUBSCRIBE_FAILED        (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 6)
#define CY_RSLT_AWS_ERROR_DISCONNECT_FAILED         (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 7)
#define CY_RSLT_AWS_ERROR_INVALID_YIELD_TIMEOUT     (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 8)
#define CY_RSLT_AWS_ERROR_HTTP_FAILURE              (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 9)
#define CY_RSLT_AWS_ERROR_GG_DISCOVERY_FAILED       (cy_rslt_t)(CY_RSLT_MODULE_AWS_BASE + 10)
