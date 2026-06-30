#include <atari7800.h>

/* Null zone display list header used to terminate empty zones. */
static atari7800_maria_null_header_t atari7800_scene_null_zone;

/* The Display List List (DLL) pointing to all zones on the screen. */
static atari7800_maria_dll_entry_t
    atari7800_scene_display_list[ATARI7800_MARIA_NTSC_DLL_ENTRIES];

/* Low-RAM and High-RAM display list zone buffers split to bypass physical memory shadow pages. */
static uint8_t atari7800_scene_zones_low[14][ATARI7800_SCENE_ZONE_BYTES]
    __attribute__((section(".scene_zones_low")));
static uint8_t atari7800_scene_zones_high[14][ATARI7800_SCENE_ZONE_BYTES]
    __attribute__((section(".scene_zones_high")));

/* Read-only lookup table to access zone pointers by index (0-27). */
static uint8_t * const atari7800_scene_zones[ATARI7800_SCENE_VISIBLE_ZONES] = {
  &atari7800_scene_zones_low[0][0],
  &atari7800_scene_zones_low[1][0],
  &atari7800_scene_zones_low[2][0],
  &atari7800_scene_zones_low[3][0],
  &atari7800_scene_zones_low[4][0],
  &atari7800_scene_zones_low[5][0],
  &atari7800_scene_zones_low[6][0],
  &atari7800_scene_zones_low[7][0],
  &atari7800_scene_zones_low[8][0],
  &atari7800_scene_zones_low[9][0],
  &atari7800_scene_zones_low[10][0],
  &atari7800_scene_zones_low[11][0],
  &atari7800_scene_zones_low[12][0],
  &atari7800_scene_zones_low[13][0],
  &atari7800_scene_zones_high[0][0],
  &atari7800_scene_zones_high[1][0],
  &atari7800_scene_zones_high[2][0],
  &atari7800_scene_zones_high[3][0],
  &atari7800_scene_zones_high[4][0],
  &atari7800_scene_zones_high[5][0],
  &atari7800_scene_zones_high[6][0],
  &atari7800_scene_zones_high[7][0],
  &atari7800_scene_zones_high[8][0],
  &atari7800_scene_zones_high[9][0],
  &atari7800_scene_zones_high[10][0],
  &atari7800_scene_zones_high[11][0],
  &atari7800_scene_zones_high[12][0],
  &atari7800_scene_zones_high[13][0],
};

/* Tracker for the next available object slot index (0-12) inside each zone display list. */
static uint8_t atari7800_scene_zone_next_object[ATARI7800_SCENE_VISIBLE_ZONES];

/* Tracker for active zones in the previous frame, used to identify and clean up inactive zones. */
static uint8_t atari7800_scene_active_zones_prev[ATARI7800_SCENE_VISIBLE_ZONES];

/* Tracker for active zones in the current frame. */
static uint8_t atari7800_scene_active_zones_curr[ATARI7800_SCENE_VISIBLE_ZONES];

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
 * Populates a blank Display List List (DLL) for NTSC display timing.
 * Reserves top, visible screen (28 zones), and bottom blank boundaries.
 */
void atari7800_maria_build_blank_ntsc(
    atari7800_maria_dll_entry_t *display_list,
    const atari7800_maria_null_header_t *zone_header) {
  uint8_t i;

  atari7800_maria_init_dll_entry(&display_list[0], 8, zone_header, 0);

  for (i = 0; i < 28; ++i) {
    atari7800_maria_init_dll_entry(&display_list[1u + i], 7, zone_header, 0);
  }

  atari7800_maria_init_dll_entry(&display_list[29], 15, zone_header, 0);
  atari7800_maria_init_dll_entry(&display_list[30], 9, zone_header, 0);
  atari7800_maria_init_dll_entry(&display_list[31], 15, zone_header, 0);
  atari7800_maria_init_dll_entry(&display_list[32], 8, zone_header, 0);
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
    return 0u;
  }

  zone[start] = (uint8_t)(sprite_addr & 0xffu);
  zone[(uint16_t)(start + 1u)] = mode;
  zone[(uint16_t)(start + 2u)] = (uint8_t)(sprite_addr >> 8);
  zone[(uint16_t)(start + 3u)] =
      atari7800_maria_pal_width(palette, width_twos_comp);
  zone[(uint16_t)(start + 4u)] = x_pos;
  zone[(uint16_t)(start + 5u)] = 0x00u;
  zone[(uint16_t)(start + 6u)] = 0x00u;
  return 1u;
}

/**
 * Resolves a sprite asset descriptor's data pointer and invokes the base plot function.
 */
uint8_t atari7800_maria_plot_sprite_asset_zone5(
    uint8_t *zone, uint16_t zone_size, uint8_t object_index,
    const atari7800_sprite_asset_t *asset, uint8_t x_pos) {
  uint16_t sprite_addr;

  if (asset == 0 || asset->data == 0) {
    return 0u;
  }

  sprite_addr = atari7800_ptr16(asset->data);

  return atari7800_maria_plot_sprite_zone5(zone, zone_size, object_index,
                                           sprite_addr, asset->mode,
                                           asset->palette,
                                           asset->width_twos_comp, x_pos);
}

