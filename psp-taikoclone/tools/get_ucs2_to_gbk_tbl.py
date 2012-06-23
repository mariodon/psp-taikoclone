import struct

f = open("gbkuni30.txt")
fout = open("ucs2_gbk.dat", "wb")

dict = {}
for line in f:
    line = line.strip()
    k, v = line.split(':')
    k = int(k, 16)
    v = int(v, 16)
    dict[k] = v
f.close()

for i in xrange(0x10000):
    v = dict.get(i, 0)
    fout.write(struct.pack("<H", v))
fout.close()

