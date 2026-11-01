/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#ifndef __FLS_CONFIG_H
#define __FLS_CONFIG_H

#include <linux/sysctl.h>
#include <linux/net.h>
#include <linux/proc_fs.h>
#include "fls_def_sensor.h"

#ifndef FLS_LITE_ENABLE
#include "fls_conn.h"
#include "fls_rfs.h"
#endif


void fls_config_deinit(void);
void fls_config_init(void);

#endif /* __FLS_CONFIG_H */
