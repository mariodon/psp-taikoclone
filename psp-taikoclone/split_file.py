import struct
import zlib

f = open("BUILD.DAT", "rb")

data = f.read()
blocks = data.split("GARC")
for i, block in enumerate(blocks):
    try:
        f2 = open("part%d" % i, "wb")
        size = struct.unpack("<I", block[:4])[0]
        data2 = zlib.decompress(block[0x38:size-4])
        f2.write(data2)
        f2.close()
    except:
        print "block %d error" % i
        print "data = "
        print "len block =", len(block)


f.close()



