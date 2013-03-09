import struct
import sys
import optparse

blend_mode_2_name = {
	0 : "normal",
	1 : "normal",
	2 : "layer",
	3 : "multiply",
	4 : "screen",
	5 : "lighten",
	6 : "darken",
	7 : "difference",
	8 : "add",
	9 : "subtract",
	10 : "invert",
	11 : "alpha",
	12 : "erase",
	13 : "overlay",
	14 : "hardlight",
}

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

def get_frame_label_dict(lm_data):
	ret = {}
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	while True:
		tag_type, = struct.unpack("<H", data[:0x2])
		if tag_type == 0xFF00:
			break
		if tag_type == 0x0027:
			sprite_id, = struct.unpack("<H", data[0x4:0x6])
			frame_label_cnt, = struct.unpack("<H", data[0xa:0xc])
			ret.setdefault(sprite_id, {})
			for i in xrange(frame_label_cnt):
				data = seek_next_tag(data, (0x002b,))
				frame_label_idx, the_frame = struct.unpack("<HH", 
					data[0x4:0x8])
				frame_label = symbol_list[frame_label_idx]
				ret[sprite_id][frame_label] = the_frame + 1
		data = seek_next_tag(data)
	
	return ret
						
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

def seek_next_tag(data, id=None):
	assert data, "No Tags Any More"
	tag_type, tag_size = struct.unpack("<HH", data[:0x4])
	data = data[tag_size * 4 + 4:]
	
	# Has Next Tag?
	if len(data):
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
	else:
		return data
		
	if id is None or tag_type in id:
		return data
	else:
		return seek_next_tag(data, id)
		
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
			frame = struct.unpack("<H", data[0x6:0x8])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\t%s, frame=%x" % (tag_type, off, \
				tag_size_bytes, symbol_list[symbol_idx], frame)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]	

def list_tag0027_symbol(lm_data):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x0027:
			characterID = struct.unpack("<I", data[0x4:0x8])[0]
			text, frame_label_cnt = struct.unpack("<HH", data[0x8:0xC])
			tag0001_cnt = struct.unpack("<H", data[0xC:0xE])[0]
			key_frame_cnt, = struct.unpack("<H", data[0xE:0x10])
			max_depth = struct.unpack("<I", data[0x10:0x14])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tCharacterID=%d\tframe=%d,\tlabel=%d,\tmaxdepth=%d,htmltext=%d,key=%d" % \
				(tag_type, off, tag_size_bytes, characterID, tag0001_cnt, frame_label_cnt, max_depth, text, key_frame_cnt)
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
			character_id, size_idx, shape_cnt = struct.unpack("<IHH", data[0x4:0xc])
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tCharacterID=%d,\tsize_idx=%d,\tshape_cnt=%d" \
			 % (tag_type, off, tag_size_bytes, character_id, size_idx, shape_cnt)
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
	
	tex_size_list = list_tagF004_symbol(lm_data)
	
	x_min = y_min = 1000000000000
	x_max = y_max = -1000000000000
	size = (x_min, y_min, x_max, y_max)
					
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF022:
			
			# check if boundary table matches
