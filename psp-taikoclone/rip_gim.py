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
    print len(data2)
    for d in data2:
        if d.startswith("PSP"):
            d = "MIG.00.1" + d
            size = struct.unpack("<I", d[0x14:0x18])[0]
            d = d[:0x10+size]
            f2 = open("gim\%s_%d.gim" % (fname, gim_cnt), "wb")
            f2.write(d)
            f2.close()
            gim_cnt += 1

    f.close()

for i in xrange(1, 1540):
    rip("part%d" % i)
#rip("part1_0x00000000")
