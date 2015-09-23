/*
 *  linux/drivers/video/fb/rgbled-fb.c
 *
 *  (c) Martin Sperl <kernel@martin.sperl.org>
 *
 *  generic Frame buffer code for LED strips
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __RGBLED_FB_H
#define __RGBLED_FB_H

#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/spinlock.h>

struct rgbled_pixel {
	u8			red;
	u8			green;
	u8			blue;
	u8			brightness;
};

enum rgbled_pixeltype {
	rgbled_pixeltype_red,
	rgbled_pixeltype_green,
	rgbled_pixeltype_blue,
	rgbled_pixeltype_brightness
};

struct rgbled_coordinates {
	int			x;
	int			y;
};

struct rgbled_board_info;

struct rgbled_fb {
	struct fb_info		*info;
	struct fb_deferred_io	deferred_io;

	struct list_head	boards;
	spinlock_t		lock;
	void			*par;
	const char		*name;
	struct device_node	*of_node;
	bool			duplicate_id;

	char __iomem		*vmem;
	int			width;
	int			height;
	int			vmem_size;

	int			pixel;

	void (*deferred_work)(struct rgbled_fb*);
	void (*finish_work)(struct rgbled_fb*);

	void (*setPixelValue)(struct rgbled_fb *rfb,
			      struct rgbled_board_info *board,
			      int pixel_num,
			      struct rgbled_pixel *pix);
	void (*getPixelValue)(struct rgbled_fb *rfb,
			      struct rgbled_board_info *board,
			      struct rgbled_coordinates * coord,
			      struct rgbled_pixel *pix);

	/* current estimates */
	u32			current_limit;
	u32			current_current;
	u32			current_current_tmp;
	u32			current_max;

	u32			max_current_red;
	u32			max_current_green;
	u32			max_current_blue;

	u8			brightness;

	u32			screen_updates;
};

struct rgbled_board_info {
	const char     		*compatible;
	const char		*name;
	u32			id;
	u32			x;
	u32			y;
	u32			width;
	u32			height;
	u32			pixel;

	u32			pitch;

	bool			layout_yx;
	bool			inverted_x;
	bool			inverted_y;

	u32			flags;
#define RGBLED_FLAG_CHANGE_WIDTH	BIT(0)
#define RGBLED_FLAG_CHANGE_HEIGHT	BIT(1)
#define RGBLED_FLAG_CHANGE_PITCH	BIT(2)
#define RGBLED_FLAG_CHANGE_LAYOUT	BIT(3)
#define RGBLED_FLAG_CHANGE_WHL  	(RGBLED_FLAG_CHANGE_WIDTH |  \
					 RGBLED_FLAG_CHANGE_HEIGHT | \
					 RGBLED_FLAG_CHANGE_LAYOUT)
#define RGBLED_FLAG_CHANGE_WHLP  	(RGBLED_FLAG_CHANGE_WHL |    \
					 RGBLED_FLAG_CHANGE_PITCH)

	int (*multiple)(struct rgbled_board_info *board, u32 val);

	void (*getPixelCoords)(struct rgbled_fb *rfb,
			       struct rgbled_board_info *board,
			       int pixel_num,
			       struct rgbled_coordinates *coord);
	void (*getPixelValue)(struct rgbled_fb *rfb,
			      struct rgbled_board_info *board,
			      struct rgbled_coordinates * coord,
			      struct rgbled_pixel *pix);

	/* current estimates */
	u32			current_limit;
	u32			current_current;
	u32			current_current_tmp;
	u32			current_max;

	/* the default brightness */
	u8			brightness;

	struct device_node	*of_node;
	struct list_head 	list;
};

extern int rgbled_board_multiple_width(struct rgbled_board_info *board, u32 val);
extern int rgbled_board_multiple_height(struct rgbled_board_info *board, u32 val);

/* typical pixel handler */
extern void rgbled_getPixelCoords_linear(
		struct rgbled_fb *rfb,
		struct rgbled_board_info *board,
		int pixel_num,
		struct rgbled_coordinates *coord);

extern void rgbled_getPixelCoords_meander(
		struct rgbled_fb *rfb,
		struct rgbled_board_info *board,
		int pixel_num,
		struct rgbled_coordinates *coord);

extern struct rgbled_fb *rgbled_alloc(struct device *dev,
				      const char *name,
				      struct rgbled_board_info *boards);
extern int rgbled_register(struct rgbled_fb *fb);


/* mostly internal functions - not exported */
extern int rgbled_register_of(struct rgbled_fb *rfb);
extern int rgbled_scan_boards_of(struct rgbled_fb *rfb,
				 struct rgbled_board_info *boards);
extern int rgbled_register_sysfs(struct rgbled_fb *rfb);
extern int rgbled_register_boards(struct rgbled_fb *rfb);
extern struct rgbled_pixel *rgbled_getPixel(struct rgbled_fb *rfb,
					    struct rgbled_coordinates *coord);

extern void rgbled_schedule(struct fb_info *info);

#endif /* __RGBLED_FB_H */
