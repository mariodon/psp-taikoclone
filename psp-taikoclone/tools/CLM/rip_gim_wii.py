import struct
import sys
import optparse
import lm_format_wii as format
import read_tag_by_fmt as tag_reader

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

def get_max_characterID(lm_data):
	v_list = list_tagF00C_symbol(lm_data)
	return v_list[3]
	
def read_tagF022(tag):
	tag_size, = struct.unpack(">I", tag[0x4: 0x8])
	tag_size_bytes = tag_size * 4 + 8

	res = tag_reader.read_tag(format.DATA[0xF022], tag)
	character_id = res["character_id"]
	unk1 = res["const0_0"]
	assert unk1 == 0
	size_idx = res["size_idx"]
	f023_cnt = res["f023_cnt"]
	f024_cnt = res["f024_cnt"]
	if f024_cnt is None:
		f024_cnt = 0
	
	return character_id, unk1, size_idx, f023_cnt, f024_cnt

def read_tagF023(data):
	uv_list = struct.unpack(">" + "f" * 16, data[0x8: 0x8 + 16 * 0x4])
	flag, idx = struct.unpack(">HH", data[0xc + 16 * 0x4: 0x10 + 16 * 0x4])
	return idx, uv_list
	
def iter_tag(lumen, type_set=None):
	lumen = lumen[0x40:]
	
	_type_set = type_set or ()
	off = 0x40
	
	while lumen:
		tag_type, tag_size = struct.unpack(">II", lumen[:0x8])
		tag_size_bytes = tag_size * 4 + 8
		
#		assert tag_type in format.DATA, "Not Analyzed Tag!!! off=%x 0x%04x" % (off, tag_type)
		
		if not _type_set or tag_type in _type_set:
			yield off, tag_type, tag_size_bytes, lumen
		if tag_type == 0xFF00:
			break
			
		off += tag_size_bytes
		lumen = seek_next_tag(lumen)
		
def rip(fname):
	f = open(fname, "rb")
	data = f.read()

	data2 = data.split("MIG.00.1")
	gim_cnt = 0
	#print len(data2)
	for d in data2:
		if d.startswith("PSP"):
			d = "MIG.00.1" + d
			size = struct.unpack(">I", d[0x14:0x18])[0]
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
	off_filecnt = struct.unpack(">I", data[0xc:0x10])[0] + 0x8
	off_filenameinfo = struct.unpack(">I", data[0x14:0x18])[0]
	off_filesizeinfo = struct.unpack(">I", data[0x18:0x1c])[0]
	off_filedata = struct.unpack(">I", data[0x1c:0x20])[0]
	
	filecnt = struct.unpack(">I", data[off_filecnt:off_filecnt+0x4])[0]
	
	cur_off_filenameinfo = off_filenameinfo
	flist = []
	max_fname_len = 0
	for i in xrange(filecnt):
		fname_len = struct.unpack(">I", 
			data[cur_off_filenameinfo:cur_off_filenameinfo+0x4])[0]
		fname = struct.unpack(">%ds" % fname_len, 
			data[cur_off_filenameinfo+0x8:cur_off_filenameinfo+0x8+fname_len])[0]
		fname = fname.rstrip('\x00')
		
		fsize = struct.unpack(">I",
			data[off_filesizeinfo+i*0x8:off_filesizeinfo+i*0x8+0x4])[0]
		foffset = struct.unpack(">I", 
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
		tag_type, = struct.unpack(">H", data[:0x2])
		if tag_type == 0xFF00:
			break
		if tag_type == 0x0027:
			sprite_id, = struct.unpack(">H", data[0x4:0x6])
			frame_label_cnt, = struct.unpack(">H", data[0xa:0xc])
			ret.setdefault(sprite_id, {})
			for i in xrange(frame_label_cnt):
				data = seek_next_tag(data, (0x002b,))
				frame_label_idx, the_frame = struct.unpack(">HH", 
					data[0x4:0x8])
				frame_label = symbol_list[frame_label_idx]
				ret[sprite_id][frame_label] = the_frame + 1
		data = seek_next_tag(data)
	
	return ret
						
def get_symbol_list(tag):
	res = tag_reader.read_tag(format.DATA[0xF001], tag)
	ret = [symbol["symbol"] or "" for symbol in res["symbol_list"]]

	return ret

def seek_next_tag(data, id=None):
	assert data, "No Tags Any More"
	_, tag_type, tag_size = struct.unpack(">HHI", data[:0x8])
	data = data[tag_size * 4 + 8:]
	
	# Has Next Tag?
	if len(data):
		_, tag_type, tag_size = struct.unpack(">HHI", data[:0x8])
	else:
		return data
		
	if id is None or tag_type in id:
		return data
	else:
		return seek_next_tag(data, id)
		
def tag_list(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data):
		print "tag:0x%04x, off=0x%x,\tsize=0x%x" % (tag_type, off, \
			tag_size_bytes)

def list_tag002b_symbol(lm_data):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack(">HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0x002b:
			symbol_idx = struct.unpack(">H", data[0x4:0x6])[0]
			frame = struct.unpack(">H", data[0x6:0x8])[0]
			print "tag:0x%04x, off=0x%x,\tsize=0x%x,\t%s, frame=%x" % (tag_type, off, \
				tag_size_bytes, symbol_list[symbol_idx], frame)
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]	

def list_tag0027_symbol(lm_data, fname=""):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	
	ret = []
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0x0027, )):
		res = tag_reader.read_tag(format.DATA[0x0027], tag)

		characterID = res["character_id"]
		unk1, class_name_idx, frame_label_cnt = res["const0_0"], res["class_name_idx"], res["frame_label_cnt"]
		tag0001_cnt = res["0001_cnt"]
		key_frame_cnt = res["key_frame_cnt"]
		max_depth, unk2 = res["max_depth"], res["const1_0"]
		class_name = symbol_list[class_name_idx]
		ret.append((tag_type, off, tag_size_bytes, characterID, tag0001_cnt, frame_label_cnt, max_depth, class_name, key_frame_cnt, unk1, unk2))
		
