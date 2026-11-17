#!/usr/bin/env python3
"""Draw the seven 48 px Pebble alert icons without third-party packages."""

from pathlib import Path
import struct
import zlib


SIZE = 24  # One design pixel becomes a crisp 2 x 2 block.
OUT = Path(__file__).resolve().parents[1] / "resources" / "images"
T = (0, 0, 0, 0)
WHITE = (255, 255, 255)
CREAM = (255, 255, 170)
GOLD = (255, 170, 0)
ORANGE = (255, 85, 0)
RED = (255, 85, 85)
PINK = (255, 170, 255)
PURPLE = (170, 85, 255)
CYAN = (85, 255, 255)
BLUE = (0, 170, 255)
STEEL = (170, 255, 255)


def canvas():
    return [[T for _ in range(SIZE)] for _ in range(SIZE)]


def put(c, x, y, color):
    if 0 <= x < SIZE and 0 <= y < SIZE:
        c[y][x] = color


def rect(c, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            put(c, x, y, color)


def ellipse(c, x0, y0, x1, y1, color):
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    rx, ry = (x1 - x0 + 1) / 2, (y1 - y0 + 1) / 2
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1:
                put(c, x, y, color)


def polygon(c, points, color):
    # Even-odd scan conversion at pixel centers.
    for y in range(SIZE):
        for x in range(SIZE):
            inside = False
            for a, b in zip(points, points[1:] + points[:1]):
                if (a[1] > y) != (b[1] > y):
                    crossing = a[0] + (y - a[1]) * (b[0] - a[0]) / (b[1] - a[1])
                    if x < crossing:
                        inside = not inside
            if inside:
                put(c, x, y, color)


def outline(c):
    original = [row[:] for row in c]
    for y in range(SIZE):
        for x in range(SIZE):
            if original[y][x] != T:
                continue
            if any(
                0 <= x + dx < SIZE and 0 <= y + dy < SIZE
                and original[y + dy][x + dx] != T
                for dy in (-1, 0, 1) for dx in (-1, 0, 1)
            ):
                c[y][x] = WHITE


def trophy():
    c = canvas()
    rect(c, 5, 3, 18, 5, GOLD)
    polygon(c, [(6, 6), (18, 6), (17, 12), (15, 15), (9, 15), (7, 12)], GOLD)
    rect(c, 10, 15, 13, 19, GOLD)
    rect(c, 7, 20, 16, 21, GOLD)
    rect(c, 3, 6, 5, 10, ORANGE)
    rect(c, 4, 11, 6, 12, ORANGE)
    rect(c, 18, 6, 20, 10, ORANGE)
    rect(c, 17, 11, 19, 12, ORANGE)
    outline(c)
    rect(c, 9, 6, 10, 11, CREAM)
    rect(c, 11, 16, 12, 18, CREAM)
    return c


def muscle():
    c = canvas()
    skin = (255, 170, 85)
    shadow = (170, 85, 0)
    # Forearm runs left from the elbow; the upper arm rises on the right.
    polygon(c, [(2, 15), (6, 14), (11, 16), (16, 16), (19, 13),
                (21, 15), (21, 18), (18, 21), (5, 21), (2, 18)], shadow)
    polygon(c, [(3, 16), (6, 15), (11, 17), (16, 17), (19, 15),
                (20, 16), (20, 18), (18, 20), (5, 20), (3, 18)], skin)
    polygon(c, [(15, 6), (20, 6), (21, 9), (20, 17), (18, 19),
                (15, 17), (14, 11)], shadow)
    polygon(c, [(16, 7), (19, 7), (20, 9), (19, 16), (17, 18),
                (16, 16), (15, 11)], skin)
    ellipse(c, 8, 8, 18, 17, shadow)
    ellipse(c, 9, 9, 17, 16, skin)
    ellipse(c, 10, 10, 14, 12, CREAM)
    # Knuckles and thumb make the raised end read as a clenched fist.
    polygon(c, [(15, 2), (19, 2), (21, 4), (21, 7), (19, 9),
                (15, 9), (13, 7), (13, 4)], shadow)
    polygon(c, [(15, 3), (19, 3), (20, 4), (20, 7), (18, 8),
                (15, 8), (14, 6), (14, 4)], skin)
    rect(c, 16, 3, 16, 4, shadow)
    rect(c, 18, 3, 18, 4, shadow)
    rect(c, 14, 6, 16, 6, shadow)
    # This crease is transparent in the 1-bit rendering.
    polygon(c, [(7, 16), (11, 16), (15, 17), (14, 18),
                (10, 17), (7, 17)], ORANGE)
    return c


def hourglass():
    c = canvas()
    rect(c, 5, 2, 18, 4, STEEL)
    rect(c, 5, 19, 18, 21, STEEL)
    polygon(c, [(6, 5), (18, 5), (16, 9), (13, 12),
                (16, 15), (18, 18), (6, 18), (8, 15),
                (11, 12), (8, 9)], CYAN)
    outline(c)
    polygon(c, [(8, 6), (16, 6), (14, 9), (12, 11), (10, 9)], T)
    polygon(c, [(12, 13), (14, 16), (16, 17), (8, 17), (10, 16)], GOLD)
    rect(c, 11, 11, 12, 12, GOLD)
    return c


def flame():
    c = canvas()
    polygon(c, [(12, 2), (15, 7), (15, 10), (18, 8), (20, 13),
                (20, 17), (17, 21), (13, 22), (8, 21), (4, 17),
                (5, 12), (8, 9), (9, 13), (10, 7)], RED)
    outline(c)
    polygon(c, [(12, 9), (16, 14), (17, 17), (15, 20),
                (10, 20), (7, 17), (8, 14), (10, 16)], ORANGE)
    polygon(c, [(12, 14), (14, 17), (14, 20), (10, 20), (10, 18)], GOLD)
    return c


def sparkles():
    c = canvas()
    polygon(c, [(12, 2), (14, 8), (20, 10), (14, 12),
                (12, 18), (10, 12), (4, 10), (10, 8)], GOLD)
    polygon(c, [(19, 15), (20, 18), (23, 19), (20, 20),
                (19, 23), (18, 20), (15, 19), (18, 18)], CYAN)
    polygon(c, [(4, 15), (5, 17), (7, 18), (5, 19),
                (4, 21), (3, 19), (1, 18), (3, 17)], PINK)
    outline(c)
    rect(c, 11, 8, 12, 10, CREAM)
    return c


def plate():
    c = canvas()
    ellipse(c, 6, 4, 19, 19, STEEL)
    rect(c, 2, 7, 3, 20, CYAN)  # Fork shaft.
    rect(c, 1, 4, 1, 9, CYAN)
    rect(c, 3, 4, 3, 9, CYAN)
    rect(c, 5, 4, 5, 9, CYAN)
    rect(c, 1, 9, 5, 10, CYAN)
    rect(c, 21, 10, 22, 20, CYAN)  # Knife handle and blade.
    polygon(c, [(20, 4), (22, 4), (22, 11), (20, 12)], CYAN)
    outline(c)
    ellipse(c, 8, 6, 17, 17, BLUE)
    ellipse(c, 10, 8, 15, 15, T)
    return c


def lock():
    c = canvas()
    ellipse(c, 6, 2, 17, 15, STEEL)
    rect(c, 5, 10, 18, 20, BLUE)
    outline(c)
    ellipse(c, 9, 5, 14, 13, T)  # Shackle opening above the body.
    rect(c, 5, 10, 18, 20, BLUE)
    rect(c, 7, 12, 16, 13, CYAN)
    rect(c, 11, 15, 12, 18, CREAM)
    rect(c, 10, 14, 13, 16, CREAM)
    return c


def png_chunk(kind, payload):
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(
        ">I", zlib.crc32(kind + payload) & 0xFFFFFFFF
    )


def write_png(path, pixels):
    raw = bytearray()
    for row in pixels:
        raw.append(0)  # PNG filter: None.
        for r, g, b, a in row:
            raw.extend((r, g, b, a))
    data = b"\x89PNG\r\n\x1a\n"
    data += png_chunk(b"IHDR", struct.pack(">IIBBBBB", 48, 48, 8, 6, 0, 0, 0))
    data += png_chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    data += png_chunk(b"IEND", b"")
    path.write_bytes(data)


def enlarged(c, monochrome=False, cutouts=()):
    result = []
    for row in c:
        expanded = []
        for pixel in row:
            if pixel == T or (monochrome and pixel in cutouts):
                rgba = T
            elif monochrome:
                rgba = (255, 255, 255, 255)
            else:
                rgba = tuple(min(255, (channel + 42) // 85 * 85)
                             for channel in pixel) + (255,)
            expanded.extend((rgba, rgba))
        result.extend((expanded[:], expanded[:]))
    return result


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for name, draw in (
        ("trophy", trophy), ("muscle", muscle), ("hourglass", hourglass),
        ("flame", flame), ("sparkles", sparkles), ("plate", plate),
        ("lock", lock),
    ):
        icon = draw()
        write_png(OUT / f"alert_{name}.png", enlarged(icon))
        bw_cutouts = (ORANGE, GOLD) if name == "flame" else (ORANGE,) if name == "muscle" else ()
        write_png(OUT / f"alert_{name}~bw.png",
                  enlarged(icon, monochrome=True, cutouts=bw_cutouts))


if __name__ == "__main__":
    main()
