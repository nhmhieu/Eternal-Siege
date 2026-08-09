#!/usr/bin/env python3
"""Deterministically extract Kingdom occluders and water presentation assets."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
import math

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/images/kingdom/kingdom_world_open.png"
OUT = ROOT / "assets/images/kingdom/layers"
SIZE = (1672, 941)

OCCLUDERS = {
    "kingdom_foreground_gate_left.png": [
        [(0, 644), (114, 644), (151, 704), (174, 784), (158, 941), (0, 941)]],
    "kingdom_foreground_gate_right.png": [
        [(278, 680), (326, 665), (404, 702), (522, 783), (522, 941), (294, 941), (288, 790)]],
    "kingdom_foreground_gate_arch.png": [
        [(102, 650), (151, 704), (178, 689), (229, 713), (278, 680),
         (326, 665), (318, 804), (282, 829), (166, 829), (132, 805)]],
    "kingdom_foreground_fountain.png": [
        [(645, 374), (678, 345), (744, 333), (797, 352), (817, 390),
         (798, 424), (753, 444), (690, 432), (653, 408)]],
    "kingdom_foreground_bridge_far.png": [[(913,693),(965,662),(1024,662),(1091,704),(1171,744),(1248,778),(1335,791),(1396,816),(1388,833),(1305,810),(1228,800),(1148,767),(1070,724),(992,692),(927,711)]],
    "kingdom_foreground_bridge_near.png": [[(927,711),(992,692),(1070,724),(1148,767),(1228,800),(1305,810),(1388,833),(1384,857),(1303,834),(1220,827),(1139,792),(1064,749),(987,717),(927,727)]],
    "kingdom_foreground_catacomb.png": [
        [(1214, 0), (1672, 0), (1672, 369), (1575, 354), (1490, 322),
         (1422, 329), (1350, 303), (1274, 316), (1214, 270)]],
    "kingdom_foreground_tree_village.png": [[(430,164),(485,122),(544,143),(572,220),(542,303),(468,327),(414,277)]],
    "kingdom_foreground_tree_bridge.png": [[(1100,522),(1142,465),(1198,491),(1223,576),(1195,661),(1132,674),(1090,615)]],
    "kingdom_foreground_tree_river.png": [[(1420,548),(1485,486),(1562,515),(1590,612),(1550,708),(1470,721),(1408,643)]],
}

# Hand-traced river ribbons. Bridge pixels remain in the base and its rail is
# separately extracted; this mask is presentation-only and never collision data.
WATER = [
    [(1672, 392), (1570, 401), (1492, 434), (1428, 488), (1394, 550),
     (1388, 626), (1435, 675), (1512, 711), (1605, 742), (1672, 776)],
    [(1672, 735), (1590, 725), (1510, 698), (1436, 659), (1375, 635),
     (1320, 650), (1270, 690), (1216, 731), (1160, 758), (1085, 770),
     (1015, 790), (966, 832), (922, 887), (889, 941), (1250, 941),
     (1305, 902), (1360, 878), (1435, 876), (1515, 894), (1588, 925),
     (1672, 941)]
]

# Independently readable copy of the authored ground-contact footprints used to
# produce the ignored alignment artifact. Keep labels visible in the preview so
# a reviewer can compare them directly with the artwork rather than a test oracle.
COLLISION_FOOTPRINTS = [
    ((34,793,152,917), "gate L"), ((298,783,410,917), "gate R"),
    ((0,884,154,917), "wall L"), ((404,891,642,917), "wall R"),
    ((37,176,159,224), "cottage"), ((210,166,353,213), "cottage"),
    ((386,175,540,220), "cottage"), ((18,444,188,509), "tavern"),
    ((197,408,441,483), "guildhall"), ((661,376,798,443), "fountain"),
    ((471,255,502,289), "tree"), ((1128,589,1162,628), "tree"),
    ((1480,626,1518,669), "tree"), ((846,279,871,310), "tree"),
    ((965,302,990,332), "tree"), ((1218,24,1382,249), "catacomb W"),
    ((1512,24,1646,334), "catacomb E"), ((1218,24,1646,97), "catacomb N"),
    ((1228,286,1365,332), "buttress"), ((1510,302,1646,351), "buttress"),
    ((1372,316,1507,344), "stair edge"), ((1090,371,1145,413), "cliff"),
    ((1568,387,1646,435), "cliff"),
]

def polygon_mask(polygons, feather=1):
    mask = Image.new("L", SIZE, 0)
    draw = ImageDraw.Draw(mask)
    for polygon in polygons:
        draw.polygon(polygon, fill=255)
    return mask.filter(ImageFilter.GaussianBlur(feather)) if feather else mask

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = Image.open(SOURCE).convert("RGBA")
    if source.size != SIZE:
        raise SystemExit(f"Unexpected Kingdom size: {source.size}")
    for name, polygons in OCCLUDERS.items():
        mask = polygon_mask(polygons, 1)
        layer = Image.new("RGBA", SIZE, (0, 0, 0, 0))
        layer.paste(source, (0, 0), mask)
        layer.save(OUT / name, optimize=True)
    water_mask = polygon_mask(WATER, 2)
    water_mask.save(OUT / "kingdom_water_mask.png", optimize=True)
    shadow = Image.new("RGBA", (128, 64), (0, 0, 0, 0))
    pixels = shadow.load()
    for y in range(64):
        for x in range(128):
            d = ((x - 63.5) / 60) ** 2 + ((y - 31.5) / 22) ** 2
            pixels[x, y] = (4, 7, 10, int(92 * max(0, 1 - d) ** 2))
    shadow.save(OUT / "kingdom_contact_shadow.png", optimize=True)
    # Two clipped frames provide a low-cost scrolling/cross-fade fallback.
    water = Image.open(ROOT / "assets/images/kingdom/water_flow.png").convert("RGBA")
    for frame, offset in enumerate(((0, 0), (83, 47))):
        tiled = Image.new("RGBA", SIZE)
        for y in range(-water.height, SIZE[1] + water.height, water.height):
            for x in range(-water.width, SIZE[0] + water.width, water.width):
                tiled.alpha_composite(water, (x + offset[0], y + offset[1]))
        alpha = water_mask.point(lambda a: int(a * 0.22))
        clipped = Image.new("RGBA", SIZE, (0, 0, 0, 0))
        clipped.paste(tiled, (0, 0), alpha)
        clipped.save(OUT / f"kingdom_water_flow_{frame}.png", optimize=True)
    qa = ROOT / "tmp/kingdom_visual_qa"
    qa.mkdir(parents=True, exist_ok=True)
    preview = source.copy()
    overlay = Image.new("RGBA", SIZE, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    for polygon in WATER:
        draw.polygon(polygon, fill=(25, 90, 235, 55), outline=(70, 170, 255, 220), width=3)
    for box, label in COLLISION_FOOTPRINTS:
        draw.rectangle(box, fill=(235, 45, 45, 65), outline=(255, 95, 70, 235), width=3)
        draw.text((box[0]+3, box[1]+3), label, fill=(255,255,220,255))
    a, b = (1030,655), (1390,880)
    dx, dy = b[0]-a[0], b[1]-a[1]
    length = math.hypot(dx,dy); nx, ny = -dy/length, dx/length
    draw.polygon([(a[0]-nx*50,a[1]-ny*50),(a[0]+nx*50,a[1]+ny*50),
                  (b[0]+nx*50,b[1]+ny*50),(b[0]-nx*50,b[1]-ny*50)],
                 fill=(30,225,190,60), outline=(60,255,220,230))
    for side in (-61,61):
        draw.line((a[0]+nx*side,a[1]+ny*side,b[0]+nx*side,b[1]+ny*side),
                  fill=(255,70,60,255), width=7)
    draw.rectangle((158,830,290,917), fill=(60,230,110,45), outline=(80,255,145,230), width=3)
    draw.ellipse((217,877,231,891), fill=(255,220,35,255))
    preview.alpha_composite(overlay)
    preview.save(qa / "collision_alignment_1672x941.png", optimize=True)

if __name__ == "__main__":
    main()