/**
 * Initializes the stateful scene manager, building the DLL structure
 * and clearing the low/high zone buffers.
 */
void atari7800_scene_init_160a(atari7800_scene_t *scene, uint8_t bgcolor) {
  uint8_t zone_index;

  atari7800_maria_init_null_header(&atari7800_scene_null_zone, 0u);
  atari7800_maria_build_blank_ntsc(atari7800_scene_display_list,
                                   &atari7800_scene_null_zone);
  for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES; ++zone_index) {
    atari7800_scene_active_zones_prev[zone_index] = 0u;
    atari7800_scene_active_zones_curr[zone_index] = 0u;
  }

  for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
       ++zone_index) {
    atari7800_scene_zone_next_object[zone_index] = 0u;
    atari7800_maria_clear_zone(atari7800_scene_zones[zone_index],
                               ATARI7800_SCENE_ZONE_BYTES);
  }

  atari7800_scene_begin_frame(scene);
  atari7800_init_160a(atari7800_ptr16(atari7800_scene_display_list), bgcolor);
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
 * Resets per-zone object cursors at the beginning of a frame.
 */
void atari7800_scene_begin_frame(atari7800_scene_t *scene) {
  (void)scene;

  for (uint8_t z_idx = 0; z_idx < ATARI7800_SCENE_VISIBLE_ZONES; ++z_idx) {
    atari7800_scene_active_zones_curr[z_idx] = 0u;
  }

  for (uint8_t zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
       ++zone_index) {
    atari7800_scene_zone_next_object[zone_index] = 0u;
  }
}

/**
 * Clears display lists of zones that were active in the previous frame
 * but went unused in the current frame, preventing flickering or trail artifacts.
 */
void atari7800_scene_end_frame(atari7800_scene_t *scene) {
  uint8_t zone_index;
  (void)scene;

  for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
       ++zone_index) {
    if (atari7800_scene_active_zones_prev[zone_index] != 0u &&
        atari7800_scene_active_zones_curr[zone_index] == 0u) {
      atari7800_maria_clear_zone(atari7800_scene_zones[zone_index],
                                 ATARI7800_SCENE_ZONE_BYTES);
      atari7800_maria_init_dll_entry(
          &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], 7u,
          &atari7800_scene_null_zone, 0u);
    }
    atari7800_scene_active_zones_prev[zone_index] =
        atari7800_scene_active_zones_curr[zone_index];
  }
}

/**
 * Draws a sprite asset at coordinates (x, y). Matches the vertical coordinate
 * to the appropriate 8-line screen zone, checks for buffer overflows, plots the
 * 5-byte header, and links the zone to the active display list.
 */
uint8_t atari7800_scene_draw_sprite(atari7800_scene_t *scene,
                                    const atari7800_sprite_asset_t *asset,
                                    uint8_t x_pos, uint8_t y_pos) {
  uint8_t zone_index;
  uint8_t object_index;
  uint8_t *zone;

  if (scene == 0 || asset == 0) {
    return 0u;
  }

  if (scene->initialized == 0u) {
    object_index = scene->next_object;
    if (!atari7800_plotsprite_asset(scene->zone, scene->zone_size, object_index,
                                    asset, x_pos)) {
      return 0u;
    }
    scene->next_object = (uint8_t)(scene->next_object + 1u);
    return 1u;
  }

  zone_index = (uint8_t)(y_pos >> 3);
  if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
    zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
  }

  zone = atari7800_scene_zones[zone_index];
  object_index = atari7800_scene_zone_next_object[zone_index];

  if (!atari7800_plotsprite_asset(zone, ATARI7800_SCENE_ZONE_BYTES,
                                  object_index, asset, x_pos)) {
    return 0u;
  }

  if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
    atari7800_maria_init_dll_entry(
        &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], 7u, zone,
        0u);
  }

  atari7800_scene_zone_next_object[zone_index] = (uint8_t)(object_index + 1u);
  atari7800_scene_active_zones_curr[zone_index] = 1u;
  return 1u;
}

/**
 * Draws a string of characters at coordinates (x, y). Highly optimized to fit
 * within the strict NTSC VBLANK window: caches font parameters in registers,
 * uses 8-bit offset indexing, loops cleanly over character glyphs, and plots
 * the 5-byte headers directly.
 */
