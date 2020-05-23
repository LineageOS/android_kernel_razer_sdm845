#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static char bandinfo[256];

static int fih_info_proc_show_bandinfo(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", bandinfo);
	return 0;
}

static int fih_info_proc_open_bandinfo(struct inode *inode, struct file *file)
{
	return single_open(file, fih_info_proc_show_bandinfo, NULL);
}

static const struct file_operations fih_info_fops_bandinfo = {
	.open    = fih_info_proc_open_bandinfo,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

static int fih_info_property(struct platform_device *pdev)
{
	int rc = 0;
	static const char *p_chr;

	p_chr = of_get_property(pdev->dev.of_node, "fih-info,bandinfo", NULL);
	if (!p_chr) {
		pr_info("%s:%d, bandinfo not specified\n", __func__, __LINE__);
	} else {
		strlcpy(bandinfo, p_chr, sizeof(bandinfo));
		pr_info("%s: bandinfo = %s\n", __func__, bandinfo);
	}

	return rc;
}

static int fih_info_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (!pdev || !pdev->dev.of_node) {
		pr_err("%s: Unable to load device node\n", __func__);
		return -ENOTSUPP;
	}

	rc = fih_info_property(pdev);
	if (rc) {
		pr_err("%s Unable to set property\n", __func__);
		return rc;
	}

	proc_create("bandinfo", 0, NULL, &fih_info_fops_bandinfo);

	return rc;
}

static int fih_info_remove(struct platform_device *pdev)
{
	remove_proc_entry ("bandinfo", NULL);

	return 0;
}

static const struct of_device_id fih_info_dt_match[] = {
	{ .compatible = "fih_info" },
	{}
};
MODULE_DEVICE_TABLE(of, fih_info_dt_match);

static struct platform_driver fih_info_driver = {
	.probe = fih_info_probe,
	.remove = fih_info_remove,
	.shutdown = NULL,
	.driver = {
		.name = "fih_info",
		.of_match_table = fih_info_dt_match,
	},
};

static int __init fih_info_init(void)
{
	int ret;

	ret = platform_driver_register(&fih_info_driver);
	if (ret) {
		pr_err("%s: failed!\n", __func__);
		return ret;
	}

	return ret;
}
module_init(fih_info_init);

static void __exit fih_info_exit(void)
{
	platform_driver_unregister(&fih_info_driver);
}
module_exit(fih_info_exit);
