#include <atari7800.h>

#define ATARI7800_SCENE_COMPACT_MAX_HEIGHT 8u
#define ATARI7800_SCENE_COMPACT_SCRATCH_BYTES \
  (ATARI7800_SCENE_COMPACT_MAX_HEIGHT * 256u)

static atari7800_maria_null_header_t atari7800_scene_null_zone;
static atari7800_maria_dll_entry_t
    atari7800_scene_display_list[ATARI7800_MARIA_NTSC_DLL_ENTRIES];
static uint8_t
    atari7800_scene_zones[ATARI7800_SCENE_VISIBLE_ZONES][ATARI7800_SCENE_ZONE_BYTES];
static uint8_t atari7800_scene_zone_next_object[ATARI7800_SCENE_VISIBLE_ZONES];
static uint8_t atari7800_scene_compact_scratch[ATARI7800_SCENE_COMPACT_SCRATCH_BYTES]
  __attribute__((aligned(256)));
static uint8_t atari7800_scene_compact_slot_next;
static uint32_t atari7800_scene_active_zones_prev;
static uint32_t atari7800_scene_active_zones_curr;

static uint16_t atari7800_scene_compact_to_maria_rows(
    const atari7800_sprite_asset_t *asset) {
  const uint8_t slot_low = atari7800_scene_compact_slot_next;
  uint8_t row;
  uint8_t col;

  if (asset->width_bytes == 0u || asset->height_lines == 0u) {
    return 0u;
  }

  if (asset->height_lines > ATARI7800_SCENE_COMPACT_MAX_HEIGHT) {
    return 0u;
  }

  if (asset->width_bytes > 32u) {
    return 0u;
  }

  if ((uint16_t)slot_low + asset->width_bytes > 256u) {
    return 0u;
  }

  atari7800_scene_compact_slot_next =
      (uint8_t)(atari7800_scene_compact_slot_next + asset->width_bytes);

  for (row = 0; row < asset->height_lines; ++row) {
    const uint16_t src_row =
        (uint16_t)(asset->height_lines - 1u - row) * asset->width_bytes;
    const uint16_t dst_row = (uint16_t)row << 8;
    for (col = 0; col < asset->width_bytes; ++col) {
      atari7800_scene_compact_scratch[(uint16_t)(dst_row + slot_low + col)] =
          asset->data[(uint16_t)(src_row + col)];
    }
  }

  return (uint16_t)(atari7800_ptr16(atari7800_scene_compact_scratch) + slot_low);
}

void atari7800_init_system(void) {
  ATARI7800_INPTCTRL = 0x07;
  ATARI7800_CTRL = 0x7f;
  ATARI7800_OFFSET = 0x00;
  ATARI7800_INPTCTRL = 0x00;
}

void atari7800_wait_vblank(void) {
  while ((ATARI7800_MSTAT & 0x80u) != 0) {
  }
  while ((ATARI7800_MSTAT & 0x80u) == 0) {
  }
}

void atari7800_configure_video(uint16_t display_list_addr, uint8_t ctrl,
                               uint8_t bgcolor) {
  atari7800_set_display_list(display_list_addr);
  atari7800_wait_vblank();
  ATARI7800_CTRL = ctrl;
  ATARI7800_BACKGRND = bgcolor;
}

void atari7800_init_160a(uint16_t display_list_addr, uint8_t bgcolor) {
  atari7800_init_system();
  atari7800_configure_video(display_list_addr,
                            ATARI7800_CTRL_MODE_160A |
                                ATARI7800_CTRL_DMA_ENABLE,
                            bgcolor);
}

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

void atari7800_maria_build_ntsc_single_zone(
    atari7800_maria_dll_entry_t *display_list,
    const atari7800_maria_null_header_t *null_zone_header,
    uint8_t visible_zone_index, const void *visible_zone_header) {
  if (visible_zone_index > 27u) {
    visible_zone_index = 0u;
  }

  atari7800_maria_build_blank_ntsc(display_list, null_zone_header);
  atari7800_maria_init_dll_entry(&display_list[(uint8_t)(1u + visible_zone_index)],
                                 7, visible_zone_header, 0);
}

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

void atari7800_maria_build_sprite_zone5(uint8_t *zone, uint16_t sprite_addr,
                                        uint8_t mode, uint8_t palette,
                                        uint8_t width_twos_comp,
                                        uint8_t x_pos) {
  zone[0] = (uint8_t)(sprite_addr & 0xffu);
  zone[1] = mode;
  zone[2] = (uint8_t)(sprite_addr >> 8);
  zone[3] = atari7800_maria_pal_width(palette, width_twos_comp);
  zone[4] = x_pos;
  zone[5] = 0x00u;
  zone[6] = 0x00u;
}

void atari7800_maria_clear_zone(uint8_t *zone, uint16_t zone_size) {
  uint16_t i;

  for (i = 0; i < zone_size; ++i) {
    zone[i] = 0x00u;
  }
}

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