#			print (x_min, y_min, x_max, y_max)	
#			print size		
#			print "-" * 20
#			if 0.0 != x_min-size[0] or 0.0 != y_min-size[1] or 0.0 != x_max-size[2] or 0.0 != y_max-size[3]:
#				print "padding: %f %f %f %f" % (x_min-size[0], y_min-size[1], x_max-size[2], y_max-size[3])
			BOUND_ERR_MSG = "texture boundary not match boundary table!"
			assert x_min >= size[0], BOUND_ERR_MSG
			assert y_min >= size[1], BOUND_ERR_MSG
	   		assert x_max <= size[2], BOUND_ERR_MSG
			assert y_max <= size[3], BOUND_ERR_MSG

			x_min = y_min = 1000000000000
   			x_max = y_max = -1000000000000			

			
			id, depth = struct.unpack("<IH", data[0x4:0xa])
			size = tex_size_list[depth]
			print "CharacterID=%d, size: (%d, %d, %d, %d)" % (id, size[0], size[1], size[2], size[3])
					
		if tag_type == 0xF023:
			id, flag = struct.unpack("<HH", data[0x44:0x48])
			fv_list = []
			for off1 in xrange(0x4, 0x44, 0x4):
				fv = struct.unpack("<f", data[off1:off1+0x4])[0]
				fv_list.append(fv)	# (x, y, u, v)

			x_min = min(x_min, fv_list[0], fv_list[4], fv_list[8], fv_list[12])
			x_max = max(x_max, fv_list[0], fv_list[4], fv_list[8], fv_list[12])
			y_min = min(y_min, fv_list[1], fv_list[5], fv_list[9], fv_list[13])
			y_max = max(y_max, fv_list[1], fv_list[5], fv_list[9], fv_list[13])
				
			if flag == 0x41:
				cnt = 0
				for sb in symbol_list:
					if sb.endswith(".png") and cnt == id:
						print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,\t%s" \
							% (tag_type, off, tag_size_bytes, sb)
						print "\t\t",fv_list[:4]
						print "\t\t",fv_list[4:8]
						print "\t\t",fv_list[8:12]
						print "\t\t",fv_list[12:]
						break
						
					if sb.endswith(".png"):
						cnt += 1
			else:
				pass
				print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,fill_color_idx=0x%x" \
					% (tag_type, off, tag_size_bytes,id)
				print "\t\t",fv_list[:4]
				print "\t\t",fv_list[4:8]
				print "\t\t",fv_list[8:12]
				print "\t\t",fv_list[12:]			
				
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
			v1, place_object2_cnt = struct.unpack("<hh", data[0x4:0x8])
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tv1=0x%x,\tplace_object2_cnt=%d" %	 \
				(tag_type, off, tag_size_bytes, v1, place_object2_cnt)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	
def get_xref(lm_data):
	ref_table = {}
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x0004:
			id = struct.unpack("<H", data[0x4:0x6])[0]
			name_idx = struct.unpack("<h", data[0xa:0xc])[0]
			if name_idx > 0:
				name = symbol_list[name_idx]
				name_set = ref_table.setdefault(id, set())
				name_set.add(name)
				ref_table[id] = name_set
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	return ref_table
	