#		assert unk1 == 0
#		assert text in range(15), fname
#		assert unk2 == 0
			
	return ret
	
def list_tagF022_symbol(lm_data, fname=""):
	bounding_box_list = list_tagF004_symbol(lm_data)
	ret = []
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF022, )):
		character_id, unk1, size_idx, f023_cnt, f024_cnt = read_tagF022(tag)
		box = bounding_box_list[size_idx]
		ret.append((tag_type, off, tag_size_bytes, character_id, f024_cnt, 
			f023_cnt, box[0], box[1], box[2], box[3]))
	return ret

def list_tagF024_img(lm_data):
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	ori_pic_list = list_tagF007_symbol(lm_data)	
	off = 0x40
	
	tex_size_list = list_tagF004_symbol(lm_data)
	
	x_min = y_min = 1000000000000
	x_max = y_max = -1000000000000
	size = (x_min, y_min, x_max, y_max)
					
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF022, 0xF023, 0xF024)):
		
		if tag_type == 0xF022:

			BOUND_ERR_MSG = "texture boundary not match boundary table!"
			assert x_min >= size[0], BOUND_ERR_MSG
			assert y_min >= size[1], BOUND_ERR_MSG
	   		assert x_max <= size[2], BOUND_ERR_MSG
			assert y_max <= size[3], BOUND_ERR_MSG

			x_min = y_min = 1000000000000
   			x_max = y_max = -1000000000000			

			res = read_tagF022(tag)
			id = res[0]
			size_idx = res[2]
			size = tex_size_list[size_idx]
			print "CharacterID=%d, size: (%d, %d, %d, %d)" % (id, size[0], size[1], size[2], size[3])
					
		elif tag_type == 0xF024:
			
			idx, flag, unk1, unk2 = struct.unpack(">IHHI", data[0x8:0x14])
			
			fv_list = []
			for off1 in xrange(0x14, 0x54, 0x4):
				fv = struct.unpack(">f", data[off1:off1+0x4])[0]
				fv_list.append(fv)	# (x, y, u, v)
				
			unk3, unk4, unk5, unk6 = struct.unpack(">IHHI", data[0x54: 0x60])

			x_min = min(x_min, fv_list[0], fv_list[4], fv_list[8], fv_list[12])
			x_max = max(x_max, fv_list[0], fv_list[4], fv_list[8], fv_list[12])
			y_min = min(y_min, fv_list[1], fv_list[5], fv_list[9], fv_list[13])
			y_max = max(y_max, fv_list[1], fv_list[5], fv_list[9], fv_list[13])
				
			if flag == 0x41:
			
				ori_pic_list = list_tagF007_symbol(lm_data)
