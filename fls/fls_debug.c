/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include "fls_debug.h"

#ifndef FLS_LITE_ENABLE
int fls_conn_ipv4_sprint(uint32_t addr, char *str, size_t len)
{
	return snprintf(str, len, "%u.%u.%u.%u",
			addr & 0xFF,
			(addr >> 8) & 0xFF,
			(addr >> 16) & 0xFF,
			addr >> 24);
}

void fls_debug_print_event_info(struct fls_event *event)
{
	char ipaddr_str[16];

	if (fls_debug_level_current < FLS_DEBUG_LEVEL_INFO) {
		return;
	}

	printk("type: %d, dir: %d, ipv: %d, pro: %d\n", event->event_type, event->dir, event->ip_version, event->protocol);
	fls_conn_ipv4_sprint(event->orig_src_ip[0], ipaddr_str, 16);
	printk("orig_src = %s:%hu", ipaddr_str, ntohs(event->orig_src_port));
	fls_conn_ipv4_sprint(event->orig_dest_ip[0], ipaddr_str, 16);
	printk("orig_dst = %s:%hu\n", ipaddr_str, ntohs(event->orig_dest_port));

	fls_conn_ipv4_sprint(event->ret_src_ip[0], ipaddr_str, 16);
	printk("repl_src = %s:%hu", ipaddr_str, ntohs(event->ret_src_port));
	fls_conn_ipv4_sprint(event->ret_dest_ip[0], ipaddr_str, 16);
	printk("repl_dst = %s:%hu\n", ipaddr_str, ntohs(event->ret_dest_port));
}

void fls_debug_print_conn_info(struct fls_conn *conn)
{
	struct fls_conn *reply;
	char ipaddr_str[16];
	uint32_t i;

	if (!conn) {
		printk("fls_debug_print_conn_info: NULL connection pointer\n");
		return;
	}

	if (fls_debug_level_current < FLS_DEBUG_LEVEL_INFO) {
		return;
	}

	printk("%p ipv: %u, pro: %u\n", conn, conn->ip_version, conn->protocol);
	if (conn->ip_version != 4) {
		printk("%p Cannot print ipv6 connections yet.\n", conn);
	}

	fls_conn_ipv4_sprint(conn->src_ip[0], ipaddr_str, 16);
	printk("%p orig_src = %s:%hu", conn, ipaddr_str, ntohs(conn->src_port));
	for (i = 1; i < 4; i++) {
		if (conn->src_ip[i]) {
			printk("%p orig_src[%u] = %x", conn, i, conn->src_ip[i]);
		}
	}

	fls_conn_ipv4_sprint(conn->dest_ip[0], ipaddr_str, 16);
	printk("%p orig_dst = %s:%hu\n", conn, ipaddr_str, ntohs(conn->dest_port));
	for (i = 1; i < 4; i++) {
		if (conn->dest_ip[i]) {
			printk("%p orig_dst[%u] = %x", conn, i, conn->dest_ip[i]);
		}
	}
	printk("%p: traffic_class=%u\n", conn, conn->traffic_class);

	reply = conn->reverse;
	if (!reply) {
		return;
	}

	fls_conn_ipv4_sprint(reply->src_ip[0], ipaddr_str, 16);
	printk("%p repl_src = %s:%hu", reply, ipaddr_str, ntohs(reply->src_port));
	for (i = 1; i < 4; i++) {
		if (reply->src_ip[i]) {
			printk("%p repl_src[%u] = %x", reply, i, reply->src_ip[i]);
		}
	}

	fls_conn_ipv4_sprint(reply->dest_ip[0], ipaddr_str, 16);
	printk("%p repl_dst = %s:%hu\n", reply, ipaddr_str, ntohs(reply->dest_port));
	for (i = 1; i < 4; i++) {
		if (reply->src_ip[i]) {
			printk("%p repl_dst[%u] = %x", reply, i, reply->dest_ip[i]);
		}
	}
	printk("%p: traffic_class=%u\n", reply, reply->traffic_class);
}
#endif /* !FLS_LITE_ENABLE */

void fls_debug_print(uint32_t level, char *fmt, ...) {
	va_list args;

	if (level <= fls_debug_level_current) {
		va_start(args, fmt);
		vprintk(fmt, args);
		va_end(args);
	}
}
