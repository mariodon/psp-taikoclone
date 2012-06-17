import struct

f = open("CP932.TXT")
dict = {}
for line in f:
    if line.startswith("#"):
        continue
    a, b = line.split('\t')[:2]
    try:
        a = int(a, 16)
        b = int(b, 16)
    except ValueError:
        continue

    # ignore duplicate
    if b in dict:
        continue

    dict[b] = a

l = []
for k, v in dict.iteritems():
    l.append((k, v))
l.sort()

sec_count = 0
sections = []
sec_head = sec_tail = None

for e in l:
    if sec_tail is None:
        sec_head = sec_tail = e[0]
    elif e[0] - sec_tail < 1000:
        sec_tail = e[0]
    else:
        sections.append((sec_head, sec_tail))
        sec_head = sec_tail = e[0]
if sec_head is not None:
    sections.append((sec_head, sec_tail))

tot_len = 0
for sec_head, sec_tail in sections:
    print sec_head, sec_tail
    tot_len += sec_tail - sec_head + 1
print len(sections)
print "tot_table length=%d" % tot_len

error_code = 0x0000

fo = open("ucs2_cp932.dat", "wb")
fo.write(struct.pack("<H", len(sections)))
for sec_head, sec_tail in sections:
    fo.write(struct.pack("<HH", sec_head, sec_tail))
for sec_head, sec_tail in sections:
    for i in xrange(sec_head, sec_tail+1):
        v = dict.get(i, error_code)
        fo.write(struct.pack("<H", v))
fo.close()


if __name__ == "__main__":
    tbl_f = open("ucs2_cp932.dat", "rb")
    str = u'\u308f\u3093\u306b\u3083\u30fc\u30ef\u30fc\u30eb\u30c9.ogg'
    sjis_str = ""

    sec_count = struct.unpack("<H", tbl_f.read(2))[0]
    sec_list = []
    for i in xrange(sec_count):
        a, b = struct.unpack("<HH", tbl_f.read(4))
        sec_list.append((a,b))
    data = []
    for i in xrange(tot_len):
        try:
            data.append(struct.unpack("<H", tbl_f.read(2))[0])
        except:
            pass
    assert len(data) == tot_len, len(data)

    print sec_count
    print sec_list

    for val in str:
        val = ord(val)
        id = 0
        for sec_head, sec_tail in sec_list:
            if val >= sec_head and val <= sec_tail:
                id += val-sec_head
                break
            else:
                id += sec_tail-sec_head+1
        print val, id, data[id]
        if data[id] < 256:
            sjis_str += chr(data[id])
        else:
            sjis_str += chr(data[id] >> 8)
            sjis_str += chr(data[id] & 0xFF)

    print sjis_str.decode("shift-jis")
