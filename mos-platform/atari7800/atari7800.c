/* This file implements only the zone-height-INDEPENDENT functions declared
 * in atari7800.h; the zone-height-dependent scene API (which must compile
 * fresh per program to honor that program's own ATARI7800_ZONE_HEIGHT) lives
 * entirely in the header now -- see the comment on ATARI7800_NO_SCENE_IMPL
 * there for why this file must not also pull that block in. */
#define ATARI7800_NO_SCENE_IMPL
#include <atari7800.h>

/**
 * Initializes the baseline hardware configuration of the Atari 7800.
 * Locks the console in 7800 mode, turns off DMA during initial setup,
 * and clears screen offsets.
 */
void atari7800_init_system(void) {
  ATARI7800_INPTCTRL = 0x07;
  ATARI7800_CTRL = 0x7f;
  ATARI7800_OFFSET = 0x00;
  ATARI7800_INPTCTRL = 0x00;
}

/**
 * Waits for the start of the next vertical blanking (VBLANK) interval.
 * Uses a double-waiting loop pattern to synchronize accurately with raster timing.
 */
void atari7800_wait_vblank(void) {
  while ((ATARI7800_MSTAT & 0x80u) != 0) {
  }
  while ((ATARI7800_MSTAT & 0x80u) == 0) {
  }
}

/**
 * Configures the MARIA display list address, waiting for VBLANK to set
 * video mode (CTRL) and background color registers safely.
 */
void atari7800_configure_video(uint16_t display_list_addr, uint8_t ctrl,
                               uint8_t bgcolor) {
  atari7800_set_display_list(display_list_addr);
  atari7800_wait_vblank();
  ATARI7800_CTRL = ctrl;
  ATARI7800_BACKGRND = bgcolor;
}

/**
 * Startup helper that configures standard 160A wide-pixel direct rendering mode.
 */
void atari7800_init_160a(uint16_t display_list_addr, uint8_t bgcolor) {
  atari7800_init_system();
  atari7800_configure_video(display_list_addr,
                             ATARI7800_CTRL_MODE_160A |
                                 ATARI7800_CTRL_DMA_ENABLE,
                             bgcolor);
}

/**
 * Sets three 8-bit colors for a specified MARIA palette slot (0-7).
 */
void atari7800_set_palette3(uint8_t palette_index, atari7800_palette3_t colors) {
  volatile uint8_t *base;

  if (palette_index > 7u) {
    return;
  }

  base = (volatile uint8_t *)(uintptr_t)(ATARI7800_REG_P0C1 +
                                         (uint16_t)palette_index * 4u);
  base[0] = colors.c1;
  base[1] = colors.c2;
  base[2] = colors.c3;
}

/**
 * Clears the specified zone buffer by filling it with zeros.
 */
void atari7800_maria_clear_zone(uint8_t *zone, uint16_t zone_size) {
  uint16_t i;

  for (i = 0; i < zone_size; ++i) {
    zone[i] = 0x00u;
  }
}

/**
 * Writes a 5-byte Direct Mode sprite header followed by two null terminator bytes
 * into the zone buffer at a specific object index.
 */
uint8_t atari7800_maria_plot_sprite_zone5(uint8_t *zone, uint16_t zone_size,
                                          uint8_t object_index,
                                          uint16_t sprite_addr, uint8_t mode,
                                          uint8_t palette,
                                          uint8_t width_twos_comp,
                                          uint8_t x_pos) {
  const uint16_t start =
      (uint16_t)object_index * ATARI7800_MARIA_ZONE5_OBJECT_BYTES;
  const uint16_t end =
      start + ATARI7800_MARIA_ZONE5_OBJECT_BYTES +
      ATARI7800_MARIA_ZONE5_TERMINATOR_BYTES;

  if (end > zone_size) {
    return ATARI7800_ERR_BUDGET_FULL;
  }

  zone[start] = (uint8_t)(sprite_addr & 0xffu);
  zone[(uint16_t)(start + 1u)] = mode;
  zone[(uint16_t)(start + 2u)] = (uint8_t)(sprite_addr >> 8);
  zone[(uint16_t)(start + 3u)] =
      atari7800_maria_pal_width(palette, width_twos_comp);
  zone[(uint16_t)(start + 4u)] = x_pos;
  zone[(uint16_t)(start + 5u)] = 0x00u;
  zone[(uint16_t)(start + 6u)] = 0x00u;
  return ATARI7800_OK;
}

/**
 * Resolves a sprite asset descriptor's data pointer and invokes the base plot function.
 */
uint8_t atari7800_maria_plot_sprite_asset_zone5(
    uint8_t *zone, uint16_t zone_size, uint8_t object_index,
    const atari7800_sprite_asset_t *asset, uint8_t x_pos) {
  uint16_t sprite_addr;

  if (asset == 0 || asset->data == 0) {
    return ATARI7800_ERR_INVALID;
  }

  sprite_addr = atari7800_ptr16(asset->data);

  return atari7800_maria_plot_sprite_zone5(zone, zone_size, object_index,
                                           sprite_addr, asset->mode,
                                           asset->palette,
                                           asset->width_twos_comp, x_pos);
}