def list_tag0004_symbol(lm_data):
	ref_table = get_xref(lm_data)
	
	xy_list = list_tagF103_symbol(lm_data)
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	color_list = list_tagF002_symbol(lm_data)
	matrix_list = list_tagF003_symbol(lm_data)
	off = 0x40
	off0x4_cnt = {}
	flag = False
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x0027:
			flag = True
			id = struct.unpack("<H", data[0x4:0x6])[0]
			print "=====================, CharacterID=%d, %s" % (id, id in ref_table and "Ref As %s" % ("".join(list(ref_table[id]))) or "")
		elif tag_type == 0x0001:
			print "Frame %d, cmd_cnt=%d" % struct.unpack("<HH", data[0x4:0x8])
		elif tag_type == 0xf014:
			print ">>>>>>>>>Do ClipAction: %d" % struct.unpack("<H", data[0x4:0x6])
		elif tag_type == 0xf105:
			print
			print ">>>>>>>>>KeyFrame: v=%d" % struct.unpack("<H", data[0x4:0x6])
		elif tag_type == 0x000c:
			print ">>>>>>>>>Do Action %d" % struct.unpack("<H", data[0x4:0x6])
		elif tag_type == 0x0005:
			print ">>>>>>>>>RemoveObject at depth%d" % struct.unpack("<H", data[0x6:0x8])
		elif tag_type == 0x002b:
			idx, frame, unk = struct.unpack("<HHI", data[0x4:0xc])
			print ">>>>>>>>>FrameLabel: %s@%d, %d" % (symbol_list[idx], frame, unk)
		elif flag and tag_type not in (0x0004, 0xFF00):
			print
			print "!!!!!!!!!!!!!!!!!!!!!"
			print "unknown tag 0x%04x, off=0x%x" % (tag_type, off)
			print "!!!!!!!!!!!!!!!!!!!!!"
			print
		if tag_type == 0x0004:
			v_cnt = struct.unpack("<H", data[0x18:0x1a])[0]
			off0x4_cnt[v_cnt] = off0x4_cnt.setdefault(v_cnt, 0) + 1		
			v_list = []
			v_list.append(struct.unpack("<H", data[0x4:0x6])[0])
			trans_idx = struct.unpack("<H", data[0x18:0x1a])[0]
			blend_mode, = struct.unpack("<H", data[0xE:0x10])
			blend_mode_name = blend_mode_2_name[blend_mode]
			if trans_idx == 0xFFFF:
				translate = scale = rotateskew = "null"
			elif (trans_idx & 0x8000) == 0:
				translate = "(%.1f, %.1f)" % (matrix_list[trans_idx][4],
					matrix_list[trans_idx][5])
				scale = "(%.1f, %.1f)" % (matrix_list[trans_idx][0],
					matrix_list[trans_idx][3])
				rotateskew = "(%.1f, %.1f)" % (
					matrix_list[trans_idx][1], 
					matrix_list[trans_idx][2])
			else:
				translate = "(%.1f, %.1f)" % xy_list[trans_idx & 0xFFF]
				scale = rotateskew = "null"
			clip_action_cnt = struct.unpack("<H", data[0x20:0x22])[0]
			name_idx = struct.unpack("<H", data[0xa:0xc])[0]
			name = symbol_list[name_idx] or "null"
			depth = struct.unpack("<H", data[0x10:0x12])[0]
			v_list.append(depth)
			v_list.append(translate)
			v_list.append(scale)
			v_list.append(rotateskew)
			v_list.append(struct.unpack("<h", data[0x6:0x8])[0])
			flags = struct.unpack("<H", data[0xc:0xe])[0]
			flags_str = ""
			flags_str += (flags & 1) and "N" or "-"
			flags_str += (flags & 2) and "M" or "-"
			if flags & (~3) != 0:
				print "==============>new flags ! %d " % flags
			v_list.append(flags_str)
			v_list.append(struct.unpack("<H", data[0x12:0x14])[0])
			color_mul_idx = struct.unpack("<h", data[0x1a:0x1c])[0]
			color_add_idx = struct.unpack("<h", data[0x1c:0x1e])[0]
			if color_mul_idx < 0:
				color_mul_str = "null"
			else:
				color_mul = color_list[color_mul_idx]
				color_mul_str = "(%.1f,%.1f,%.1f,%.1f)" % tuple([c/256.0 for c in color_mul])
			if color_add_idx < 0:
				color_add_str = "null"
			else:
				color_add = color_list[color_add_idx]
				color_add_str = "(%d,%d,%d,%d)" % tuple(color_add)			
			v_list.append(color_mul_str)
			v_list.append(color_add_str)
			v_list.append(clip_action_cnt)
			v_list.append(name)
			v_list.append(trans_idx)
						
			clip_depth, = struct.unpack("<H", data[0x12:0x14])
			
			if True:
				print "PlaceObject, off=0x%x,\tsize=0x%x" % (off,	tag_size_bytes)			
				print "\tID=%d,\tdepth=%d,\tpos=%s,\tscale=%s,\tskew=%s,\tInstID=%d,\tflags=%s,\t%d,\tcolMul=%s,\tcolAdd=%s\t,clipAction=%d\tname=%s,\ttrans_idx=%x" % tuple(v_list)
				if blend_mode_name != "normal":
					print "\t==> blend_mode = %s" % blend_mode_name
				if clip_depth > 0:
					print "\t==> clip depth = %d" % clip_depth
			
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
#
#	print "{{{{{{{{{{{{{{{{{{{{{{{{{{{"
#	for k, v in sorted(off0x4_cnt.items()):
#		print "\t%x:%d" % (k, v)
#	print "}}}}}}}}}}}}}}}}}}}}}}}}}}}"		

def list_tagF103_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	v_list = []
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF103:
			v_cnt = struct.unpack("<I", data[0x4:0x8])[0]
			for i in range(v_cnt):
				v_list.append(struct.unpack("<ff", data[0x8+i*0x8:0x8+i*0x8+0x8]))
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	return v_list

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
				v_list.append(struct.unpack("<ffff", data[0x8+i*0x10:0x8+i*0x10+0x10]))
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % \
				(tag_type, off, tag_size_bytes)
			for v in v_list:
				print "\t", v
			
			return tuple(v_list)
			
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
			
			return v_list
			
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
				v.append(symbol_list[v[1]])
				v_list.append(v)
			
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % \
				(tag_type, off, tag_size_bytes)
			for v in v_list:
				print "\t", v
				
			return v_list
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
						
