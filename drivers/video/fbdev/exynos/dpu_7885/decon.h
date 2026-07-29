/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Header file for Exynos DECON driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ___SAMSUNG_DECON_H__
#define ___SAMSUNG_DECON_H__

#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/pm_qos.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/platform_device.h>
#include <media/v4l2-device.h>
#include <media/videobuf2-core.h>
#include <soc/samsung/bts.h>
#include <soc/samsung/exynos-itmon.h>

#include "regs-decon.h"
#include "./panels/decon_lcd.h"
#include "decon_abd.h"
#include "dsim.h"
#include "../../../../staging/android/sw_sync.h"

#define MAX_DECON_CNT		3
#define SUCCESS_EXYNOS_SMC	0

extern struct ion_device *ion_exynos;
extern struct decon_device *decon_drvdata[MAX_DECON_CNT];
extern int decon_log_level;
extern int dpu_bts_log_level;
extern int win_update_log_level;
extern struct decon_bts_ops decon_bts_control;

#define DECON_MODULE_NAME	"exynos-decon"
#define MAX_NAME_SIZE		32
#define MAX_PLANE_CNT		3
#define DECON_ENTER_HIBER_CNT	3
#define DECON_ENTER_LPD_CNT	3
#define MIN_BLK_MODE_WIDTH	144
#define MIN_BLK_MODE_HEIGHT	16
#define VSYNC_TIMEOUT_MSEC	200
#define DEFAULT_BPP		32

#define MAX_DECON_WIN		4
#define MAX_DPP_SUBDEV		4	/* check later */

#define MIN_WIN_BLOCK_WIDTH	360
#define MIN_WIN_BLOCK_HEIGHT	222

#if defined(CONFIG_DPU_20)
#define CHIP_VER		(7885)
#define FD_TRY_CNT		3
#define VALID_FD_VAL		3
#endif
#define DECON_WIN_UPDATE_IDX	(4)

#ifndef KHZ
#define KHZ (1000)
#endif
#ifndef MHZ
#define MHZ (1000*1000)
#endif

#define SHADOW_UPDATE_TIMEOUT	(300 * 1000) /* 300ms */
#define IDLE_WAIT_TIMEOUT	(50 * 1000) /* 50ms */
#define CEIL(x)			((x-(u32)(x) > 0 ? (u32)(x+1) : (u32)(x)))
#define DSC_INIT_XMIT_DELAY	0x200

#define EINT_PEND(x)		((x == 0) ? 2 : ((x == 1) ? 4 : 1))

#define MAX_DSC_SLICE_CNT	4

void dpu_debug_printk(const char *function_name, const char *format, ...);