#				print ori_pic_list, idx
				ori_pic_fname_idx = ori_pic_list[idx][1]
				ori_pic_tga_idx = ori_pic_list[idx][0]
				sb = symbol_list[ori_pic_fname_idx]

				print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,\tfill_img=%s" \
					% (tag_type, off, tag_size_bytes, sb)
				print "\t\t",fv_list[:4]
				print "\t\t",fv_list[4:8]
				print "\t\t",fv_list[8:12]
				print "\t\t",fv_list[12:]

			else:
				pass
				print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,fill_color_idx=0x%x" \
					% (tag_type, off, tag_size_bytes,idx)
				print "\t\t",fv_list[:4]
				print "\t\t",fv_list[4:8]
				print "\t\t",fv_list[8:12]
				print "\t\t",fv_list[12:]			
				
			print "unk = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x" % (unk1, unk2, unk3, unk4, unk5, unk6)
			
		if tag_type == 0xF023:
			
			d = tag_reader.read_tag(format.DATA[0xF023], tag)
			fv_list = [
						d["x0"], d["y0"], d["u0"], d["v0"],
						d["x1"], d["y1"], d["u1"], d["v1"],
						d["x2"], d["y2"], d["u2"], d["v2"],
						d["x3"], d["y3"], d["u3"], d["v3"],
						]
			flag, idx = d["fill_style"], d["fill_idx"]
			unk = d["const0_0"]

			x_min = min(x_min, fv_list[0], fv_list[4], fv_list[8], fv_list[12])
			x_max = max(x_max, fv_list[0], fv_list[4], fv_list[8], fv_list[12])
			y_min = min(y_min, fv_list[1], fv_list[5], fv_list[9], fv_list[13])
			y_max = max(y_max, fv_list[1], fv_list[5], fv_list[9], fv_list[13])
				
			if flag in (0x41, 0x40):
			
				ori_pic_fname_idx = ori_pic_list[idx][1]
				ori_pic_tga_idx = ori_pic_list[idx][0]
				sb = symbol_list[ori_pic_fname_idx]
				if not sb:
					sb = "[IMAGE%d]" % ori_pic_list[idx][0]

				print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,\tfill_img=%s" \
					% (tag_type, off, tag_size_bytes, sb)
				print "\t\t",fv_list[:4]
				print "\t\t",fv_list[4:8]
				print "\t\t",fv_list[8:12]
				print "\t\t",fv_list[12:]
				
			else:
				pass
				print "\ttag:0x%04x, off=0x%x,\tsize=0x%x,fill_color_idx=0x%x" \
					% (tag_type, off, tag_size_bytes, idx)
				print "\t\t",fv_list[:4]
				print "\t\t",fv_list[4:8]
				print "\t\t",fv_list[8:12]
				print "\t\t",fv_list[12:]			

		elif tag_type == 0x0027:
			break
		elif tag_type == 0xFF00:
			break
	
# DefineButton	
def list_tag0007_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF00D, 0x0007, )):
		d = tag_reader.read_tag(format.DATA[tag_type], tag)
		if tag_type == 0xF00D:
			tag0007_cnt = d["0007_cnt"]
		elif tag_type == 0x0007:
			tag0007_cnt -= 1
			print "%x, %x, %x" % (d["unk2"], d["unk3"], d["unk5"], )
			if tag0007_cnt == 0:
				break
def list_tag0001_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0x0001,)):
		d = tag_reader.read_tag(format.DATA[0x0001], tag)
		print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tframe=0x%x,\tsub_tag_cnt_cnt=%d" % \
			(tag_type, off, tag_size_bytes, d["frame_id"], d["cmd_cnt"])
	