uint8_t atari7800_maria_plot_sprite_asset_zone5(
    uint8_t *zone, uint16_t zone_size, uint8_t object_index,
    const atari7800_sprite_asset_t *asset, uint8_t x_pos) {
  uint16_t sprite_addr;

  if (asset == 0 || asset->data == 0) {
    return 0u;
  }

  if (asset->data_layout == ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A) {
    sprite_addr = atari7800_scene_compact_to_maria_rows(asset);
    if (sprite_addr == 0u) {
      return 0u;
    }
  } else {
    sprite_addr = atari7800_ptr16(asset->data);
  }

  return atari7800_maria_plot_sprite_zone5(zone, zone_size, object_index,
                                           sprite_addr, asset->mode,
                                           asset->palette,
                                           asset->width_twos_comp, x_pos);
}

void atari7800_scene_init_160a(atari7800_scene_t *scene, uint8_t bgcolor) {
  uint8_t zone_index;

  scene->zone = &atari7800_scene_zones[0][0];
  scene->zone_size = ATARI7800_SCENE_ZONE_BYTES;
  scene->next_object = 0u;
  scene->initialized = 1u;

  atari7800_maria_init_null_header(&atari7800_scene_null_zone, 0u);
  atari7800_maria_build_blank_ntsc(atari7800_scene_display_list,
                                   &atari7800_scene_null_zone);
  atari7800_scene_active_zones_prev = 0u;
  atari7800_scene_active_zones_curr = 0u;

  for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
       ++zone_index) {
    atari7800_scene_zone_next_object[zone_index] = 0u;
    atari7800_maria_clear_zone(atari7800_scene_zones[zone_index],
                               ATARI7800_SCENE_ZONE_BYTES);
  }

  atari7800_scene_begin_frame(scene);
  atari7800_init_160a(atari7800_ptr16(atari7800_scene_display_list), bgcolor);
}

void atari7800_scene_set_palette(atari7800_scene_t *scene,
                                 uint8_t palette_index,
                                 atari7800_palette3_t colors) {
  (void)scene;
  atari7800_set_palette3(palette_index, colors);
}

void atari7800_scene_begin_frame(atari7800_scene_t *scene) {
  (void)scene;

  atari7800_scene_compact_slot_next = 0u;
  atari7800_scene_active_zones_curr = 0u;

  /* Reset per-zone object cursors but keep currently active DLL bindings stable
   * through the frame to avoid visible blank windows.
   */
  for (uint8_t zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
       ++zone_index) {
    atari7800_scene_zone_next_object[zone_index] = 0u;
  }
}

void atari7800_scene_end_frame(atari7800_scene_t *scene) {
  uint8_t zone_index;
  (void)scene;

  /* Zones that were active last frame but not redrawn this frame are disabled
   * here, after draw submission is complete.
   */
  for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
       ++zone_index) {
    const uint32_t mask = ((uint32_t)1u << zone_index);
    if ((atari7800_scene_active_zones_prev & mask) != 0u &&
        (atari7800_scene_active_zones_curr & mask) == 0u) {
      atari7800_maria_clear_zone(atari7800_scene_zones[zone_index],
                                 ATARI7800_SCENE_ZONE_BYTES);
      atari7800_maria_init_dll_entry(
          &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], 7u,
          &atari7800_scene_null_zone, 0u);
    }
  }

  atari7800_scene_active_zones_prev = atari7800_scene_active_zones_curr;
}

void atari7800_scene_begin(atari7800_scene_t *scene, uint8_t *zone,
                           uint16_t zone_size) {
  scene->zone = zone;
  scene->zone_size = zone_size;
  scene->next_object = 0u;
  scene->initialized = 0u;
  atari7800_maria_clear_zone(zone, zone_size);
}

uint8_t atari7800_scene_plotsprite(atari7800_scene_t *scene,
                                   const void *sprite_data, uint8_t mode,
                                   uint8_t palette, uint8_t width_twos_comp,
                                   uint8_t x_pos) {
  const uint8_t object_index = scene->next_object;

  if (!atari7800_plotsprite(scene->zone, scene->zone_size, object_index,
                            sprite_data, mode, palette, width_twos_comp,
                            x_pos)) {
    return 0u;
  }

  scene->next_object = (uint8_t)(scene->next_object + 1u);
  return 1u;
}

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

  zone_index = (uint8_t)(y_pos / 7u);
  if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
    zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
  }

  zone = atari7800_scene_zones[zone_index];
  object_index = atari7800_scene_zone_next_object[zone_index];

  if (!atari7800_plotsprite_asset(zone, ATARI7800_SCENE_ZONE_BYTES,
                                  object_index, asset, x_pos)) {
    return 0u;
  }

  if ((atari7800_scene_active_zones_curr & ((uint32_t)1u << zone_index)) ==
      0u) {
    atari7800_maria_init_dll_entry(
        &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], 7u, zone,
        0u);
  }

  atari7800_scene_zone_next_object[zone_index] = (uint8_t)(object_index + 1u);
  atari7800_scene_active_zones_curr |= ((uint32_t)1u << zone_index);
  return 1u;
}

uint8_t atari7800_scene_plotchars(atari7800_scene_t *scene,
                                  const void *char_data, uint8_t mode,
                                  uint8_t palette, uint8_t width_twos_comp,
                                  uint8_t x_pos) {
  const uint8_t object_index = scene->next_object;

  if (!atari7800_plotchars(scene->zone, scene->zone_size, object_index,
                           char_data, mode, palette, width_twos_comp, x_pos)) {
    return 0u;
  }

  scene->next_object = (uint8_t)(scene->next_object + 1u);
  return 1u;
}
