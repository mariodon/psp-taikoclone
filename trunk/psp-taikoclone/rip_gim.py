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

def list_tag0027_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x0027:
			depth = struct.unpack("<I", data[0x4:0x8])[0]
			tag0001_cnt = struct.unpack("<H", data[0xC:0xE])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\ttag0001=%d\tCharacterID=%d" % \
				(tag_type, off, tag_size_bytes, tag0001_cnt, depth)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
		
def list_tagF022_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF022:
			depth = struct.unpack("<I", data[0x4:0x8])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tCharacterID=%d" % \
				(tag_type, off, tag_size_bytes, depth)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
		
def list_tagF023_symbol(lm_data, off1, off2):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF023:
			v = struct.unpack("<H", data[off1:off2])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tv=%x" % (tag_type, off, \
				tag_size_bytes, v)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]

def list_tagF023_img(lm_data):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF022:
			id, depth = struct.unpack("<IH", data[0x4:0xa])
			print "CharacterID=%d, depth:%d" % (id, depth)
		if tag_type == 0xF023:
			id, flag = struct.unpack("<HH", data[0x44:0x48])
			fv_list = []
			for off1 in xrange(0x4, 0x44, 0x4):
				fv = struct.unpack("<f", data[off1:off1+0x4])[0]
				fv_list.append(fv)
			if flag == 0x41:
				cnt = 0
				for sb in symbol_list:
					if sb.endswith(".png") and cnt == id:
						print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,\t%s" \
							% (tag_type, off, tag_size_bytes, sb)
#						print "\t\t",fv_list[:4]
#						print "\t\t",fv_list[4:8]
#						print "\t\t",fv_list[8:12]
#						print "\t\t",fv_list[12:]
						break
					if sb.endswith(".png"):
						cnt += 1
			else:
				print "\ttag:0x%04x, off=0x%x,\tsize=0x%x" \
					% (tag_type, off, tag_size_bytes)
#				print "\t\t",fv_list[:4]
#				print "\t\t",fv_list[4:8]
#				print "\t\t",fv_list[8:12]
#				print "\t\t",fv_list[12:]			
				
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
			
def list_tag0001_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0001:
			v1, v2 = struct.unpack("<hh", data[0x4:0x8])
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tv1=0x%x,\tv2=0x%x" % 	\
				(tag_type, off, tag_size_bytes, v1, v2)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	
def list_tag0004_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	off0x4_cnt = {}
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x0027:
			print "====================="
		elif tag_type == 0x0001 or tag_type == 0xf014:
			print
		if tag_type == 0x0004:
			v_cnt = struct.unpack("<H", data[0x4:0x6])[0]
			off0x4_cnt[v_cnt] = off0x4_cnt.setdefault(v_cnt, 0) + 1		
			v_list = []
			v_list.append(struct.unpack("<H", data[0x4:0x6])[0])
			v_list.append(struct.unpack("<H", data[0x6:0x8])[0])
			v_list.append(struct.unpack("<H", data[0xc:0xe])[0])
			v_list.append(struct.unpack("<H", data[0x10:0x12])[0])
			v_list.append(struct.unpack("<H", data[0x12:0x14])[0])
			v_list.append(struct.unpack("<H", data[0x18:0x1a])[0])
			v_list.append(struct.unpack("<H", data[0x1a:0x1c])[0])
			v_list.append(struct.unpack("<H", data[0x1c:0x1e])[0])
						
			if True or v_list[4] == 1:
				print "tag:0x%04x, off=0x%x,\tsize=0x%x" % (tag_type, off, \
					tag_size_bytes)			
				print "\tID=%d,\t0x%04x,\t%d,\t%d,\t%d,\t0x%04x,\t0x%04x,\t0x%04x" % tuple(v_list)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]

	print "{{{{{{{{{{{{{{{{{{{{{{{{{{{"
	for k, v in off0x4_cnt.items():
		print "\t%d:%d" % (k, v)
	print "}}}}}}}}}}}}}}}}}}}}}}}}}}}"		

def list_tagF103_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF103:
			v_cnt = struct.unpack("<I", data[0x4:0x8])[0]
			v_list = []
			for i in range(v_cnt):
				v_list.append(struct.unpack("<ff", data[0x8+i*0x8:0x8+i*0x8+0x8]))
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % \
				(tag_type, off, tag_size_bytes)
			print "\t", v_list
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]