def get_xref(lm_data):
	ref_table = {}
	data = lm_data[0x40:]
	symbol_list = get_symbol_list(data)
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack(">II", data[:0x8])
		tag_size_bytes = tag_size * 4 + 8
		if tag_type == 0x0004:
			id = struct.unpack(">I", data[0x8:0xc])[0]
			name_idx = struct.unpack(">i", data[0x14:0x18])[0]
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
	as_list = list_tagF005_symbol(lm_data)
	matrix_list = list_tagF003_symbol(lm_data)

	for off, tag_type, tag_size_bytes, data in iter_tag(lm_data):
		if tag_type == 0x0027:
			d = tag_reader.read_tag(format.DATA[0x0027], data)
			id = d["character_id"]
			max_depth = d["max_depth"]
			print "===================== offset=0x%x, CharacterID=%d, max_depth=%d %s" % (off, id, max_depth, id in ref_table and "Ref As %s" % (",".join(list(ref_table[id]))) or "")
		elif tag_type == 0x0001:
			d = tag_reader.read_tag(format.DATA[0x0001], data)
			print "Frame %d, cmd_cnt=%d" % (d["frame_id"], d["cmd_cnt"])
		elif tag_type == 0xf014:
			d = tag_reader.read_tag(format.DATA[0xF014], data)		
			print ">>>>>>>>>Do ClipAction: %d" % d["as_idx"]
		elif tag_type == 0xf105:
			d = tag_reader.read_tag(format.DATA[0xF105], data)
			print ">>>>>>>>>KeyFrame: v=%d" % d["frame_id"]
		elif tag_type == 0x000c:
			print ">>>>>>>>>Do Action %d" % struct.unpack(">I", data[0x8:0xc])
		elif tag_type == 0x0005:
			d = tag_reader.read_tag(format.DATA[0x0005], data)
			print ">>>>>>>>>RemoveObject at depth%d" % d["depth"]
			assert d["unk1"] == 0
			assert d["unk0"] == 0
			assert d["depth"] < max_depth
		elif tag_type == 0x002b:
			d = tag_reader.read_tag(format.DATA[0x002B], data)
			print ">>>>>>>>>FrameLabel: %s@%d" % (symbol_list[d["name_idx"]], d["frame_id"])
			assert d["unk0"] == 0
		if tag_type == 0x0004:
			res = tag_reader.read_tag(format.DATA[0x0004], data)
			
			character_id = res["character_id"]
			inst_id = res["inst_id"]
			unk1, name_idx = res["unk1"], res["name_idx"]
			flags, blend_mode, = res["flags"], res["blend_mode"]
			depth, unk3, ratio, unk5 = res["depth"], res["unk3"], res["ratio"], res["unk5"]
			trans_idx, color_mul_idx ,color_add_idx, unk6, clip_action_cnt = res["trans_idx"], res["color_mul_idx"], res["color_add_idx"], res["unk6"], res["clip_action_cnt"]

			blend_mode_name = blend_mode_2_name[blend_mode]
			if trans_idx == 0xFFFFFFFF:
				translate = scale = rotateskew = "null"
			elif (trans_idx & 0x80000000) == 0:
				translate = "(%.1f, %.1f)" % (matrix_list[trans_idx][4],
					matrix_list[trans_idx][5])
				scale = "(%.1f, %.1f)" % (matrix_list[trans_idx][0],
					matrix_list[trans_idx][3])
				rotateskew = "(%.1f, %.1f)" % (
					matrix_list[trans_idx][1], 
					matrix_list[trans_idx][2])
			else:
				translate = "(%.1f, %.1f)" % xy_list[trans_idx & 0xFFFFFFF]
				scale = rotateskew = ""
			if name_idx >= 0:
				name = symbol_list[name_idx]
			else:
				name = ""

			flags_str = ""
			flags_str += (flags & 0x1) and "N" or "-"
			flags_str += (flags & 0x2) and "M" or "-"
			if flags & (~0x3) != 0:
				assert False, "==============#new flags ! 0x%x " % flags

			if color_mul_idx < 0:
				color_mul_str = ""
			else:
				color_mul = color_list[color_mul_idx]
				color_mul_str = "(%.1f,%.1f,%.1f,%.1f)" % tuple([c/256.0 for c in color_mul])
			if color_add_idx < 0:
				color_add_str = ""
			else:
				color_add = color_list[color_add_idx]
				color_add_str = "(%d,%d,%d,%d)" % tuple(color_add)

			print "PlaceObject, off=0x%x,\tsize=0x%x" % (off, tag_size_bytes)
			print ("\tID=%d,\tdepth=%d,\tpos=%s,\tscale=%s,\tskew=%s,\tInstID=%d," \
				+"\tflags=%s,\tcolMul=%s,\tcolAdd=%s,\tclipAction=%d,\tname=%s,\tratio=%d\tblend_mode=%s") % \
				(character_id, depth, translate, scale, rotateskew, inst_id,
					flags_str, color_mul_str, color_add_str, clip_action_cnt, name, ratio, blend_mode_name)

			assert unk1 == 0 and unk3 == 0 and unk5 == 0 and unk6 == 0
			
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
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF103,)):
		res = tag_reader.read_tag(format.DATA[0xF103], tag)
		pos_list = [(pos["x"], pos["y"]) for pos in res["pos_list"]]
		return pos_list
	assert False, "Missing tag F103"