def list_tagF005_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	action_record_list = []
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF005:
#			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % (tag_type, off, \
#				tag_size_bytes)   
			cnt, = struct.unpack("<I", data[0x4:0x8])
			as_off = 0x8
			for i in xrange(cnt):
				as_len, = struct.unpack("<I", data[as_off:as_off+0x4])
				as_record, = struct.unpack("<%ds" % as_len, 
					data[as_off+0x4:as_off+0x4+as_len])
				action_record_list.append(as_record)
				as_off += 0x4 + (as_len + 3) / 4 * 4
			break
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	
#	print "as record count %d" % len(action_record_list)
	return action_record_list

def list_tagF002_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	color_list = []
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF002:
			color_cnt, = struct.unpack("<I", data[0x4:0x8])
			for i in xrange(color_cnt):
				color = struct.unpack("<hhhh", \
					data[0x8+0x8*i:0x8+0x8*i+0x8])
				color_list.append(color)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	return color_list
			
def list_tagF003_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	matrix_list = []
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF003:
			v_cnt = struct.unpack("<I", data[0x4:0x8])[0]
			v_list = []
			for i in range(v_cnt):
				v_list.append(struct.unpack("<ffffff", data[0x8+i*0x4*6:0x8+i*0x4*6+0x4*6]))
				matrix_list.append(v_list[-1])
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	return matrix_list
					
def list_tagF00D_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF00D:	
			print "tag:0x%04x, off=0x%x,\tsize=0x%x" % (tag_type, off, \
				tag_size_bytes)
			print "\timg_cnt=%d, f018cnt=%d, mc_cnt=%d, f00e=%d, 0025cnt=%d, %d" % struct.unpack("<HHHHII", data[0x4:0x14])
			break
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
			
def list_tag0025_symbol(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	ret = []
	
	symbol_list = get_symbol_list(data)
	
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x0025:
			html_text_idx, = struct.unpack("<H", data[0x4:0x6])
			html_text = symbol_list[html_text_idx]
			print "html_text %s" % html_text.decode("utf8")
			pass
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	return ret
	
	
if __name__ == "__main__":

	parser = optparse.OptionParser()
	parser.add_option("-f", dest="filename")
	parser.add_option("-o", dest="outfile")
	parser.add_option("-t", action="store_true", dest="print_tag")
	parser.add_option("-S", action="store_true", dest="shuffle_tagF022")
	parser.add_option("-i", type="int", action="store", dest="tag_id")
	parser.add_option("-F", dest="subfile")
	
	(options, args) = parser.parse_args(sys.argv)
	
	if not options.filename:
		print "No filename is specified!"

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
			xy_list = list_tagF103_symbol(data)
			if xy_list:
				print "point list:"
				for i, point in enumerate(xy_list):
					print "0x%x\t" % i, point
		elif options.tag_id == 0xF001:
			f = open(options.filename, "rb")
			data = f.read()
			f.close()
		
			tag = data[0x40:]
			symbol_list = get_symbol_list(tag)
			print "symbols:"
			for i, symbol in enumerate(symbol_list[:65]):
				print "0x%x\t" % i, symbol			
		elif options.tag_id == 0xF002:
			color_list = list_tagF002_symbol(data)
			if color_list:
				print "color list:"
				for i, color in enumerate(color_list):
					print "0x%x\t" % i, color				
		elif options.tag_id == 0xF003:
			matrix_list = list_tagF003_symbol(data)
			if matrix_list:
				print "matrix list:"	
				for i, v in enumerate(matrix_list):
					print "0x%x\t[%.3f, %.3f, %.3f, %.3f, %.3f, %.3f]" % ((i,)+v)			
		elif options.tag_id == 0xF004:
			list_tagF004_symbol(data)
		elif options.tag_id == 0xF005:
			print "Actionscript!"
			list_tagF005_symbol(data)
		elif options.tag_id == 0xF007:
			list_tagF007_symbol(data)
		elif options.tag_id == 0xF00C:
			list_tagF00C_symbol(data)
		elif options.tag_id == 0xF00D:
			list_tagF00D_symbol(data)
		elif options.tag_id == 0x0025:
			res = list_tag0025_symbol(data)
					
	elif options.shuffle_tagF022:
		f = open(options.filename, "rb")
		data = f.read()
		f.close()
		
		fout = open(options.outfile, "wb")
		fout.write(shuffle_tagF022(data))
		fout.close()
	
	else:						# deal with sub files in a LWARC file
		rip_file(options.filename, options.subfile)