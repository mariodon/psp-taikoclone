DATA = {
    "bg_upper": {
        "tex_name": "bg",
        "sx": 0,
        "sy": 28,
        "w": 128,
        "h": 80,
    },
}

import struct
base_fmt = "<I24sIIIIIIIIffIfI"
base_size = struct.calcsize(base_fmt)
for frame_name, cfg in DATA.iteritems():
    tex_name = cfg.get("tex_name", "")
    x = cfg.get("x", 0)
    y = cfg.get("y", 0)
    sx = cfg.get("sx", 0)
    sy = cfg.get("sy", 0)
    w = cfg.get("w", 0)
    h = cfg.get("h", 0)
    center_x = cfg.get("center_x", 0)
    center_y = cfg.get("center_y", 0)
    scale_x = cfg.get("scale_x", 1.0)
    scale_y = cfg.get("scale_y", 1.0)
    angle = cfg.get("angle", 0)
    alpha = cfg.get("alpha", 1.0)
    palette = cfg.get("palette", [])
    size_palette = len(palette)
    size = size_palette * struct.calcsize("<I") + base_size

    data = struct.pack(base_fmt, size, tex_name, x, y, sx, sy, w, h, center_x,
            center_y, scale_x, scale_y, angle, alpha, size_palette)
    for v in palette:
        data += struct.pack("<I", v)

    f = open("../frame/" + frame_name + ".f", "wb")
    f.write(data)
    f.close()

    