def list_tagF004_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF004,)):
		res = tag_reader.read_tag(format.DATA[0xF004], tag)
		box_list = [(box["xmin"], box["ymin"], box["xmax"], box["ymax"]) for box in res["box_list"]]
		return box_list
	assert False, "Missing tag F004"
		
def list_tagF00C_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF00C,)):
		res = tag_reader.read_tag(format.DATA[0xF00C], tag)

		assert res["v"] == 0 and res["e"] == 1 and res["r"] == 2
		assert res["max_character_id"] == res["start_character_id"]
		assert res["reserved"] == -1
		assert res["reserved2"] == 0
		
		return res
	assert False, "Missing tag F00C"
			
def list_tagF008_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF008,)):
		res = tag_reader.read_tag(format.DATA[0xF008], tag)
		return
	assert False, "Missing tag F008"

def list_tagF009_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF009,)):
		res = tag_reader.read_tag(format.DATA[0xF009], tag)
		assert res["unk"] == 0
		return		
	assert False, "Missing tag F009"
	
def list_tagF00A_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF00A,)):
		res = tag_reader.read_tag(format.DATA[0xF00A], tag)
		return		
	assert False, "Missing tag F00A"

def list_tag000A_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0x000A,)):
		res = tag_reader.read_tag(format.DATA[0x000A], tag)
		return		
	assert False, "Missing tag 000A"

def list_tag000C_symbol(lm_data):
	as_list = list_tagF005_symbol(lm_data)
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0x000C,)):
		res = tag_reader.read_tag(format.DATA[0x000C], tag)
#		assert res["unk1"] == 0, "%d" % res["unk1"]
		assert res["unk0"] < len(as_list)
		
def list_tagF00B_symbol(lm_data):

	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF00B,)):
		res = tag_reader.read_tag(format.DATA[0xF00B], tag)
		assert res["unk"] == 1
		return		
	assert False, "Missing tag F00B"
				
def list_tagF007_symbol(lm_data, prefix_for_noname=""):
	symbol_list = get_symbol_list(lm_data[0x40:])
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF007,)):
		res = tag_reader.read_tag(format.DATA[0xF007], tag)
		image_list = []
		for image_info in res["img_list"]:
			if symbol_list[image_info["name_idx"]] == "":
				fname = "noname_%s_0x%x.png" % (prefix_for_noname, 
					image_info["img_idx"])
			else:
				fname = symbol_list[image_info["name_idx"]]
			image_list.append((image_info["img_idx"], image_info["name_idx"], image_info["width"], image_info["height"], fname))
		return image_list
	assert False, "Missing tag F007"
						
def list_tagF005_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF005,)):
		res = tag_reader.read_tag(format.DATA[0xF005], tag)
		as_list = []
		off = 0xc
		idx = 0
		for abc in res["as_list"]:
			as_list.append((off+0x4, abc["as_len"]))
#			print "%x\toff=0x%x len=0x%x" % ((idx, ) + as_list[-1])
			abc["padding"] = abc["padding"] or ""
			off += 0x4 + abc["as_len"] + len(abc["padding"])
			idx += 1

		as_list = [(abc["as_len"], abc["bytecode"]) for abc in res["as_list"]]
		return as_list
	assert False, "Missing tag F005"

def list_tagF002_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF002,)):
		res = tag_reader.read_tag(format.DATA[0xF002], tag)
		color_list = [(color["R"], color["G"], color["B"], color["A"]) for color in res["color_list"]]
		return color_list
	assert False, "Missing tag F002"
			
def list_tagF003_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF003,)):
		res = tag_reader.read_tag(format.DATA[0xF003], tag)
		mat_list = [(mat["trans_x"], mat["trans_y"], mat["rotateskew_x"], mat["rotateskew_y"], mat["scale_x"], mat["scale_y"]) for mat in res["mat_list"]]
		return mat_list
	assert False, "Missing tag F003"
					
def list_tagF00D_symbol(lm_data):
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF00D,)):
		res = tag_reader.read_tag(format.DATA[0xF00D], tag)
#		assert res["unk6"] in (0, ), "%d" % res["unk6"]
		
		return res
		
