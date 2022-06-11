/*
 **************************************************************************
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include "exports/nsm_nl_fam.h"
#include "nsm_sfe.h"
#include "nsm_lat.h"
#include "nsm_procfs.h"

#define NSM_NL_OPS_CNT (NSM_NL_CMD_MAX - 1)

static int nsm_nl_get_latency(struct sk_buff *skb, struct genl_info *info);
static int nsm_nl_get_stats(struct sk_buff *skb, struct genl_info *info);
static int nsm_nl_get_throughput(struct sk_buff *skb, struct genl_info *info);

/*
 * nsm_nl_pol
 *	Policies for the nsm_nl generic netlink family attributes.
 */
struct nla_policy nsm_nl_pol[NSM_NL_ATTR_MAX] = {
	[NSM_NL_ATTR_RX_PACKETS] = { .type = NLA_U64 },
	[NSM_NL_ATTR_RX_BYTES] = { .type = NLA_U64 },
	[NSM_NL_ATTR_SERVICE_ID] = { .type = NLA_U8 },
	[NSM_NL_ATTR_NET_DEVICE] = { .type = NLA_STRING, .len = IFNAMSIZ },
	[NSM_NL_ATTR_LATENCY_MEAN] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST0] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST1] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST2] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST3] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST4] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST5] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST6] = { .type = NLA_U64 },
	[NSM_NL_ATTR_LATENCY_HIST7] = { .type = NLA_U64 },
};

/*
 * nsm_nl_ops
 *	Operations for the nsm_nl generic netlink family.
 */
struct genl_ops nsm_nl_ops[NSM_NL_OPS_CNT] = {
	{
		.cmd = NSM_NL_CMD_GET_STATS,
		.flags = 0,
		.doit = nsm_nl_get_stats,
		.dumpit = NULL,
	},
	{
		.cmd = NSM_NL_CMD_GET_THROUGHPUT,
		.flags = 0,
		.doit = nsm_nl_get_throughput,
		.dumpit = NULL,
	},
	{
		.cmd = NSM_NL_CMD_GET_LATENCY,
		.flags = 0,
		.doit = nsm_nl_get_latency,
		.dumpit = NULL,
	},
};

/*
 * nsm_nl_fam
 *	Structure defining the nsm_nl generic netlink family.
 */
struct genl_family nsm_nl_fam = {
	.hdrsize = 0,
	.name = NSM_NL_NAME,
	.version = 1,
	.policy = nsm_nl_pol,
	.maxattr = ARRAY_SIZE(nsm_nl_pol),
	.ops = nsm_nl_ops,
	.n_ops = ARRAY_SIZE(nsm_nl_ops)
};

/*
 * nsm_nl_get_latency()
 *	Callback to retrieve per-service-class latency from a netdevice.
 */
static int nsm_nl_get_latency(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *reply;
	struct nlattr *nla;
	char netdev_name[IFNAMSIZ];
	uint8_t sid;
	uint64_t hist[NETDEV_SAWF_DELAY_BUCKETS];
	uint64_t avg;
	uint32_t bucket;
	void *reply_header;


	nla = info->attrs[NSM_NL_ATTR_NET_DEVICE];
	if (!nla) {
		return -1;
	}

	nla_strlcpy(netdev_name, nla, IFNAMSIZ);

	nla = info->attrs[NSM_NL_ATTR_SERVICE_ID];
	if (!nla) {
		return -1;
	}

	sid = nla_get_u8(nla);
	if (sid >= SFE_MAX_SERVICE_CLASS_ID) {
		return -1;
	}

	reply = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!reply) {
		return -1;
	}

	/*
	 * Initialize reply header.
	 */
	reply_header = genlmsg_put(reply, info->snd_portid, info->snd_seq,
				&nsm_nl_fam, 0, NSM_NL_CMD_GET_LATENCY);

	if (!reply_header) {
		goto error;
	}

	if (!nsm_lat_get(netdev_name, sid, hist, &avg)) {
		goto error;
	}

	for (bucket = 0; bucket < NETDEV_SAWF_DELAY_BUCKETS; bucket++) {
		if (nla_put_u64_64bit(reply, NSM_NL_ATTR_LATENCY_HIST0 + bucket, hist[bucket], NSM_NL_ATTR_PAD)) {
			goto error;
		}
	}

	if (nla_put_u64_64bit(reply, NSM_NL_ATTR_LATENCY_MEAN, avg, NSM_NL_ATTR_PAD) ||
		nla_put_string(reply, NSM_NL_ATTR_NET_DEVICE, netdev_name) ||
		nla_put_u8(reply, NSM_NL_ATTR_SERVICE_ID, sid)) {
		goto error;
	}

	/*
	 * Finalize the reply.
	 */
	genlmsg_end(reply, reply_header);

	/*
	 * Send the reply.
	 */
	if (genlmsg_unicast(genl_info_net(info), reply, info->snd_portid)) {
		return -1;
	}

	return 0;

error:
	nlmsg_free(reply);
	return -1;
}

