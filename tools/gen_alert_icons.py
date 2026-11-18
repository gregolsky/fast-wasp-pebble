#!/usr/bin/env python3
"""Draw seven Pebble alert icons and a 3x contact sheet, using only stdlib."""
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'resources' / 'images'
W, H = 24, 16
INK = {
    '.': (0, 0, 0, 0),
    'Y': (255, 170, 0, 255),   # orange
    'L': (255, 255, 0, 255),   # yellow
    'O': (255, 85, 0, 255),    # red orange
    'W': (255, 255, 255, 255), # white highlight
    'D': (85, 85, 0, 255),     # dark cutout on monochrome
    'B': (170, 85, 0, 255),    # dark orange outline
}
PLATFORMS = (('basalt', '', 3), ('bw', '~bw', 3),
             ('emery', '~emery', 5), ('gabbro', '~gabbro', 5))


def canvas():
    return [['.'] * W for _ in range(H)]


def rect(c, x0, y0, x1, y1, ink):
    for y in range(max(0, y0), min(H, y1 + 1)):
        for x in range(max(0, x0), min(W, x1 + 1)):
            c[y][x] = ink


def poly(c, points, ink):
    for y in range(H):
        for x in range(W):
            inside = False
            for a, b in zip(points, points[1:] + points[:1]):
                if (a[1] > y + .5) != (b[1] > y + .5):
                    crossing = a[0] + (y + .5 - a[1]) * (b[0] - a[0]) / (b[1] - a[1])
                    if x + .5 < crossing:
                        inside = not inside
            if inside:
                c[y][x] = ink


def ellipse(c, cx, cy, rx, ry, ink):
    for y in range(H):
        for x in range(W):
            if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1:
                c[y][x] = ink


def trophy():
    c = canvas()
    rect(c, 7, 1, 16, 2, 'L')
    poly(c, [(7, 3), (17, 3), (16, 8), (14, 10), (10, 10), (8, 8)], 'Y')
    rect(c, 4, 3, 6, 4, 'L')
    rect(c, 3, 5, 4, 7, 'L')
    rect(c, 4, 8, 6, 9, 'L')
    rect(c, 17, 3, 19, 4, 'L')
    rect(c, 19, 5, 20, 7, 'L')
    rect(c, 17, 8, 19, 9, 'L')
    rect(c, 7, 10, 9, 11, '.')
    rect(c, 15, 10, 17, 11, '.')
    rect(c, 11, 10, 12, 13, 'O')
    rect(c, 8, 14, 15, 15, 'L')
    rect(c, 10, 4, 13, 4, 'W')
    return c


def muscle():
    c = canvas()
    # Half a solid disk, half an outlined ring. The vertical seam is deliberate.
    ellipse(c, 11.5, 7.5, 7.5, 7.5, 'B')
    ellipse(c, 11.5, 7.5, 6.5, 6.5, 'L')
    ellipse(c, 11.5, 7.5, 4.5, 4.5, '.')
    for y in range(H):
        for x in range(4, 12):
            if ((x - 11.5) / 6.5) ** 2 + ((y - 7.5) / 6.5) ** 2 <= 1:
                c[y][x] = 'L'
    rect(c, 11, 2, 11, 13, 'B')
    return c


def hourglass():
    c = canvas()
    rect(c, 5, 0, 18, 2, 'L')
    rect(c, 5, 13, 18, 15, 'L')
    rect(c, 6, 3, 6, 12, 'B')
    rect(c, 17, 3, 17, 12, 'B')
    poly(c, [(7, 3), (17, 3), (15, 6), (13, 7),
             (13, 8), (15, 10), (17, 13), (7, 13),
             (9, 10), (11, 8), (11, 7), (9, 6)], 'Y')
    poly(c, [(9, 4), (15, 4), (13, 6), (11, 6)], '.')
    rect(c, 11, 7, 12, 8, 'W')
    poly(c, [(11, 10), (12, 9), (13, 10), (15, 12), (9, 12)], 'O')
    return c


def flame():
    c = canvas()
    poly(c, [(13, 0), (16, 5), (15, 8), (18, 5),
             (21, 9), (20, 13), (17, 16), (7, 16),
             (4, 13), (3, 9), (7, 4), (8, 9), (10, 6)], 'B')
    poly(c, [(13, 1), (15, 6), (14, 10), (18, 7),
             (20, 10), (19, 13), (16, 15), (8, 15),
             (5, 13), (5, 9), (7, 6), (8, 11), (11, 7)], 'O')
    poly(c, [(12, 8), (14, 11), (16, 12), (15, 15),
             (9, 15), (8, 13), (10, 11)], 'D')
    poly(c, [(12, 10), (13, 12), (14, 13), (14, 15),
             (10, 15), (10, 13)], 'L')
    return c


def sparkles():
    c = canvas()
    # Two broad arcs circle an open center; the tips show the renewal direction.
    ellipse(c, 11.5, 7.5, 9, 7.5, 'B')
    ellipse(c, 11.5, 7.5, 8, 6.5, 'L')
    ellipse(c, 11.5, 7.5, 5.5, 4.5, '.')
    rect(c, 0, 4, 8, 9, '.')
    rect(c, 15, 8, 23, 11, '.')
    poly(c, [(16, 3), (23, 6), (17, 10)], 'B')
    poly(c, [(17, 4), (21, 6), (18, 8)], 'Y')
    poly(c, [(7, 5), (1, 9), (8, 12)], 'B')
    poly(c, [(6, 7), (3, 9), (7, 10)], 'Y')
    return c


