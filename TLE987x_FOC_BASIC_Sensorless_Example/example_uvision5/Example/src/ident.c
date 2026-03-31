/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

#include "ident.h"

/* Macros to define the example project version */
#define ID_VERSION_MAJOR (2U)
#define ID_VERSION_MINOR (0U)
#define ID_VERSION_PATCH (0U)
#define ID_VERSION_T     (4U)

/* Example Project Version */
const id_ExampleProjectVersion id_exampleProjectVersion = {
    .major = ID_VERSION_MAJOR, .minor = ID_VERSION_MINOR, .patch = ID_VERSION_PATCH, .t = ID_VERSION_T
};
const uint32_t                 id_imageID               = 0x5347584d;
