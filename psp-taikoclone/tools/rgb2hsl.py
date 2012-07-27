def rgb2hls(r, g, b):
    maxv = max(r, g, b)
    minv = min(r, g, b)

    # calc h
    if maxv == minv:
        h = 0.0
    elif maxv == r and g >= b:
        h = 60.0 * (g - b) / (maxv - minv)
    elif maxv == r and g < b:
        h = 60.0 * (g - b) / (maxv - minv) + 360.0
    elif maxv == g:
        h = 60.0 * (b - r) / (maxv - minv) + 120.0
    elif maxv == b:
        h = 60.0 * (r - g) / (maxv - minv) + 240.0
    else:
        assert False, "impossible"

    # calc l
    l = 0.5 * (maxv + minv)

    # calc s
    if l == 0 or maxv == minv:
        s = 0.0
    elif 0 < l <= 0.5:
        s = 0.5 * (maxv - minv) / l
    elif l > 0.5:
        s = 0.5 * (maxv - minv) / (1.0 - l)

    return (h, l, s)

def hls2rgb(h, l, s):
    if s == 0:
        r, g, b = l, l, l
    else:
        if l < 0.5:
            q = l * (1 + s)
        else:
            q = l + s - l * s
        p = 2 * l - q
        hk = h / 360.0

        def format_t(t):
            if t < 0:
                t += 1.0
            elif t > 1:
                t -= 1.0
            return t

        def calc_color_comp(t):
            if t < 1.0 / 6:
                return p + ((q - p) * 6.0 * t)
            elif t < 0.5:
                return q
            elif t < 2.0 / 3:
                return p + ((q - p) * 6.0 * (2.0 / 3 - t))
            else:
                return p

        t = (hk + 1.0 / 3, hk, hk - 1.0 / 3)
        t2 = map(format_t, t)

        return map(calc_color_comp, t2)

    pass

def colorize(r, g, b, hue, saturation, lightness):
    lum = r * 0.2126 + g * 0.7152 + b * 0.0722
    print "old lum = %f" % lum
    if lightness < 0:
        lum = lum * (lightness + 100) / 100.0
    else:
        lum = lum + lightness * (255 - lum) / 100.0

    h = hue
    s = saturation / 100.0
    l = lum / 255.0

    return hls2rgb(h, l, s)

def test(r, g, b):
    r_f1, g_f1, b_f1 = colorize(r, g, b, 63, 50, 52)

    return map(lambda x: round(x * 255), (r_f1, g_f1, b_f1))

if __name__ == "__main__":
    print rgb2hls(128, 112, 0)
    print rgb2hls(136, 112, 8)
    print rgb2hls(152, 128, 16)
    print rgb2hls(160, 136, 16)
    print rgb2hls(174, 144, 16)

    print rgb2hls(248, 248, 126)
    print rgb2hls(248, 216, 64)
    print rgb2hls(184, 144, 24)
    print rgb2hls(248, 168, 24)    
