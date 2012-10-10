import struct

#1537_55 can not work right
#365_13
#356_3
#356_4
def rip(fname):
    f = open(fname, "rb")
    data = f.read()

    data2 = data.split("MIG.00.1")
    gim_cnt = 0
    #print len(data2)
    for d in data2:
        if d.startswith("PSP"):
            d = "MIG.00.1" + d
            size = struct.unpack("<I", d[0x14:0x18])[0]
            d = d[:0x10+size]
            i = d.rfind("\xff")
            name = d[i+0x10:]
            name = name.split('\x00')[0]
            assert name.endswith(".png")
            f2 = open("gim\\" + name[:-3] + "gim", "wb")
            f2.write(d)
            f2.close()
            gim_cnt += 1

    f.close()

for i in xrange(1, 1540):
    rip("part%d" % i)
#rip("part1")