#define decon_err(fmt, ...)										\
	do {												\
		if (decon_log_level >= 3) {\
			pr_err(pr_fmt("decon: "fmt), ##__VA_ARGS__);\
			exynos_ss_printk(fmt, ##__VA_ARGS__);\
		}\
	} while (0)

#define decon_warn(fmt, ...)\
	do {\
		if (decon_log_level >= 4) {\
			pr_warn(pr_fmt("decon: "fmt), ##__VA_ARGS__);\
			exynos_ss_printk(fmt, ##__VA_ARGS__);\
		}\
	} while (0)

#define decon_info(fmt, ...)\
	do {\
		if (decon_log_level >= 6)\
			pr_info(pr_fmt("decon: "fmt), ##__VA_ARGS__);\
	} while (0)

#define decon_dbg(fmt, ...)\
	do {\
		if (decon_log_level >= 7)\
			pr_info(pr_fmt("decon: "fmt), ##__VA_ARGS__);\
	} while (0)

#define DPU_DEBUG_WIN(fmt, args...)\
	do {\
		if (win_update_log_level >= 7)\
			dpu_debug_printk("WIN_UPDATE", fmt,  ##args);\
	} while (0)

#define DPU_DEBUG_BTS(fmt, args...)\
	do {\
		if (dpu_bts_log_level >= 8)\
			dpu_debug_printk("BTS", fmt,  ##args);\
	} while (0)

#define DPU_LOG_BTS(fmt, args...)\
	do {\
		if (dpu_bts_log_level >= 7)\
			dpu_debug_printk("BTS", fmt,  ##args);\
	} while (0)

#define DPU_INFO_BTS(fmt, args...)\
	do {\
		if (dpu_bts_log_level >= 6)\
			dpu_debug_printk("BTS", fmt,  ##args);\
	} while (0)

#define DPU_ERR_BTS(fmt, args...)\
	do {\
		if (dpu_bts_log_level >= 3)\
			dpu_debug_printk("BTS", fmt, ##args);\
	} while (0)

enum decon_trig_mode {
	DECON_HW_TRIG = 0,
	DECON_SW_TRIG
};

enum decon_out_type {
	DECON_OUT_DSI = 0,
	DECON_OUT_EDP,
	DECON_OUT_DP,
	DECON_OUT_WB
};

enum decon_dsi_mode {
	DSI_MODE_SINGLE = 0,
	DSI_MODE_DUAL_DSI,
	DSI_MODE_DUAL_DISPLAY,
	DSI_MODE_NONE
};

enum decon_hold_scheme {
	/*  should be set to this value in case of DSIM video mode */
	DECON_VCLK_HOLD_ONLY		= 0x00,
	/*  should be set to this value in case of DSIM command mode */
	DECON_VCLK_RUNNING_VDEN_DISABLE = 0x01,
	DECON_VCLK_HOLD_VDEN_DISABLE	= 0x02,
	/*  should be set to this value in case of HDMI, eDP */
	DECON_VCLK_NOT_AFFECTED		= 0x03,
};

enum decon_rgb_order {
	DECON_RGB = 0x0,
	DECON_GBR = 0x1,
	DECON_BRG = 0x2,
	DECON_BGR = 0x4,
	DECON_RBG = 0x5,
	DECON_GRB = 0x6,
};

enum decon_win_func {
	PD_FUNC_CLEAR			= 0x0,
	PD_FUNC_COPY			= 0x1,
	PD_FUNC_DESTINATION		= 0x2,
	PD_FUNC_SOURCE_OVER		= 0x3,
	PD_FUNC_DESTINATION_OVER	= 0x4,
	PD_FUNC_SOURCE_IN		= 0x5,
	PD_FUNC_DESTINATION_IN		= 0x6,
	PD_FUNC_SOURCE_OUT		= 0x7,
	PD_FUNC_DESTINATION_OUT		= 0x8,
	PD_FUNC_SOURCE_A_TOP		= 0x9,
	PD_FUNC_DESTINATION_A_TOP	= 0xa,
	PD_FUNC_XOR			= 0xb,
	PD_FUNC_PLUS			= 0xc,
	PD_FUNC_USER_DEFINED		= 0xd,
};

/* truncated: file unchanged after the fixed lines */

#if defined(CONFIG_DECON_EVENT_LOG)
/* ... */
#define DPU_EVENT_START() ktime_t start = ktime_get()
void DPU_EVENT_LOG(dpu_event_t type, struct v4l2_subdev *sd, ktime_t time);
void DPU_EVENT_LOG_WINCON(struct v4l2_subdev *sd, struct decon_reg_data *regs);
void DPU_EVENT_LOG_FENCE(struct v4l2_subdev *sd, struct decon_reg_data *regs, dpu_event_t type);
void DPU_EVENT_LOG_CMD(struct v4l2_subdev *sd, u32 cmd_id, unsigned long data, u32 size);
void DPU_EVENT_SHOW(struct seq_file *s, struct decon_device *decon);
void DPU_EVENT_LOG_WIN_CONFIG(struct v4l2_subdev *sd, struct decon_win_config_data *win_data);
void DPU_EVENT_LOG_INSTANT_OFF(struct v4l2_subdev *sd);
int decon_create_debugfs(struct decon_device *decon);
void decon_destroy_debugfs(struct decon_device *decon);
#else /*!*/
#define DPU_EVENT_START(...) do { } while(0)
#define DPU_EVENT_LOG(...) do { } while(0)
#define DPU_EVENT_LOG_WINCON(...) do { } while(0)
#define DPU_EVENT_LOG_FENCE(...) do { } while (0)
#define DPU_EVENT_LOG_CMD(...) do { } while(0)
#define DPU_EVENT_SHOW(...) do { } while(0)
#define DPU_EVENT_LOG_WIN_CONFIG(...) do { } while(0)
#define DPU_EVENT_LOG_INSTANT_OFF(...) do { } while (0)
#define decon_create_debugfs(...) do { } while(0)
#define decon_destroy_debugfs(...) do { } while(0)
#endif

/* rest of file unchanged */
#endif /* ___SAMSUNG_DECON_H__
 */
