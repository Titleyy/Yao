/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

#ifndef IDENT_H
#define IDENT_H

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include <stdint.h>

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/

/**********************************************************************************************************************
* DATA STRUCTURES
**********************************************************************************************************************/
typedef struct id_ExampleProjectVersion
{
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    uint16_t t;
} id_ExampleProjectVersion;

/***********************************************************************************************************************
* EXTERN DECLARATIONS
***********************************************************************************************************************/
extern const id_ExampleProjectVersion id_exampleProjectVersion;
extern const uint32_t                 id_imageID;
#endif /* IDENT_H */