/**
 * Rewrites an already-placed 5-byte Direct Mode object header in place,
 * touching only its own 5 bytes -- unlike atari7800_maria_plot_sprite_zone5,
 * it does NOT zero the 2 bytes after it. Those bytes either belong to the
 * next occupied slot's header (zeroing them would corrupt that object) or
 * are that slot's already-correct list terminator (nothing to fix). Use
 * this to update a slot that was already terminated correctly when it was
 * first placed; use atari7800_maria_plot_sprite_zone5 only for placing into
 * a slot for the first time, so its terminator gets written once.
 */
uint8_t atari7800_maria_patch_sprite_zone5(uint8_t *zone, uint16_t zone_size,
                                           uint8_t object_index,
                                           uint16_t sprite_addr, uint8_t mode,
                                           uint8_t palette,
                                           uint8_t width_twos_comp,
                                           uint8_t x_pos) {
  const uint16_t start =
      (uint16_t)object_index * ATARI7800_MARIA_ZONE5_OBJECT_BYTES;
  const uint16_t end = start + ATARI7800_MARIA_ZONE5_OBJECT_BYTES;

  if (end > zone_size) {
    return ATARI7800_ERR_BUDGET_FULL;
  }

  zone[start] = (uint8_t)(sprite_addr & 0xffu);
  zone[(uint16_t)(start + 1u)] = mode;
  zone[(uint16_t)(start + 2u)] = (uint8_t)(sprite_addr >> 8);
  zone[(uint16_t)(start + 3u)] =
      atari7800_maria_pal_width(palette, width_twos_comp);
  zone[(uint16_t)(start + 4u)] = x_pos;
  return ATARI7800_OK;
}

/**
 * Resolves a sprite asset descriptor's data pointer and invokes
 * atari7800_maria_patch_sprite_zone5.
 */
uint8_t atari7800_maria_patch_sprite_asset_zone5(
    uint8_t *zone, uint16_t zone_size, uint8_t object_index,
    const atari7800_sprite_asset_t *asset, uint8_t x_pos) {
  uint16_t sprite_addr;

  if (asset == 0 || asset->data == 0) {
    return ATARI7800_ERR_INVALID;
  }

  sprite_addr = atari7800_ptr16(asset->data);

  return atari7800_maria_patch_sprite_zone5(zone, zone_size, object_index,
                                            sprite_addr, asset->mode,
                                            asset->palette,
                                            asset->width_twos_comp, x_pos);
}

/**
 * Set a color palette inside a managed scene.
 */
void atari7800_scene_set_palette(atari7800_scene_t *scene,
                                 uint8_t palette_index,
                                 atari7800_palette3_t colors) {
  (void)scene;
  atari7800_set_palette3(palette_index, colors);
}

/**
 * Resolves a single-line string into a glyph run once (e.g. at startup, or
 * whenever the displayed text changes) so that per-frame redraws can skip
 * character-to-glyph lookup and whitespace/newline branching entirely.
 * Stops at the first '\n' or the string terminator; use one run per line
 * for multi-line HUD text.
 */
uint8_t atari7800_build_glyph_run(const atari7800_font_descriptor_t *font,
                                  const char *text,
                                  atari7800_glyph_run_entry_t *entries,
                                  uint8_t max_entries,
                                  atari7800_glyph_run_t *out_run) {
  uint8_t pen_x = 0;
  uint8_t count = 0;

  if (font == 0 || text == 0 || entries == 0 || out_run == 0 ||
      font->glyph_count == 0u || font->glyph_advance == 0u) {
    return ATARI7800_ERR_INVALID;
  }

  const uint8_t glyph_advance = font->glyph_advance;
  const uint8_t space_advance =
      (font->space_advance != 0u) ? font->space_advance : font->glyph_advance;
  const uint8_t * const char_to_glyph = font->char_to_glyph;
  const uint8_t char_to_glyph_len = font->char_to_glyph_len;
  const uint8_t first_char = font->first_char;
  const uint8_t glyph_count = font->glyph_count;

  while (*text != '\0' && *text != '\n') {
    const uint8_t ch = (uint8_t)*text;

    if (ch == (uint8_t)' ') {
      pen_x = (uint8_t)(pen_x + space_advance);
      ++text;
      continue;
    }

    {
      uint8_t glyph_index = ATARI7800_GLYPH_RUN_BLANK;

      if (char_to_glyph != 0 && ch < char_to_glyph_len) {
        glyph_index = char_to_glyph[ch];
      } else if (ch >= first_char) {
        glyph_index = (uint8_t)(ch - first_char);
      }

      if (glyph_index < glyph_count) {
        if (count >= max_entries) {
          return ATARI7800_ERR_BUDGET_FULL;
        }
        entries[count].glyph_index = glyph_index;
        entries[count].x_offset = pen_x;
        ++count;
      }
    }

    pen_x = (uint8_t)(pen_x + glyph_advance);
    ++text;
  }

  out_run->entries = entries;
  out_run->count = count;
  return ATARI7800_OK;
}
