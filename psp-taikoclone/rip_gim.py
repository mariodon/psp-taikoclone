import struct
import sys

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

def rip2(fname):
	f = open(fname, "rb")
	data = f.read()
	f.close()
	
	if not data.startswith("LWARC"):
		return
	
	off_filecnt = struct.unpack("<I", data[0xc:0x10])[0] + 0x8
	off_filenameinfo = struct.unpack("<I", data[0x14:0x18])[0]
	off_filesizeinfo = struct.unpack("<I", data[0x18:0x1c])[0]
	off_filedata = struct.unpack("<I", data[0x1c:0x20])[0]
	
	filecnt = struct.unpack("<I", data[off_filecnt:off_filecnt+0x4])[0]
	
	cur_off_filenameinfo = off_filenameinfo
	flist = []
	max_fname_len = 0
	for i in xrange(filecnt):
		fname_len = struct.unpack("<I", 
			data[cur_off_filenameinfo:cur_off_filenameinfo+0x4])[0]
		fname = struct.unpack("<%ds" % fname_len, 
			data[cur_off_filenameinfo+0x8:cur_off_filenameinfo+0x8+fname_len])[0]
		fname = fname.rstrip('\x00')
		
		fsize = struct.unpack("<I",
			data[off_filesizeinfo+i*0x8:off_filesizeinfo+i*0x8+0x4])[0]
		foffset = struct.unpack("<I", 
			data[off_filesizeinfo+i*0x8+0x4:off_filesizeinfo+i*0x8+0x8])[0]
			
		cur_off_filenameinfo += 0x8 + fname_len
		
		flist.append((fname, foffset+off_filedata, fsize))
		max_fname_len = max(max_fname_len, len(fname))
		
	fmt_str = "filename: %%%ds" % max_fname_len + "\toff: 0x%x,\tsize: 0x%x"
	
	for finfo in flist:
		print fmt_str % finfo
	
#for i in xrange(1, 1540):
#    rip("part%d" % i)
#rip2("E:\PSP\part10")

if __name__ == "__main__":
	rip2(sys.argv[1])