def shuffle_tagF022(lm_data):
	head = ""
	tail = ""
	tagF022_list = []
	tagF022 = ""
	
	head += lm_data[:0x40]
	data = lm_data[0x40:]
	while True:
		tag_type, tag_size = struct.unpack(">HH", data[:0x4])
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
	symbol_list = get_symbol_list(data)
	
	box_list = list_tagF004_symbol(lm_data)
	
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, (0xF00D, 0x0025)):
		d = tag_reader.read_tag(format.DATA[tag_type], tag)
		if tag_type == 0xF00D:
			tag0025_cnt = d["0025_cnt"]
		elif tag_type == 0x0025:
			tag0025_cnt -= 1
			print "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x" % (d["unk1"], d["unk3"], d["unk7"], d["unk8"], d["unk9"], d["unk10"])
			
			# A neat print for all known fields of tag 0x0025
			flags = (d["unk9"] << 32) + d["unk10"]
			fHTML = flags & 0x1
			
			
			box = box_list[d["rect_idx"]]
			var_name = symbol_list[d["var_name_idx"]]
			init_text = symbol_list[d["init_txt_idx"]]
			print "ID=%d, rect:(%.2f, %.2f, %.2f, %.2f), font_size=%.2f, spacing=(%.2f, %.2f, %.2f, %.2f), var=%s, init_text=%s," % ((d["character_id"],) + tuple(box) + (d["font_size"], d["left_margin"], d["right_margin"], d["indent"], d["leading"], var_name, init_text))
			
		if tag0025_cnt == 0:
			break
	
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
		elif options.tag_id == 0x0027 or options.tag_id == 0xF022:
			res = list_tagF022_symbol(data)
			for arg in res:
				print ("tag:0x%04x off=0x%x,\tsize=0x%x,\tCharacterID=%d," + \
				"\tf024_cnt=%d,f023_cnt=%d\n\tbox=(%.2f,%.2f,%.2f,%.2f)\n") \
				 % arg
			print "==============="
			res = list_tag0027_symbol(data)
			for arg in res:
				print "tag:0x%04x, off=0x%x,\tsize=0x%x,\tCharacterID=%d\tframe=%d,\tlabel=%d,\tmaxdepth=0x%x,class_name=%s,key_frame_cnt=%d,unk=%d, %d" % arg
		elif options.tag_id == 0xF023:
#			for off in range(0, 0x48, 2):
#				print "off %x~%x" % (off, off+2)
#				list_tagF023_symbol(data, off, off+2)
			list_tagF023_img(data)

		elif options.tag_id == 0x0001:
			list_tag0001_symbol(data)
		elif options.tag_id == 0x0004:
			list_tag0004_symbol(data)
		elif options.tag_id == 0x0007:
			list_tag0007_symbol(data)
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
			for i, symbol in enumerate(symbol_list):
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
			res = list_tagF004_symbol(data)
			print "Bounding Box Info:"
			for i, v in enumerate(res):
				print "0x%x\t" % i, v	
		elif options.tag_id == 0xF005:
			res = list_tagF005_symbol(data)
			print "Actionscript:"
			for i, (as_len, bytecode) in enumerate(res):
				print "%x:\tsize=0x%x" % (i, as_len)
		elif options.tag_id == 0xF007:
			v_list = list_tagF007_symbol(data)
			print "img file info:"
			for v in v_list:
				print "\timg_idx=%d, width=%.2f, height=%.2f, fname=%s" % \
					(v[0], v[2], v[3], v[4].decode("utf8"))
		elif options.tag_id == 0xF008:
			list_tagF008_symbol(data)
		elif options.tag_id == 0xF009:
			list_tagF009_symbol(data)			
		elif options.tag_id == 0xF00A:
			list_tagF00A_symbol(data)				
		elif options.tag_id == 0xF00B:
			list_tagF00B_symbol(data)					
		elif options.tag_id == 0xF00C:

			res = list_tagF00C_symbol(data)
			
			print "Stage info:"
			print "ver = %d%d%d" % (res["v"], res["e"], res["r"])
			print "max/start character id = %d" % res["max_character_id"]
			print "stage size (%.2f, %.2f), pos=(%.2f, %.2f), fps = %.2f" % (res["width"], res["height"], res["x"], res["y"], res["fps"])
			print "UNKNOWN unk = %d" % res["unk"]
			print "reserved = %d, reserved2 = %d" % (res["reserved"], res["reserved2"])
			
		elif options.tag_id == 0xF00D:
			res = list_tagF00D_symbol(data)
			print "\tF022:%d, 0027:%d" % (res["f022_cnt"], res["0027_cnt"])			
		elif options.tag_id == 0x0025:
			res = list_tag0025_symbol(data)
		elif options.tag_id == 0xF024:
			res = list_tagF024_img(data)