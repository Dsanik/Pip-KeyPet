/* Procedural 3D-ish eye renderer: pure C, no LVGL, no platform code.
   Each eye is a lit ellipsoid seen orthographically, with the pupil painted on its surface,
   plus flat 2D cuts (lid, smile) for expressions. */
#pragma once
#include <stdint.h>

typedef struct {
    float x, y;              /* center on screen, px */
    float w, h, depth;       /* ellipsoid size, px */
    float yaw, pitch, roll;  /* orientation, rad: yaw + faces right, pitch + faces down, roll + clockwise */
    float taper;             /* egg shape: + narrower at the top, -0.6..0.6 */
    float lid;               /* upper lid closure, 0..1 of the eye height */
    float tilt;              /* lid slope (px/px in the eye frame), + lower toward +x */
    float smile;             /* 0..1, lower edge cut by an arc -> ^ ^ */
    float look_x, look_y;    /* pupil direction on the ball relative to its front, rad */
    float pw, ph;            /* pupil half size, fraction of the ball radius */
    float square;            /* pupil shape: 0 round .. 1 square */
    float heart;             /* pupil morph into a heart, 0..1, also tints it to heart_color */
    float wobble, wob_t;     /* living outline: amplitude px, and its own clock in s */
    float glint;             /* white catchlights in the pupil, 0..1 */
    float spiral;            /* hypnotic spiral pupil, 0..1 */
    float veins;             /* red veins on the white, 0..1 */
    float wet;               /* tears welling up on the lower lid, 0..1 */
} eye_t;

typedef struct { int x1, y1, x2, y2; } face_rect_t; /* inclusive; empty when x2 < x1 */

typedef struct {
    eye_t eye[2];
    uint16_t color, pupil_color, heart_color; /* RGB565 */
    float shading;               /* strength of the soft shadow crescent on the lower right, 0 = flat */
    float blush;                 /* pink cheeks under the eyes, 0..1 */
    uint16_t blush_color, vein_color, water_color;
    uint8_t swapped;             /* buf holds byte-swapped RGB565 (the panel's order); colours above stay plain */
    face_rect_t drawn[4];        /* render state: boxes painted last frame (eyes, cheeks) */
    float scar;                  /* > 0.5: the right-hand eye on screen (larger x) is shut: a closed-lid arc with a diagonal
                                    scar across it, instead of white and pupil. 0 = both eyes open (the battle moods) */
    uint16_t scar_color;         /* RGB565 of the scar; 0 = default dark red */
} face_t;

/* The peak battle moods open both eyes: RAGE, FURIOUS and BELLIGERENT. In every other mood the right eye stays shut
   with its scar. Call it whenever the mood changes (or just set f->scar yourself).
   The left eye, its blinking and all its expressions are not touched. */
#include "moods.h"
static inline void face_set_scar_for_mood(face_t *f, int mood)
{
    f->scar = (mood == PIP_RAGE || mood == PIP_FURIOUS || mood == PIP_BELLIGERENT) ? 0.0f : 1.0f;
}

/* Clears last frame's eyes and paints the new ones into a w*h RGB565 buffer on black.
   Returns the area that changed. */
face_rect_t face_draw(face_t *f, uint16_t *buf, int w, int h);

/* Optional: run job(ctx, y0, y1) over rows y0..y1-1 split between cores and return when all is done.
   Bands can be any split of the range; the result is the same. NULL = one core. */
/* Optional (firmware): draw a frame in the background. face_start returns at once with face_busy set; it drops to 0
   when the frame is in buf. The caller must not touch f or buf meanwhile. NULL = draw in place, synchronously. */
extern void (*face_start)(face_t *f, uint16_t *buf, int w, int h);
extern volatile int face_busy;

extern void (*face_parallel)(void (*job)(void *ctx, int y0, int y1), void *ctx, int y0, int y1);