uint8_t atari7800_scene_draw_text(atari7800_scene_t *scene,
                                  const atari7800_font_descriptor_t *font,
                                  uint8_t x_pos, uint8_t y_pos,
                                  const char *text) {
  uint8_t pen_x = x_pos;
  uint8_t pen_y = y_pos;
  const uint8_t start_x = x_pos;

  if (scene == 0 || font == 0 || text == 0 || font->data == 0 ||
      font->glyph_count == 0u || font->glyph_advance == 0u) {
    return 0u;
  }

  /* Cache font properties into local variables (fast register allocation) */
  const uint16_t font_data_addr = atari7800_ptr16(font->data);
  const uint8_t glyph_mode = font->glyph_mode;
  const uint8_t pal_width = atari7800_maria_pal_width(font->glyph_palette, font->glyph_width_twos_comp);
  const uint8_t glyph_advance = font->glyph_advance;
  const uint8_t space_advance = (font->space_advance != 0u) ? font->space_advance : font->glyph_advance;
  const uint8_t line_advance = font->line_advance;
  const uint8_t * const char_to_glyph = font->char_to_glyph;
  const uint8_t char_to_glyph_len = font->char_to_glyph_len;
  const uint8_t first_char = font->first_char;
  const uint8_t glyph_count = font->glyph_count;

  if (scene->initialized == 0u) {
    /* If scene not initialized, use the simple slow path (rarely executed) */
    while (*text != '\0') {
      const uint8_t ch = (uint8_t)*text;
      if (ch == (uint8_t)'\n') {
        pen_x = start_x;
        pen_y = (uint8_t)(pen_y + line_advance);
        ++text;
        continue;
      }
      if (ch == (uint8_t)' ') {
        pen_x += space_advance;
        ++text;
        continue;
      }
      uint8_t glyph_index = 0xffu;
      if (char_to_glyph != 0 && ch < char_to_glyph_len) {
        glyph_index = char_to_glyph[ch];
      } else if (ch >= first_char) {
        glyph_index = (uint8_t)(ch - first_char);
      }
      if (glyph_index < glyph_count) {
        uint16_t sprite_addr = font_data_addr + ((uint16_t)glyph_index << 1);
        const uint8_t obj_idx = scene->next_object;
        if (!atari7800_maria_plot_sprite_zone5(scene->zone, scene->zone_size,
                                               obj_idx, sprite_addr,
                                               glyph_mode, font->glyph_palette,
                                               font->glyph_width_twos_comp, pen_x)) {
          return 0u;
        }
        scene->next_object = (uint8_t)(scene->next_object + 1u);
      }
      pen_x = (uint8_t)(pen_x + glyph_advance);
      ++text;
    }
    return 1u;
  }

  /* Main optimized drawing loop for initialized scenes */
  uint8_t zone_index = (uint8_t)(pen_y >> 3);
  if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
    zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
  }
  uint8_t *zone = atari7800_scene_zones[zone_index];
  uint8_t object_index = atari7800_scene_zone_next_object[zone_index];
  uint8_t start_offset = (uint8_t)(object_index * 5u);

  if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
    atari7800_maria_init_dll_entry(
        &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], 7u,
        zone, 0u);
    atari7800_scene_active_zones_curr[zone_index] = 1u;
  }

  while (*text != '\0') {
    const uint8_t ch = (uint8_t)*text;

    if (ch == (uint8_t)'\n') {
      /* Write back previous zone's object cursor */
      atari7800_scene_zone_next_object[zone_index] = object_index;

      pen_x = start_x;
      pen_y = (uint8_t)(pen_y + line_advance);
      zone_index = (uint8_t)(pen_y >> 3);
      if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
        zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
      }
      zone = atari7800_scene_zones[zone_index];
      object_index = atari7800_scene_zone_next_object[zone_index];
      start_offset = (uint8_t)(object_index * 5u);

      if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
        atari7800_maria_init_dll_entry(
            &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], 7u,
            zone, 0u);
        atari7800_scene_active_zones_curr[zone_index] = 1u;
      }

      ++text;
      continue;
    }

    if (ch == (uint8_t)' ') {
      pen_x += space_advance;
      ++text;
      continue;
    }

    {
      uint8_t glyph_index = 0xffu;

      if (char_to_glyph != 0 && ch < char_to_glyph_len) {
        glyph_index = char_to_glyph[ch];
      } else if (ch >= first_char) {
        glyph_index = (uint8_t)(ch - first_char);
      }

      if (glyph_index < glyph_count) {
        uint16_t sprite_addr = font_data_addr + ((uint16_t)glyph_index << 1);

        const uint8_t end = (uint8_t)(start_offset + 7u);
        if (end > ATARI7800_SCENE_ZONE_BYTES) {
          atari7800_scene_zone_next_object[zone_index] = object_index;
          return 0u;
        }

        zone[start_offset] = (uint8_t)(sprite_addr & 0xffu);
        zone[start_offset + 1] = glyph_mode;
        zone[start_offset + 2] = (uint8_t)(sprite_addr >> 8);
        zone[start_offset + 3] = pal_width;
        zone[start_offset + 4] = pen_x;
        zone[start_offset + 5] = 0x00u;
        zone[start_offset + 6] = 0x00u;

        object_index = (uint8_t)(object_index + 1u);
        start_offset = (uint8_t)(start_offset + 5u);
      }
    }

    pen_x = (uint8_t)(pen_x + glyph_advance);
    ++text;
  }

  /* Write back final zone's object cursor */
  atari7800_scene_zone_next_object[zone_index] = object_index;
  return 1u;
}
