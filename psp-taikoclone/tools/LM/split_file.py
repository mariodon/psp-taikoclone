import struct
import zlib

f = open("F:\PSP_GAME\USRDIR\HOME\BUILD.DAT", "rb")

data = f.read()
blocks = data.split("GARC")
off = 0

failed_off = []

for i, block in enumerate(blocks):

    if not block:
        continue

    block_len = len(block) + 4

    print "off=0x%08x" % off

    f2 = open("part%d" % i, "wb")
    #f2 = open("part%d_0x%08x" % (i, off), "wb")
    block = block[0x30:]
    while True:
        size = struct.unpack("<I", block[:0x4])[0]
        flag = struct.unpack("<I", block[0x4:0x8])[0]
        print "\tblock_size=0x%08x, flag=%d" % (size, flag)
        if size == 0:
            break
        if flag:
            break
        data = block[0x8:0x8+size]
        data2 = zlib.decompress(data)
        print "\t\tblock_size_d=0x%08x" % len(data2)
        f2.write(data2)

        off2 = 0x8 + size
        if off2 % 4 != 0:
            off2 = off2 // 4 * 4 + 4
        block = block[off2:]

    if flag:
        failed_off.append((off,flag,i))

    f2.close()
    off += block_len        

print
print "failed num = %d", len(failed_off)
for off,flag,i in failed_off:
    print "%d, unknown flag = %d, off = 0x%08x" % (i,flag, off)

f.close()



