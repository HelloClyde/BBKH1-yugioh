"""Generate original geometric artwork; no third-party pixels are inputs."""
import argparse
from pathlib import Path
from PIL import Image, ImageDraw
from common import ROOT


def attack_header(raw: bytes, target: Path):
    import struct
    if len(raw) != 2 * 35 * 71 * 2:
        raise ValueError('Attack sprite data must be two 35x71 RGB565 images')
    words = struct.unpack('<4970H', raw)
    text = '#pragma once\n#include <stdint.h>\n#define SCENE_ATTACK_W 35\n#define SCENE_ATTACK_H 71\n'
    text += 'static const uint16_t scene_attack_pixels[2][35*71]={\n'
    for i in range(2):
        text += '{' + ','.join(f'0x{x:04x}' for x in words[i*2485:(i+1)*2485]) + '},\n'
    target.write_text(text + '};\n', encoding='utf-8')


def generate(folder: Path):
    import struct
    folder.mkdir(parents=True, exist_ok=True)
    # Abstract crossed cards: no character, original card back or trademark.
    icon = Image.new('RGBA', (96, 96), (0, 0, 0, 0))
    d = ImageDraw.Draw(icon)
    d.rounded_rectangle((6, 6, 89, 89), radius=19, fill='#17354e', outline='#abc7d3', width=3)
    d.polygon([(23, 24), (60, 17), (72, 69), (35, 76)], fill='#3c6576', outline='#eef5f5', width=2)
    d.rounded_rectangle((39, 29, 77, 80), radius=3, fill='#c7a85c', outline='#f8e8ba', width=2)
    d.polygon([(58, 39), (70, 53), (58, 67), (46, 53)], fill='#26465b')
    icon.save(folder / 'icon.png', optimize=False)
    raw = bytearray()
    for color in ('#e9c85c', '#84c6ee'):
        pic = Image.new('RGB', (35, 71), (0, 252, 0))
        p = ImageDraw.Draw(pic)
        p.polygon([(17, 0), (23, 12), (21, 47), (13, 47), (11, 12)], fill=color, outline='#ffffff')
        p.rectangle((4, 47, 30, 51), fill='#98763b')
        p.rectangle((14, 52, 20, 65), fill='#54412f')
        p.ellipse((12, 64, 22, 70), fill='#c6a45c')
        for r, g, b in pic.getdata():
            raw.extend(struct.pack('<H', ((r & 248) << 8) | ((g & 252) << 3) | (b >> 3)))
    attack_header(bytes(raw), folder / 'duel_attack_assets.h')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out', type=Path, default=ROOT / 'build/open-assets')
    generate(p.parse_args().out)
