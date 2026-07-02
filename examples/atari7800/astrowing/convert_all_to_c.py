import os
import sys
from PIL import Image

GFX_DIR = "/Users/jasonrowe/Software/Atari7800/Astrowing/src/graphics"
OUT_DIR = "/Users/jasonrowe/Software/Atari7800/llvm-mos-sdk-7800/examples/atari7800/astrowing/assets"
SRC_DIR = "/Users/jasonrowe/Software/Atari7800/llvm-mos-sdk-7800/examples/atari7800/astrowing"

os.makedirs(OUT_DIR, exist_ok=True)

def load_160a_image_rows(filename, width_bytes, height_lines=16):
    filepath = os.path.join(GFX_DIR, filename)
    if not os.path.exists(filepath):
        print(f"Error: {filepath} not found")
        return [[0] * width_bytes for _ in range(height_lines)]
    img = Image.open(filepath).convert("P")
    width, height = img.size
    pixels = img.load()
    rows = []
    for y in range(height_lines):
        row = []
        for xb in range(width_bytes):
            if y < height and xb * 4 < width:
                p0 = pixels[xb * 4, y] & 3
                p1 = pixels[xb * 4 + 1, y] & 3 if xb * 4 + 1 < width else 0
                p2 = pixels[xb * 4 + 2, y] & 3 if xb * 4 + 2 < width else 0
                p3 = pixels[xb * 4 + 3, y] & 3 if xb * 4 + 3 < width else 0
                byte_val = (p0 << 6) | (p1 << 4) | (p2 << 2) | p3
                row.append(byte_val)
            else:
                row.append(0)
        rows.append(row)
    return rows

def load_multi_frame_rows(filenames, width_bytes, height_lines=16):
    combined_rows = [[] for _ in range(height_lines)]
    for filename in filenames:
        frame_rows = load_160a_image_rows(filename, width_bytes, height_lines)
        for y in range(height_lines):
            combined_rows[y].extend(frame_rows[y])
    return combined_rows

def load_title_screen_zones(filename):
    filepath = os.path.join(GFX_DIR, filename)
    if not os.path.exists(filepath):
        print(f"Error: {filepath} not found")
        return [[[0] * 40 for _ in range(16)] for _ in range(6)]
    img = Image.open(filepath).convert("P")
    width, height = img.size
    pixels = img.load()
    zones = []
    for z in range(6):
        zone_rows = []
        for y in range(16):
            img_y = z * 16 + y
            row = []
            for xb in range(40):
                p0 = pixels[xb * 4, img_y] & 3
                p1 = pixels[xb * 4 + 1, img_y] & 3
                p2 = pixels[xb * 4 + 2, img_y] & 3
                p3 = pixels[xb * 4 + 3, img_y] & 3
                byte_val = (p0 << 6) | (p1 << 4) | (p2 << 2) | p3
                row.append(byte_val)
            zone_rows.append(row)
        zones.append(zone_rows)
    return zones

class BlockPacker:
    def __init__(self, name):
        self.name = name
        self.block = [0] * (16 * 256)
        self.current_offset = 0
        
    def pack(self, rows):
        width = len(rows[0])
        self.current_offset = (self.current_offset + 1) & ~1  # 2-byte alignment
        if self.current_offset + width > 256:
            raise ValueError(f"Block {self.name} overflowed! Offset {self.current_offset} + width {width} > 256")
        offset = self.current_offset
        for y in range(16):
            for xb in range(width):
                self.block[(15 - y) * 256 + offset + xb] = rows[y][xb]
        self.current_offset += width
        return offset

def write_block_source(filename, block_name, block_data):
    filepath = os.path.join(SRC_DIR, filename)
    mode = "w" if block_name == "title_block" else "a"
    with open(filepath, mode) as f:
        if mode == "w":
            f.write("#include <stdint.h>\n\n")
        f.write(f"const uint8_t {block_name}[] __attribute__((aligned(256))) = {{\n")
        for y in range(16):
            f.write(f"    /* Row {y} */\n")
            row = block_data[y * 256 : (y + 1) * 256]
            for r in range(16):
                chunk = row[r * 16 : (r + 1) * 16]
                row_str = ", ".join(f"0x{b:02x}" for b in chunk)
                comma = "," if (y < 15 or r < 15) else ""
                f.write(f"    {row_str}{comma}\n")
        f.write("};\n\n")

def generate_header(name, block_name, offset, width_bytes, height_lines=16, palette=0, num_frames=None):
    header_name = name.lower() + ".h"
    out_path = os.path.join(OUT_DIR, header_name)
    with open(out_path, "w") as f:
        f.write(f"#ifndef ATARI7800_ASSET_{name.upper()}_H\n")
        f.write(f"#define ATARI7800_ASSET_{name.upper()}_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define {name.upper()}_WIDTH_BYTES {width_bytes}u\n")
        f.write(f"#define {name.upper()}_HEIGHT_LINES {height_lines}u\n")
        f.write(f"#define {name.upper()}_DEFAULT_PALETTE {palette}u\n")
        if num_frames is not None:
            f.write(f"#define {name.upper()}_NUM_FRAMES {num_frames}u\n")
        f.write("\n")
        f.write(f"extern const uint8_t {block_name}[];\n")
        f.write(f"#define {name.lower()}_data ({block_name} + {offset})\n\n")
        f.write("#endif\n")
    print(f"Generated {header_name}")