def plate():
    c = canvas()
    rect(c, 1, 1, 1, 5, 'L')
    rect(c, 3, 1, 3, 5, 'L')
    rect(c, 5, 1, 5, 5, 'L')
    rect(c, 1, 5, 5, 7, 'L')
    rect(c, 3, 8, 3, 15, 'L')
    ellipse(c, 13, 8, 6, 7, 'L')
    ellipse(c, 13, 8, 4, 5, 'D')
    # A small fish silhouette is food, with a clear tail and eye.
    poly(c, [(10, 7), (12, 5), (15, 6), (17, 8),
             (15, 10), (12, 11), (10, 9)], 'O')
    poly(c, [(10, 7), (8, 6), (8, 10), (10, 9)], 'O')
    rect(c, 14, 7, 14, 7, 'W')
    rect(c, 22, 1, 22, 15, 'L')
    rect(c, 21, 1, 21, 6, 'L')
    return c


def lock():
    c = canvas()
    poly(c, [(5, 8), (5, 5), (7, 1), (10, 0), (14, 0),
             (17, 1), (19, 5), (19, 8), (16, 8),
             (16, 5), (14, 3), (10, 3), (8, 5), (8, 8)], 'L')
    rect(c, 4, 7, 19, 15, 'B')
    rect(c, 5, 8, 18, 14, 'Y')
    rect(c, 11, 9, 12, 11, 'D')
    rect(c, 10, 12, 13, 13, 'D')
    return c


ICONS = (('trophy', trophy), ('muscle', muscle),
         ('hourglass', hourglass), ('flame', flame),
         ('sparkles', sparkles), ('plate', plate), ('lock', lock))


def raster(design, scale, bw=False):
    rows = []
    for row in design:
        line = bytearray()
        for mark in row:
            color = ((0, 0, 0, 0) if mark in ('.', 'D') else
                     (255, 255, 255, 255)) if bw else INK[mark]
            line.extend(bytes(color) * scale)
        rows.extend(bytes(line) for _ in range(scale))
    return rows


def chunk(kind, payload):
    return (struct.pack('>I', len(payload)) + kind + payload +
            struct.pack('>I', zlib.crc32(kind + payload) & 0xffffffff))


def write_png(path, width, rows):
    raw = b''.join(b'\0' + row for row in rows)
    data = b'\x89PNG\r\n\x1a\n'
    data += chunk(b'IHDR', struct.pack('>IIBBBBB', width, len(rows), 8, 6, 0, 0, 0))
    data += chunk(b'IDAT', zlib.compress(raw, 9))
    data += chunk(b'IEND', b'')
    path.write_bytes(data)


# Three-by-five glyphs for labels on the review sheet.
FONT = {
    'A': ('010','101','111','101','101'), 'B': ('110','101','110','101','110'),
    'C': ('011','100','100','100','011'), 'D': ('110','101','101','101','110'),
    'E': ('111','100','110','100','111'), 'F': ('111','100','110','100','100'),
    'G': ('011','100','101','101','011'), 'H': ('101','101','111','101','101'),
    'I': ('111','010','010','010','111'), 'K': ('101','101','110','101','101'),
    'L': ('100','100','100','100','111'), 'M': ('101','111','111','101','101'),
    'N': ('101','111','111','111','101'), 'O': ('010','101','101','101','010'),
    'P': ('110','101','110','100','100'), 'R': ('110','101','110','101','101'),
    'S': ('011','100','010','001','110'), 'T': ('111','010','010','010','010'),
    'U': ('101','101','101','101','111'), 'W': ('101','101','111','111','101'),
    'Y': ('101','101','010','010','010'), ' ': ('000',) * 5,
}


def preview(icons):
    cell_w, cell_h, margin, label_h = 460, 274, 24, 36
    width = margin * 2 + cell_w * 4
    height = margin * 2 + label_h + cell_h * len(ICONS)
    black = bytes((0, 0, 0, 255))
    pixels = [bytearray(black * width) for _ in range(height)]

    def dot(x, y, color):
        pixels[y][x * 4:x * 4 + 4] = color

    def label(message, x, y, color=bytes((170, 170, 170, 255))):
        for letter in message.upper():
            for dy, line in enumerate(FONT[letter]):
                for dx, bit in enumerate(line):
                    if bit == '1':
                        for sy in range(3):
                            for sx in range(3):
                                dot(x + dx * 3 + sx, y + dy * 3 + sy, color)
            x += 12

    for col, (platform, _, _) in enumerate(PLATFORMS):
        label(platform, margin + col * cell_w + 12, margin)
    for row, (name, _) in enumerate(ICONS):
        top = margin + label_h + row * cell_h
        label(name, margin + 12, top + 8, bytes((255, 170, 0, 255)))
        for col, (platform, _, scale) in enumerate(PLATFORMS):
            source = icons[(name, platform)]
            sw, sh = W * scale, H * scale
            left = margin + col * cell_w + (cell_w - sw * 3) // 2
            image_top = top + 28 + (cell_h - 28 - sh * 3) // 2
            for y, scan in enumerate(source):
                for x in range(sw):
                    rgba = scan[x * 4:x * 4 + 4]
                    if rgba[3]:
                        for sy in range(3):
                            start = (left + x * 3) * 4
                            pixels[image_top + y * 3 + sy][start:start + 12] = rgba * 3
    write_png(ROOT / 'tools' / 'alert_icons_preview.png', width, pixels)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    icons = {}
    for name, draw in ICONS:
        design = draw()
        for platform, suffix, scale in PLATFORMS:
            rows = raster(design, scale, platform == 'bw')
            write_png(OUT / f'alert_{name}{suffix}.png', W * scale, rows)
            icons[(name, platform)] = rows
    preview(icons)


if __name__ == '__main__':
    main()
