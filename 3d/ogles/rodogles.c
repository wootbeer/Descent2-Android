//
//  RODOGLES.c
//  3D
//
//

#ifdef OGLES
#include "rodogles.h"
#include "oglestex.h"

extern int Sprite_blend_mode;	// main/menu.c, Video Options "Sprite Blending"

bool g3_draw_bitmap_ogles(g3s_point *pos, fix width, fix height, grs_bitmap *bm) {
	GLfloat x0f, y0f, x1f, y1f, zf;
	
	// Calculate OGLES coords
	x0f = f2fl(pos->x - width / 2);
	y0f = f2fl(pos->y - height / 2);
	x1f = f2fl(pos->x + width / 2);
	y1f = f2fl(pos->y + height / 2);
	zf = -f2fl(pos->z);
	
	// Draw
	GLfloat vertices[] = { x1f, y0f, zf, x0f, y0f, zf, x0f, y1f, zf, x1f, y1f, zf};
	GLfloat texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
	// Per-vertex white instead of glColor4f(): every other 3D draw (walls, robots -- see
	// g3_draw_tmap_ogles()) feeds a color array, and GLES1 on Android is emulated with
	// shaders the driver generates per state combination. Flipping between "color array" and
	// "constant color" for every sprite made the Adreno 750 (Snapdragon 8 Gen 3) driver stall
	// on each one -- 60fps dropping to 12-20 whenever an explosion or laser hit was on screen.
	// Same result on screen, same state as the walls.
	GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f };
	// "Sprite Blending" option: sprites are either fully solid or fully see-through, so
	// blending and the alpha test each make the transparent parts disappear on their own.
	// Some drivers are slow with one of them, so let the player turn either off.
	//   1 Blend Off: alpha test only.
	//   2 Alpha Off: blending only; no depth writes so the see-through corners can't hide
	//     things behind (objects are already drawn far-to-near).
	GLboolean was_blend = glIsEnabled(GL_BLEND);
	GLboolean was_alpha = glIsEnabled(GL_ALPHA_TEST);
	if (Sprite_blend_mode == 1) {
		glDisable(GL_BLEND);
	} else if (Sprite_blend_mode == 2) {
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
	}
	glEnable(GL_TEXTURE_2D);
	ogles_bm_bind_teximage_2d(bm);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);	// leave the current color as before, for anything that relies on it
	if (Sprite_blend_mode == 1) {
		if (was_blend) glEnable(GL_BLEND);
	} else if (Sprite_blend_mode == 2) {
		if (was_alpha) glEnable(GL_ALPHA_TEST);
		glDepthMask(GL_TRUE);
	}
	return 0;
}

#endif