def list_tagF004_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF004:
			
			v_cnt = struct.unpack("<I", data[0x4:0x8])[0]
			v_list = []
			for i in range(v_cnt):
				v_list.append(struct.unpack("<ffff", data[0x8+i*0x8:0x8+i*0x8+0x10]))
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % \
				(tag_type, off, tag_size_bytes)
			for v in v_list:
				print "\t", v
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
		
def list_tagF00C_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF00C:
			v_list = struct.unpack("<HHHHHHHHfffff", data[0x4:0x28])
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % \
				(tag_type, off, tag_size_bytes)
			print "\t", v_list
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
		
def list_tagF007_symbol(lm_data):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF007:
			v_cnt = struct.unpack("<I", data[0x4:0x8])[0]
			v_list = []
			for i in range(v_cnt):
				v = list(struct.unpack("<HHff", data[0x8+i*0xc:0x8+i*0xc+0xc]))
				j = 0
				for sb in symbol_list:
					if sb.endswith(".png") and j == i:
						v.append(sb)
						break
					if sb.endswith(".png"):
						j += 1
						
				v_list.append(v)
			
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % \
				(tag_type, off, tag_size_bytes)
			for v in v_list:
				print "\t", v
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
												
def shuffle_tagF022(lm_data):
	head = ""
	tail = ""
	tagF022_list = []
	tagF022 = ""
	
	head += lm_data[:0x40]
	data = lm_data[0x40:]
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		
		if tagF022 and tag_type != 0xF023:
			tagF022_list.append(tagF022)
			tagF022 = ""
			#print "TAG BLOCK END"
			
		if not tagF022 and tag_type == 0xF022:
			pass
			#print "TAG BLOCK BEGIN"
			
		if tagF022 or tag_type == 0xF022:
			tagF022 += data[:tag_size_bytes]
			#print "\tTAG%04x" % tag_type
		elif tagF022_list:
			#print "================"
			break
		else:
			head += data[:tag_size_bytes]
			#print "APPEND TO HEAD TAG%04x" % tag_type
				
		data = data[tag_size_bytes:]
	tail = data
	
	tot_len = len(head) + len(tail) + sum([len(s) for s in tagF022_list])
	assert tot_len == len(lm_data), "%x %x (%x) != %x"  % (len(head), len(tail), len(tagF022_list), len(lm_data))
	
	import random
	random.shuffle(tagF022_list)
	
	return head+"".join(tagF022_list)+tail
			
if __name__ == "__main__":
	parser = optparse.OptionParser()
	parser.add_option("-f", dest="filename")
	parser.add_option("-o", dest="outfile")
	parser.add_option("-s", action="store_true", dest="print_symbol")
	parser.add_option("-t", action="store_true", dest="print_tag")
	parser.add_option("-S", action="store_true", dest="shuffle_tagF022")
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
		elif options.tag_id == 0x0027:
			list_tagF022_symbol(data)
			print "==============="
			list_tag0027_symbol(data)
		elif options.tag_id == 0xF023:
#			for off in range(0, 0x48, 2):
#				print "off %x~%x" % (off, off+2)
#				list_tagF023_symbol(data, off, off+2)
			list_tagF023_img(data)
		elif options.tag_id == 0xF022:
			list_tagF022_symbol(data)
			print "==============="
			list_tag0027_symbol(data)
		elif options.tag_id == 0x0001:
			list_tag0001_symbol(data)
		elif options.tag_id == 0x0004:
			list_tag0004_symbol(data)
		elif options.tag_id == 0xF103:
			list_tagF103_symbol(data)
		elif options.tag_id == 0xF004:
			list_tagF004_symbol(data)
		elif options.tag_id == 0xF007:
			list_tagF007_symbol(data)
		elif options.tag_id == 0xF00C:
			list_tagF00C_symbol(data)			
					
	elif options.shuffle_tagF022:
		f = open(options.filename, "rb")
		data = f.read()
		f.close()
		
		fout = open(options.outfile, "wb")
		fout.write(shuffle_tagF022(data))
		fout.close()
	
	else:						# deal with sub files in a LWARC file
		rip_file(options.filename, options.subfile)