if __name__ == "__main__":
    title_packer = BlockPacker("title_block")
    game0_packer = BlockPacker("game_block_0")
    game1_packer = BlockPacker("game_block_1")
    
    # 1. Title screen
    title_zones = load_title_screen_zones("title_screen_conv.png")
    title_offsets = []
    for z in range(6):
        title_offsets.append(title_packer.pack(title_zones[z]))
        
    # Write title_screen.h containing the zone macro offsets
    with open(os.path.join(OUT_DIR, "title_screen.h"), "w") as f:
        f.write("#ifndef ATARI7800_ASSET_TITLE_SCREEN_H\n")
        f.write("#define ATARI7800_ASSET_TITLE_SCREEN_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write("extern const uint8_t title_block[];\n\n")
        for z in range(6):
            f.write(f"#define title_screen_zone{z} (title_block + {title_offsets[z]})\n")
        f.write("\n#endif\n")
    print("Generated title_screen.h")
    
    # 2. Pack game_block_0
    # Font
    font_rows = load_160a_image_rows("unified_font.png", 112, 16)
    font_offset = game0_packer.pack(font_rows)
    generate_header("UNIFIED_FONT", "game_block_0", font_offset, 112, 16, 7)
    
    # Spaceship (16 frames)
    spaceship_files = [f"sprite_spaceship{i}.png" for i in range(1, 17)]
    spaceship_rows = load_multi_frame_rows(spaceship_files, 4, 16)
    spaceship_offset = game0_packer.pack(spaceship_rows)
    generate_header("SPACESHIP", "game_block_0", spaceship_offset, 4, 16, 5, 16)
    
    # Asteroid M
    asteroid_m_rows = load_160a_image_rows("asteroid_M_conv.png", 4, 16)
    asteroid_m_offset = game0_packer.pack(asteroid_m_rows)
    generate_header("ASTEROID_M", "game_block_0", asteroid_m_offset, 4, 16, 2)
    
    # Asteroid S
    asteroid_s_rows = load_160a_image_rows("asteroid_S_conv.png", 2, 16)
    asteroid_s_offset = game0_packer.pack(asteroid_s_rows)
    generate_header("ASTEROID_S", "game_block_0", asteroid_s_offset, 2, 16, 2)
    
    # 3. Pack game_block_1
    # Boss
    boss_files = [f"BossV2_{i:02d}.png" for i in range(1, 5)]
    boss_rows = load_multi_frame_rows(boss_files, 4, 16)
    boss_offset = game1_packer.pack(boss_rows)
    generate_header("BOSS", "game_block_1", boss_offset, 4, 16, 3, 4)
    
    # Arrows
    arrow_files = [f"arrows_{i:02d}.png" for i in range(1, 5)]
    arrow_rows = load_multi_frame_rows(arrow_files, 2, 16)
    arrows_offset = game1_packer.pack(arrow_rows)
    generate_header("ARROWS", "game_block_1", arrows_offset, 2, 16, 3, 4)
    
    # Explosion
    explosion_files = [f"fighter_explode_{i:02d}_conv.png" for i in range(8)]
    explosion_rows = load_multi_frame_rows(explosion_files, 1, 16)
    explosion_offset = game1_packer.pack(explosion_rows)
    generate_header("EXPLOSION", "game_block_1", explosion_offset, 1, 16, 3, 8)
    
    # Bullet
    bullet_rows = load_160a_image_rows("bullet_conv.png", 1, 16)
    bullet_offset = game1_packer.pack(bullet_rows)
    generate_header("BULLET", "game_block_1", bullet_offset, 1, 16, 1)
    
    # Fighter
    fighter_rows = load_160a_image_rows("fighter_conv.png", 1, 16)
    fighter_offset = game1_packer.pack(fighter_rows)
    generate_header("FIGHTER", "game_block_1", fighter_offset, 1, 16, 3)
    
    # Blue Fighter
    blue_fighter_rows = load_160a_image_rows("Blue_fighter.png", 2, 16)
    blue_fighter_offset = game1_packer.pack(blue_fighter_rows)
    generate_header("BLUE_FIGHTER", "game_block_1", blue_fighter_offset, 2, 16, 3)
    
    # Energy
    energy_rows = load_160a_image_rows("Energy.png", 1, 16)
    energy_offset = game1_packer.pack(energy_rows)
    generate_header("ENERGY", "game_block_1", energy_offset, 1, 16, 5)
    
    # Heart
    heart_rows = load_160a_image_rows("heart_conv.png", 2, 16)
    heart_offset = game1_packer.pack(heart_rows)
    generate_header("HEART", "game_block_1", heart_offset, 2, 16, 5)
    
    # Lives Icon
    lives_icon_rows = load_160a_image_rows("lives_icon_conv.png", 2, 16)
    lives_icon_offset = game1_packer.pack(lives_icon_rows)
    generate_header("LIVES_ICON", "game_block_1", lives_icon_offset, 2, 16, 5)
    
    # Write assets_blocks.c
    write_block_source("assets_blocks.c", "title_block", title_packer.block)
    write_block_source("assets_blocks.c", "game_block_0", game0_packer.block)
    write_block_source("assets_blocks.c", "game_block_1", game1_packer.block)
    print("Generated assets_blocks.c")
