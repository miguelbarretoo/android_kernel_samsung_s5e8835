#include "ufs-vs-mmio.h"

struct uic_pwr_mode {
	u8 lane;
	u8 gear;
	u8 mode;
	u8 hs_series;
};

enum {
	HOST_EMBD = 0,
	HOST_CARD = 1,
};

enum {
	GEAR_1 = 1,
	GEAR_2,
	GEAR_3,
	GEAR_4,
};

struct ufs_eom_result_s {
	union {
		u32 phase;
		u32 v_phase;
	};
	union {
		u32 vref;
		u32 v_vref;
	};
	union {
		u64 err;
		u32 v_err;
	};
};

struct ufs_cal_param {
	void *host;		/* Host adaptor */
	struct ufs_vs_handle *handle;
	u8 available_lane;
	u8 connected_tx_lane;
	u8 connected_rx_lane;
	u8 active_tx_lane;
	u8 active_rx_lane;
	u32 mclk_rate;
	u8 tbl;
	u32 board;
	u8 evt_ver;
	u8 max_gear;
	struct uic_pwr_mode *pmd;
	struct ufs_eom_result_s *eom[4];
	u32 eom_sz;
	u32 mclk_period;
	u32 mclk_period_rnd_off;
	u32 mclk_period_unipro_18;
	u32 support_ah8_cal;
	u32 ah8_thinern8_time;
	u32 ah8_brefclkgatingwaittime;
	u32 save_and_restore_mode;
	unsigned long m_phy_bias;
};

typedef enum {
	UFS_CAL_NO_ERROR = 0,
	UFS_CAL_TIMEOUT,
	UFS_CAL_ERROR,
	UFS_CAL_INV_ARG,
	UFS_CAL_INV_CONF,
} ufs_cal_errno;

enum {
	__BRD_SMDK,
	__BRD_ASB,
	__BRD_HSIE,
	__BRD_ZEBU,
	__BRD_UNIV,
	__BRD_MAX,
};
#define BRD_SMDK	(1U << __BRD_SMDK)
#define BRD_ASB		(1U << __BRD_ASB)
#define BRD_HSIE	(1U << __BRD_HSIE)
#define BRD_ZEBU	(1U << __BRD_ZEBU)
#define BRD_UNIV	(1U << __BRD_UNIV)
#define BRD_MAX		(1U << __BRD_MAX)
#define BRD_ALL		((1U << __BRD_MAX) - 1)

#define MAX_LANE	4

#define EOM_PH_SEL_MAX	72
#define EOM_DEF_VREF_MAX	256
#define EOM_MAX_SIZE	(EOM_PH_SEL_MAX * EOM_DEF_VREF_MAX * 8)

static const u32 ufs_s_eom_repeat[5] = { 0, 8, 4, 2, 1 };

enum {
	NO_MODE = 0,
	SAVE_MODE,
	RESTORE_MODE,
};

/* UFS CAL interface */
typedef ufs_cal_errno (*cal_if_func)(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_post_h8_enter(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_pre_h8_exit(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_post_pmc(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_pre_pmc(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_post_link(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_pre_link(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_init(struct ufs_cal_param *p);

ufs_cal_errno ufs_cal_loopback_init(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_loopback_set_1(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_loopback_set_2(struct ufs_cal_param *p);

ufs_cal_errno ufs_cal_eom(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_pre_pm(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_post_pm(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_during_hce_enable(struct ufs_cal_param *p);
ufs_cal_errno ufs_cal_resume_hibern8(struct ufs_cal_param *p);

/* Adaptor for UFS CAL */
void ufs_lld_dme_set(void *h, u32 addr, u32 val);
void ufs_lld_dme_get(void *h, u32 addr, u32 *val);
void ufs_lld_dme_peer_set(void *h, u32 addr, u32 val);
void ufs_lld_pma_write(void *h, u32 val, u32 addr);
u32 ufs_lld_pma_read(void *h, u32 addr);
void ufs_lld_unipro_write(void *h, u32 val, u32 addr);
void ufs_lld_udelay(u32 val);
void ufs_lld_usleep_delay(u32 min, u32 max);
unsigned long ufs_lld_get_time_count(unsigned long offset);
unsigned long ufs_lld_calc_timeout(const unsigned int ms);
