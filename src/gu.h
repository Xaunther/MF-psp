/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#pragma once

#include <psptypes.h>

#define FONT_WIDTH 6
#define FONT_HEIGHT 10

#define GBA_SCREEN_WIDTH 240
#define GBA_SCREEN_HEIGHT 160

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272

#define FRAME_GAME 0
#define FRAME_MENU 1

void init_video();
void video_resolution(u32 mode);
void clear_screen(u16 color);
void blit_to_screen(u16 *src, u32 w, u32 h, u32 x, u32 y);
u16 *copy_screen();
void flip_screen();
void video_read_mem_savestate(u32 ver);
void video_write_mem_savestate(u32 ver);
void video_get_size_savestate(u32 ver);

void debug_screen_clear();
void debug_screen_start();
void debug_screen_end();
void debug_screen_printf(const char *format, ...);
void debug_screen_printl(const char *format, ...);
void debug_screen_newline(u32 count);
void debug_screen_update();

enum video_scale_type
{
    unscaled,
    scaled_aspect,
    fullscreen,
    option1,
    option2,
};

enum video_filter_type
{
    filter_nearest,
    filter_bilinear
};

enum VIDEO_INTERLACE_TYPE
{
    PROGRESSIVE,
    INTERLACE
};

enum VIDEO_RATIO_TYPE
{
    R4_3,
    R16_9
};

struct SPRITE
{
    float u1;
    float v1;
    float x1;
    float y1;
    float z1;
    float u2;
    float v2;
    float x2;
    float y2;
    float z2;
};

struct VIDEO_OUT_PARAMETER
{
    int u;
    int displaymode;
    int width;
    int height;
    int x;
    int y;
    int z;
};

struct VIEW_PORT
{
    int x;
    int y;
    int width;
    int height;
};

struct TEXTURE_BIT
{
    u32 x;
    u32 y;
};

struct TEXTURE_SIZE
{
    u32 pitch;
    u32 width;
    u32 height;
};

struct SCREEN_SIZE
{
    u32 width;
    u32 height;
};

struct SCREEN_PARAMETER
{
    VIDEO_OUT_PARAMETER video_out; /* Parameter of pspDveMgrSetVideoOut */
    int filter[2];                 /* Filer when MENU is displayed */
    TEXTURE_SIZE texture_size;     /* Texture size */
    TEXTURE_BIT texture_bit;       /* Number of vertical and horizontal bits of the texture */
    SCREEN_SIZE screen_size;       /* Display buffer size */
    VIEW_PORT view;                /* Display range */
    SPRITE screen_setting_1;       /* Sprite data 1 */
    SPRITE screen_setting_2;       /* Spirte data 2 */
};

extern float *temp_vertex;
extern u16 *vram_data;

extern u16 *screen_address;
extern u32 screen_pitch;
extern u32 screen_width;
extern u32 screen_height;
extern u32 screen_width2;
extern u32 screen_height2;
extern u32 video_out_mode;

extern u32 current_scale;

extern const SCREEN_PARAMETER screen_parameter_psp_game_init[];
extern const SCREEN_PARAMETER screen_parameter_psp_menu_init;

extern const SCREEN_PARAMETER screen_parameter_composite_game_init[];
extern const SCREEN_PARAMETER screen_parameter_composite_menu_init[];

extern const SCREEN_PARAMETER screen_parameter_component_game_init[];
extern const SCREEN_PARAMETER screen_parameter_component_menu_init[];

extern SCREEN_PARAMETER *current_parameter;

void set_resolution_parameter_game(video_scale_type scale);
void set_resolution_parameter_menu();

#define UNIVERSAL_VRAM_ADDR (0x441A5C00)
