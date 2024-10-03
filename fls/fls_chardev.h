/*
 **************************************************************************
 * Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef FLS_CHARDEV_H
#define FLS_CHARDEV_H

#include <linux/cdev.h>
#include <linux/types.h>
#include "fls_def_sensor.h"

enum FLS_CMD_TYPE {
	FLS_CHARDEV_FLUSH,
	FLS_CHARDEV_EVENT,
	FLS_CHARDEV_RESULT
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

struct fls_chardev {
	struct cdev cdev;
	struct class *cl;
	dev_t devid;
	wait_queue_head_t readq;
};

void fls_chardev_shutdown(void);
int fls_chardev_init(void);

#endif
