// SPDX-License-Identifier: GPL-2.0
/*
 * Legacy "schedtune" cgroup interface for pre-GKI Android userspace.
 *
 * Android 11-era task_profiles mount a v1 schedtune hierarchy and write
 * schedtune.boost / schedtune.prefer_idle; libprocessgroup aborts cgroup
 * setup when the controller is missing. This keeps the interface only:
 * values are stored and read back, scheduling is left to uclamp.
 */
#include <linux/cgroup.h>
#include <linux/slab.h>

struct schedtune {
	struct cgroup_subsys_state css;
	s64 boost;
	u64 prefer_idle;
};

static inline struct schedtune *css_st(struct cgroup_subsys_state *css)
{
	return container_of(css, struct schedtune, css);
}

static struct cgroup_subsys_state *
schedtune_css_alloc(struct cgroup_subsys_state *parent_css)
{
	struct schedtune *st = kzalloc(sizeof(*st), GFP_KERNEL);

	return st ? &st->css : ERR_PTR(-ENOMEM);
}

static void schedtune_css_free(struct cgroup_subsys_state *css)
{
	kfree(css_st(css));
}

static s64 boost_read(struct cgroup_subsys_state *css, struct cftype *cft)
{
	return READ_ONCE(css_st(css)->boost);
}

static int boost_write(struct cgroup_subsys_state *css, struct cftype *cft,
		       s64 boost)
{
	if (boost < -100 || boost > 100)
		return -EINVAL;
	WRITE_ONCE(css_st(css)->boost, boost);
	return 0;
}

static u64 prefer_idle_read(struct cgroup_subsys_state *css, struct cftype *cft)
{
	return READ_ONCE(css_st(css)->prefer_idle);
}

static int prefer_idle_write(struct cgroup_subsys_state *css,
			     struct cftype *cft, u64 prefer_idle)
{
	WRITE_ONCE(css_st(css)->prefer_idle, !!prefer_idle);
	return 0;
}

static struct cftype schedtune_files[] = {
	{
		.name = "boost",
		.read_s64 = boost_read,
		.write_s64 = boost_write,
	},
	{
		.name = "prefer_idle",
		.read_u64 = prefer_idle_read,
		.write_u64 = prefer_idle_write,
	},
	{ }	/* terminate */
};

struct cgroup_subsys schedtune_cgrp_subsys = {
	.css_alloc	= schedtune_css_alloc,
	.css_free	= schedtune_css_free,
	.legacy_cftypes	= schedtune_files,
};
