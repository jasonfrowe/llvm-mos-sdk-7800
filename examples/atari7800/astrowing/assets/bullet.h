#ifndef ATARI7800_ASSET_BULLET_H
#define ATARI7800_ASSET_BULLET_H

#include <stdint.h>
#include <atari7800.h>

/* Player bullet, converted by hand from astrowing.bas's bullet_conv.png
 * (4x16, indexed): the only real ink is a 2px-wide, 4-row mark at the top
 * of the image (rows 0-3, packed byte 0x14 -- pack_160a([0,1,1,0])), the
 * rest of the 16-line canvas is blank. Palette 1 ("Player Bullets"),
 * matching astrowing.bas's `plotsprite bullet_conv 1 ...`.
 *
 * Same "single reference point, fine-Y shift" layout as astrowing.c's own
 * star_sprite_data: vertically flipped (row r at page 15-r, matching every
 * other MARIA_STRIDED asset in this codebase), sized to 31 pages (0-30) so
 * draw_sprite_fine's full y&15 shift range ([shift, shift+15] for shift
 * 0-15) always keeps the real content's highest page (15) reachable. */
static const uint8_t bullet_sprite_data[31 * 256] __attribute__((aligned(256))) = {
  [12 * 256] = 0x14, /* Page 12: row 3 */
  [13 * 256] = 0x14, /* Page 13: row 2 */
  [14 * 256] = 0x14, /* Page 14: row 1 */
  [15 * 256] = 0x14, /* Page 15: row 0 (top) */
};

static const atari7800_sprite_asset_t bullet_sprite = {
  .data = bullet_sprite_data,
  .width_bytes = 1u,
  .height_lines = 8u,
  .mode = 0x40u,
  .palette = 1u,
  .width_twos_comp = 0x1fu,
  .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED
};

#endif
