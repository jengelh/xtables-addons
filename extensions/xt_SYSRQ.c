/*
 *	"SYSRQ" target extension for Netfilter
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2008
 *
 *	Based upon the ipt_SYSRQ idea by Marek Zalem <marek [at] terminus sk>
 *	xt_SYSRQ does not use hashing or timestamps.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	version 2 or 3 as published by the Free Software Foundation.
 */
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/sysrq.h>
#include <linux/udp.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/x_tables.h>
#include <net/ip.h>
#include "compat_xtables.h"

static bool sysrq_once;
static char sysrq_password[64];
module_param_string(password, sysrq_password, sizeof(sysrq_password),
	S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(password, "password for remote sysrq");

static unsigned int sysrq_tg(const void *pdata, uint16_t len)
{
	const char *data = pdata;
	char c;

	if (*sysrq_password == '\0') {
		if (!sysrq_once)
			printk(KERN_INFO KBUILD_MODNAME "No password set\n");
		sysrq_once = true;
		return NF_DROP;
	}

	if (len == 0)
		return NF_DROP;

	c = *data;
	if (strncmp(&data[1], sysrq_password, len - 1) != 0) {
		printk(KERN_INFO KBUILD_MODNAME "Failed attempt - "
		       "password mismatch\n");
		return NF_DROP;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
	handle_sysrq(c, NULL);
#else
	handle_sysrq(c, NULL, NULL);
#endif
	return NF_ACCEPT;
}

static unsigned int
sysrq_tg4(struct sk_buff **pskb, const struct xt_target_param *par)
{
	struct sk_buff *skb = *pskb;
	const struct iphdr *iph;
	const struct udphdr *udph;
	uint16_t len;

	if (skb_linearize(skb) < 0)
		return NF_DROP;

	iph  = ip_hdr(skb);
	udph = (void *)iph + ip_hdrlen(skb);
	len  = ntohs(udph->len) - sizeof(struct udphdr);

	printk(KERN_INFO KBUILD_MODNAME ": " NIPQUAD_FMT ":%u -> :%u len=%u\n",
	       NIPQUAD(iph->saddr), htons(udph->source), htons(udph->dest),
	       len);
	return sysrq_tg((void *)udph + sizeof(struct udphdr), len);
}

static unsigned int
sysrq_tg6(struct sk_buff **pskb, const struct xt_target_param *par)
{
	struct sk_buff *skb = *pskb;
	const struct ipv6hdr *iph;
	const struct udphdr *udph;
	uint16_t len;

	if (skb_linearize(skb) < 0)
		return NF_DROP;

	iph  = ipv6_hdr(skb);
	udph = udp_hdr(skb);
	len  = ntohs(udph->len) - sizeof(struct udphdr);

	printk(KERN_INFO KBUILD_MODNAME ": " NIP6_FMT ":%hu -> :%hu len=%u\n",
	       NIP6(iph->saddr), ntohs(udph->source),
	       ntohs(udph->dest), len);
	return sysrq_tg(udph + sizeof(struct udphdr), len);
}

static bool sysrq_tg_check(const struct xt_tgchk_param *par)
{
	if (par->target->family == NFPROTO_IPV4) {
		const struct ipt_entry *entry = par->entryinfo;

		if ((entry->ip.proto != IPPROTO_UDP &&
		    entry->ip.proto != IPPROTO_UDPLITE) ||
		    entry->ip.invflags & XT_INV_PROTO)
			goto out;
	} else if (par->target->family == NFPROTO_IPV6) {
		const struct ip6t_entry *entry = par->entryinfo;

		if ((entry->ipv6.proto != IPPROTO_UDP &&
		    entry->ipv6.proto != IPPROTO_UDPLITE) ||
		    entry->ipv6.invflags & XT_INV_PROTO)
			goto out;
	}

	return true;

 out:
	printk(KERN_ERR KBUILD_MODNAME ": only available for UDP and UDP-Lite");
	return false;
}

static struct xt_target sysrq_tg_reg[] __read_mostly = {
	{
		.name       = "SYSRQ",
		.revision   = 0,
		.family     = NFPROTO_IPV4,
		.target     = sysrq_tg4,
		.checkentry = sysrq_tg_check,
		.me         = THIS_MODULE,
	},
	{
		.name       = "SYSRQ",
		.revision   = 0,
		.family     = NFPROTO_IPV6,
		.target     = sysrq_tg6,
		.checkentry = sysrq_tg_check,
		.me         = THIS_MODULE,
	},
};

static int __init sysrq_tg_init(void)
{
	return xt_register_targets(sysrq_tg_reg, ARRAY_SIZE(sysrq_tg_reg));
}

static void __exit sysrq_tg_exit(void)
{
	return xt_unregister_targets(sysrq_tg_reg, ARRAY_SIZE(sysrq_tg_reg));
}

module_init(sysrq_tg_init);
module_exit(sysrq_tg_exit);
MODULE_DESCRIPTION("Xtables: triggering SYSRQ remotely");
MODULE_AUTHOR("Jan Engelhardt <jengelh@medozas.de>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_SYSRQ");
MODULE_ALIAS("ip6t_SYSRQ");