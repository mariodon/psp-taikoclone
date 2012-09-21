DATA = {
    "bg_upper": {
        "tex_name": "bg",
        "sx": 0,
        "sy": 30,
        "w": 128,
        "h": 75,
    },

    "bg_note_normal": {
        "tex_name": "note_bg",
        "sx": 0,
        "sy": 0,
        "w": 1,
        "h": 82,
        "scale_x": 480.0,
    },

    "bg_note_ggt": {
        "tex_name": "note_bg_ggt",
        "sx": 0,
        "sy": 0,
        "w": 16,
        "h": 4,
        "scale_x":24.875,
        "scale_y":13.5,
        "x": 82,
        "y": 4,
    },

    "bg_note_expert": {
        "tex_name": "note_bg_expert",
        "sx": 0,
        "sy": 0,
        "w": 16,
        "h": 4,
        "scale_x":24.875,
        "scale_y":13.5,
        "x": 82,
        "y": 4,
    },

    "bg_note_master": {
        "tex_name": "note_bg_master",
        "sx": 0,
        "sy": 0,
        "w": 16,
        "h": 4,
        "scale_x":24.875,
        "scale_y":13.5,
        "x": 82,
        "y": 4,
    },    

    "text_normal": {
        "tex_name": "text_normal",
        "sx": 0,
        "sy": 0,
        "w": 90,
        "h": 32,
    },

    "text_expert": {
        "tex_name": "text_expert",
        "sx": 0,
        "sy": 0,
        "w": 90,
        "h": 32,
    },

    "text_master": {
        "tex_name": "text_master",
        "sx": 0,
        "sy": 0,
        "w": 90,
        "h": 32,
    },    

    "taiko": {
        "tex_name": "taiko",
        "sx": 14,
        "sy": 0,
        "w": 82,
        "h": 87,
    },

    "soulbar_shadow0": {
        "tex_name": "soulbar_shadow0",
        "sx": 0,
        "sy": 0,
        "w": 512,
        "h": 64,
    },

    "soulbar_shadow1": {
        "tex_name": "soulbar_shadow12",
        "sx": 0,
        "sy": 0,
        "w": 512,
        "h": 64,
    },

    "soulbar_shadow2": {
        "tex_name": "soulbar_shadow12",
        "sx": 0,
        "sy": 0,
        "w": 512,
        "h": 64,
    },

    "soulbar_shadow3": {
        "tex_name": "soulbar_shadow3",
        "sx": 0,
        "sy": 0,
        "w": 512,
        "h": 64,
    },

    "soulbar_bg0": {
        "tex_name": "soulbar_bg012",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_bg1": {
        "tex_name": "soulbar_bg012",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_bg2": {
        "tex_name": "soulbar_bg012",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_bg3": {
        "tex_name": "soulbar_bg3",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_frame0": {
        "tex_name": "soulbar_frame0",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_frame1": {
        "tex_name": "soulbar_frame12",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_frame2": {
        "tex_name": "soulbar_frame12",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },

    "soulbar_frame3": {
        "tex_name": "soulbar_frame3",
        "sx": 0,
        "sy": 0,
        "w": 256,
        "h": 32,
    },    
}

def dump(cfg):

    import struct
    base_fmt = "<I24sIIIIIIIIffIfI"
    base_size = struct.calcsize(base_fmt)

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
    
    return data


def dump_f(fname, cfg):
    data = dump(cfg)
    f = open("../frame/" + fname + ".f", "wb")
    f.write(data)
    f.close()    
    
if __name__ == "__main__":
    for fname, cfg in DATA.iteritems():
        dump_f(fname, cfg)