/*
 * nsm_nl_get_stats
 *	Callback to get stats from a given service class.
 */
static int nsm_nl_get_stats(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *reply;
	struct nlattr *nla;
	uint8_t sid;
	struct nsm_sfe_stats *stats;
	void *reply_header;

	/*
	 * Start by allocating a buffer for the response so we fail early in
	 * case of insufficient skbs.
	 * TODO: Research whether we can put the replay in the buffer we
	 * received.
	 */
	reply = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!reply) {
		return -1;
	}

	/*
	 * Extract service id from info. If the service id attribute is absent,
	 * the request is badly formed and we return an error.
	 */
	nla = info->attrs[NSM_NL_ATTR_SERVICE_ID];
	if (!nla) {
		goto error;
	}

	sid = nla_get_u8(nla);
	if (sid >= SFE_MAX_SERVICE_CLASS_ID) {
		goto error;
	}

	/*
	 * Fetch relevant data.
	 */
	stats = nsm_sfe_get_stats(sid);
	if (!stats) {
		goto error;
	}

	/*
	 * Initialize reply header.
	 */
	reply_header = genlmsg_put(reply, info->snd_portid, info->snd_seq,
				&nsm_nl_fam, 0, NSM_NL_CMD_GET_STATS);
	if (!reply_header) {
		goto error;
	}

	/*
	 * Populate reply with information.
	 */
	if (nla_put_u64_64bit(reply, NSM_NL_ATTR_RX_BYTES, stats->bytes, NSM_NL_ATTR_PAD) ||
		nla_put_u64_64bit(reply, NSM_NL_ATTR_RX_PACKETS, stats->packets, NSM_NL_ATTR_PAD) ||
		nla_put_u8(reply, NSM_NL_ATTR_SERVICE_ID, sid)) {
		goto error;
	}

	/*
	 * Finalize the reply.
	 */
	genlmsg_end(reply, reply_header);

	/*
	 * Send the reply. genlmsg_unicast() frees the reply on failure, so no
	 * need to free here.
	 */
	if (genlmsg_unicast(genl_info_net(info), reply, info->snd_portid)) {
		return -1;
	}

	return 0;
error:
	nlmsg_free(reply);
	return -1;
}

/*
 * nsm_nl_get_throughput
 *	Callback to get throughput from a given service class.
 */
static int nsm_nl_get_throughput(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *reply;
	struct nlattr *nla;
	uint8_t sid;
	uint64_t byte_rate, packet_rate;
	void *reply_header;

	/*
	 * Start by allocating a buffer for the response so we fail early in
	 * case of insufficient skbs.
	 */
	reply = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!reply) {
		return -1;
	}

	/*
	 * Extract service id from info. If the service id attribute is absent,
	 * the request is badly formed and we return an error.
	 */
	nla = info->attrs[NSM_NL_ATTR_SERVICE_ID];
	if (!nla) {
		goto error;
	}

	sid = nla_get_u8(nla);
	if (sid >= SFE_MAX_SERVICE_CLASS_ID) {
		goto error;
	}

	/*
	 * Fetch relevant data.
	 */
	if (nsm_sfe_get_throughput(sid, &packet_rate, &byte_rate)) {
		goto error;
	}

	/*
	 * Initialize reply header.
	 */
	reply_header = genlmsg_put(reply, info->snd_portid, info->snd_seq,
				&nsm_nl_fam, 0, NSM_NL_CMD_GET_THROUGHPUT);
	if (!reply_header) {
		goto error;
	}

	/*
	 * Populate reply with information.
	 */
	if (nla_put_u64_64bit(reply, NSM_NL_ATTR_RX_BYTES, byte_rate, NSM_NL_ATTR_PAD) ||
		nla_put_u64_64bit(reply, NSM_NL_ATTR_RX_PACKETS, packet_rate, NSM_NL_ATTR_PAD) ||
		nla_put_u8(reply, NSM_NL_ATTR_SERVICE_ID, sid)) {
		goto error;
	}

	/*
	 * Finalize the reply.
	 */
	genlmsg_end(reply, reply_header);

	/*
	 * Send the reply.
	 */
	if (genlmsg_unicast(genl_info_net(info), reply, info->snd_portid)) {
		return -1;
	}

	return 0;

error:
	nlmsg_free(reply);
	return -1;
}

/*
 * nsm_nl_exit()
 *	Shut down the netlink module.
 */
void __exit nsm_nl_exit(void)
{
	genl_unregister_family(&nsm_nl_fam);
	nsm_procfs_deinit();
}

/*
 * nsm_nl_init()
 *	INitialize the netlink module.
 */
int __init nsm_nl_init(void)
{
	int err = genl_register_family(&nsm_nl_fam);
	if (err) {
		printk("qca-nss-nsm: Register family failed with error %i", err);
	}

	nsm_procfs_init();

	return err;
}

module_init(nsm_nl_init)
module_exit(nsm_nl_exit)

MODULE_AUTHOR("Qualcomm Technologies");
MODULE_DESCRIPTION("Networking State Module");
MODULE_LICENSE("Dual BSD/GPL");
