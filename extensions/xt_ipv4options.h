#pragma once

/* IPv4 allows for a 5-bit option number - 32 options */

/**
 * %XT_V4OPTS_ALL:	all options in @map must be present (respecting @invert)
 * %XT_V4OPTS_ANY:	any of the option in @map
 */
enum xt_ipv4options_flags {
	XT_V4OPTS_ALL = 1 << 0,
	XT_V4OPTS_ANY = 1 << 1,
};

/**
 * @map:	bitmask of options that should appear
 * @invert:	inversion map
 * @flags:	see above
 */
struct xt_ipv4options_mtinfo1 {
	__u32 map, invert;
	__u8 flags;
};
