// SPDX-License-Identifier: GPL-2.0
/*
 * Legacy /dev/ion for vendors built against the 4.14+ ION UAPI.
 *
 * Exynos 990 ships an Android 11 (API 30) vendor whose gralloc, camera and
 * codec HALs allocate through /dev/ion. ION is gone in 5.15, so this exposes
 * just the two ioctls that UAPI kept (ALLOC, HEAP_QUERY) and backs every
 * legacy heap with the matching DMA-BUF heap. Heap ids are table indices,
 * which userspace learns from HEAP_QUERY, as with 4.19 ION.
 */
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/uaccess.h>

/* 4.19 drivers/staging/android/uapi/ion.h */
#define ION_HEAP_TYPE_SYSTEM	0
#define ION_FLAG_CACHED		1
#define ION_FLAG_PROTECTED	16
#define MAX_HEAP_NAME		32

struct ion_allocation_data {
	__u64 len;
	__u32 heap_id_mask;
	__u32 flags;
	__u32 fd;
	__u32 unused;
};

struct ion_heap_data {
	char name[MAX_HEAP_NAME];
	__u32 type;
	__u32 heap_id;
	__u32 size;
	__u32 heap_flags;
	__u32 reserved2;
};

struct ion_heap_query {
	__u32 cnt;
	__u32 reserved0;
	__u64 heaps;
	__u32 reserved1;
	__u32 reserved2;
};

#define ION_IOC_MAGIC		'I'
#define ION_IOC_ALLOC		_IOWR(ION_IOC_MAGIC, 0, struct ion_allocation_data)
#define ION_IOC_FREE		_IOWR(ION_IOC_MAGIC, 1, int)	/* deprecated no-op */
#define ION_IOC_HEAP_QUERY	_IOWR(ION_IOC_MAGIC, 8, struct ion_heap_query)

struct ion_compat_heap {
	const char *ion_name;
	u32 type;
	const char *cached;	/* DMA-BUF heap for ION_FLAG_CACHED */
	const char *uncached;
};

/* shortcut: system heap only; add secure/camera/codec heaps with their DT nodes */
static const struct ion_compat_heap heaps[] = {
	{ "ion_system_heap", ION_HEAP_TYPE_SYSTEM, "system", "system-uncached" },
};

static bool heap_present(const struct ion_compat_heap *h)
{
	struct dma_heap *dh = dma_heap_find(h->cached);

	if (!dh)
		return false;
	dma_heap_put(dh);
	return true;
}

static long ion_compat_query(struct ion_heap_query __user *uq)
{
	struct ion_heap_query q;
	struct ion_heap_data __user *out;
	u32 i, n = 0;

	if (copy_from_user(&q, uq, sizeof(q)))
		return -EFAULT;

	out = u64_to_user_ptr(q.heaps);
	for (i = 0; i < ARRAY_SIZE(heaps); i++) {
		struct ion_heap_data d = { .type = heaps[i].type, .heap_id = i };

		if (!heap_present(&heaps[i]))
			continue;
		/* cnt == 0 or heaps == 0 asks only for the count */
		if (q.cnt && q.heaps) {
			if (n >= q.cnt)
				break;
			strscpy(d.name, heaps[i].ion_name, sizeof(d.name));
			if (copy_to_user(&out[n], &d, sizeof(d)))
				return -EFAULT;
		}
		n++;
	}

	q.cnt = n;
	return copy_to_user(uq, &q, sizeof(q)) ? -EFAULT : 0;
}

static long ion_compat_alloc(struct ion_allocation_data __user *ua)
{
	struct ion_allocation_data a;
	struct dma_heap *dh = NULL;
	struct dma_buf *buf;
	u32 i;
	int fd;

	if (copy_from_user(&a, ua, sizeof(a)))
		return -EFAULT;
	if (!a.len || a.flags & ION_FLAG_PROTECTED)
		return -EINVAL;

	/* ION tried the requested heaps from the highest id down */
	for (i = ARRAY_SIZE(heaps); i-- > 0 && !dh;)
		if (a.heap_id_mask & BIT(i))
			dh = dma_heap_find(a.flags & ION_FLAG_CACHED ?
					   heaps[i].cached : heaps[i].uncached);
	if (!dh)
		return -ENODEV;

	buf = dma_heap_buffer_alloc(dh, PAGE_ALIGN(a.len), O_RDWR, 0);
	dma_heap_put(dh);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	fd = dma_buf_fd(buf, O_CLOEXEC);
	if (fd < 0) {
		dma_buf_put(buf);
		return fd;
	}

	a.fd = fd;
	if (copy_to_user(ua, &a, sizeof(a))) {
		/* the fd is already installed; userspace owns it via close() */
		return -EFAULT;
	}
	return 0;
}

static long ion_compat_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case ION_IOC_ALLOC:
		return ion_compat_alloc((void __user *)arg);
	case ION_IOC_HEAP_QUERY:
		return ion_compat_query((void __user *)arg);
	case ION_IOC_FREE:
		return 0;
	default:
		return -ENOTTY;
	}
}

static const struct file_operations ion_compat_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= ion_compat_ioctl,
	.compat_ioctl	= compat_ptr_ioctl,
};

static struct miscdevice ion_compat_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "ion",
	.fops	= &ion_compat_fops,
};

module_misc_device(ion_compat_dev);
MODULE_LICENSE("GPL v2");
