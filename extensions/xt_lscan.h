#ifndef _LINUX_NETFILTER_XT_LSCAN_H
#define _LINUX_NETFILTER_XT_LSCAN_H 1

enum {
	LSCAN_FL1_STEALTH = 1 << 0,
	LSCAN_FL2_SYN     = 1 << 0,
	LSCAN_FL3_CN      = 1 << 0,
	LSCAN_FL4_GR      = 1 << 0,
};

struct xt_lscan_mtinfo {
	uint8_t match_fl1, match_fl2, match_fl3, match_fl4;
};

#endif /* _LINUX_NETFILTER_XT_LSCAN_H */
