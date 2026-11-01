/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#ifndef __FLS_DEBUG_H
#define __FLS_DEBUG_H

#include "fls_config.h"

#ifndef FLS_LITE_ENABLE
enum FLS_PFS_CMD_TYPE {
	FLS_PFS_FLUSH,
	FLS_PFS_EVENT,
	FLS_PFS_RESULT,
	FLS_PFS_CLEAN_EVENTS,
	FLS_PFS_REINSPECT
};

struct fls_cmdinfo {
	uint8_t cmd;
	uint8_t version;
	uint32_t src_ip[4];
	uint32_t dst_ip[4];
	uint16_t src_port;
	uint16_t dst_port;
	uint8_t protocol;
	union fls_data {
		struct packetinfo{
			uint32_t packet_size;
			uint32_t timestamp_sec;
			long timestamp_nsec;
		} fls_packetinfo;
		uint8_t classid;
	} data;
};
#endif

enum fls_debug_level {
	FLS_DEBUG_LEVEL_NONE,
	FLS_DEBUG_LEVEL_ERROR,
	FLS_DEBUG_LEVEL_WARN,
	FLS_DEBUG_LEVEL_INFO,
	FLS_DEBUG_LEVEL_TRACE,
	FLS_DEBUG_LEVEL_MAX
};

#define FLS_ERROR(...) fls_debug_print(FLS_DEBUG_LEVEL_ERROR, __VA_ARGS__)
#define FLS_WARN(...) fls_debug_print(FLS_DEBUG_LEVEL_WARN, __VA_ARGS__)
#define FLS_INFO(...) fls_debug_print(FLS_DEBUG_LEVEL_INFO, __VA_ARGS__)
#define FLS_TRACE(...) fls_debug_print(FLS_DEBUG_LEVEL_TRACE, __VA_ARGS__)

extern uint32_t fls_debug_level_current;

void fls_debug_print(uint32_t level, char *fmt, ...);
#ifndef FLS_LITE_ENABLE
int fls_conn_ipv4_sprint(uint32_t addr, char *str, size_t len);
void fls_debug_print_event_info(struct fls_event *event);
void fls_debug_print_conn_info(struct fls_conn *conn);
#endif

#endif
