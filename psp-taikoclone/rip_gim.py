import struct
import sys
import optparse

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
	return flist

def rip_file(fname, fname2=None):
    f = open(fname, "rb")
    data = f.read()
    f.close()

    flist = rip2(fname)

    for _fname, foffset, fsize in flist:
        if _fname == fname2:
            f = open(fname2, "wb")
            f.write(data[foffset:foffset+fsize])
            f.close()
            break

def get_symbol_list(tag):
	symbol_list = []
	tag_type, tag_size = struct.unpack("<HH", tag[:0x4])
	if tag_type != 0xF001:
		print "Unkown Tag %x" % tag_type
		return ()
	symbol_count = struct.unpack("<I", tag[0x4:0x8])[0]
	
	tag = tag[0x8:]
	for i in xrange(symbol_count):
		symbol_len = struct.unpack("<I", tag[:0x4])[0]
		symbol_list.append(\
			struct.unpack("<%ds" % symbol_len, tag[0x4:0x4+symbol_len])[0])
		tag = tag[symbol_len / 4 * 4 + 8:]
		
	return symbol_list

def tag_list(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		print "tag:0x%04x, off=0x%x,\tsize=0x%x" % (tag_type, off, \
			tag_size_bytes)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
		
def list_tag002b_symbol(lm_data):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x002b:
			symbol_idx = struct.unpack("<H", data[0x4:0x6])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\t%s" % (tag_type, off, \
				tag_size_bytes, symbol_list[symbol_idx])
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]	

if __name__ == "__main__":
	parser = optparse.OptionParser()
	parser.add_option("-f", dest="filename")
	parser.add_option("-s", action="store_true", dest="print_symbol")
	parser.add_option("-t", action="store_true", dest="print_tag")
	parser.add_option("-i", type="int", action="store", dest="tag_id")
	parser.add_option("-F", dest="subfile")
	
	(options, args) = parser.parse_args(sys.argv)
	
	if not options.filename:
		print "No filename is specified!"
	
	if options.print_symbol:	# print symbols in LM file
		f = open(options.filename, "rb")
		data = f.read()
		f.close()
		
		tag = data[0x40:]
		symbol_list = get_symbol_list(tag)
		print "symbols:"
		for symbol in symbol_list:
			print symbol.decode("utf8")
	elif options.print_tag:		# print tag info in LM file
		f = open(options.filename, "rb")
		data = f.read()
		f.close()
		
		if options.tag_id is None:
			tag_list(data)
		elif options.tag_id == 0x002b:
			list_tag002b_symbol(data)
			
	else:						# deal with sub files in a LWARC file
		rip_file(options.filename, options.subfile)