import os
import sys
import struct
import swf_helper
import optparse

# the 'LM ripper' flag
# for different platform, use different ripper module
PLATFORM_WII = 0
PLATFORM_PSPDX = 1

prefix_for_noname = ""

# get an image_dict
# {filename: data}
def get_image_dict(lm_data, image_root):
	global prefix_for_noname
	
	image_dict = {}
	symbol_list = rip_gim.get_symbol_list(lm_data[0x40:])
	ori_pic_list = rip_gim.list_tagF007_symbol(lm_data, prefix_for_noname)
	
	for ori_pic_info in ori_pic_list:
	
		# get the right image file name
		sb = ori_pic_info[-1]

		# remove blend mode suffix in filename
		# e.g: xyz.png__BLEND_ADD__ ---> xyz.png		
		idx = sb.rfind(".png")
		sb = sb[:idx+len(".png")]
		image_file = sb[:-4] + ".png"
		
		# store image data in a dict
		f = open(os.path.join(image_root, image_file), "rb")
		image_data = f.read()
		f.close()
		image_dict[ori_pic_info[-1]] = image_data
		
	return image_dict
	
# get a shape info dict
def get_shape_dict(lm_data):
	global prefix_for_noname
	
	data = lm_data[0x40:]
	color_list = rip_gim.list_tagF002_symbol(lm_data)
	symbol_list = rip_gim.get_symbol_list(lm_data[0x40:])
	ori_pic_list = rip_gim.list_tagF007_symbol(lm_data, prefix_for_noname)
	off = 0x40
	ret = {}
	ret2 = {} # image_name_2_fill_type
	
	for off, tag_type, tag_size_bytes, tag in iter_tag(lm_data, \
		(0xF022, 0xF023, 0xF024)):
		
		if tag_type == 0xF022:
			sprite_id, unk1, size_idx, f023_cnt, f024_cnt = \
				rip_gim.read_tagF022(tag)
		elif tag_type == 0xF023:
			pass
		elif tag_type == 0xF024:
			pass
			
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF022:
			sprite_id, = struct.unpack("<I", data[0x4:0x8])
			img_cnt, = struct.unpack("<H", data[0xa:0xc])
			for i in xrange(img_cnt):
				data = rip_gim.seek_next_tag(data)
				img_fname_idx, flag = struct.unpack("<HH", data[0x44:0x48])
				
				# calc shape size
				all_floats = struct.unpack("<"+"f"*16, 
					data[0x4:0x4+0x4*16])
				xs = all_floats[::4]
				ys = all_floats[1::4]
				xmin, xmax = min(xs), max(xs)
				ymin, ymax = min(ys), max(ys)
				width = xmax - xmin
				height = ymax - ymin
				
				# fill return values
				if flag == 0x00:
				
					ret[(sprite_id << 16)+i] = (color_list[img_fname_idx], 
						(int(width), int(height)))
				else:
					
					ret2[symbol_list[ori_pic_list[img_fname_idx][1]]] = (flag, int(width), int(height))

			continue
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	return ret, ret2
	
def get_texture_sprite_tags(lm_data, image_2_id, shape_2_id, image_dict):
	global prefix_for_noname
	
	data = lm_data[0x40:]
	symbol_list = rip_gim.get_symbol_list(data)
	img_fname_list = [symbol for symbol in symbol_list \
		if symbol.endswith(".png")]
	define_sprite_tags = []
	ori_pic_list = rip_gim.list_tagF007_symbol(lm_data, prefix_for_noname)
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		if tag_type == 0xFF00:
			break
		if tag_type == 0xF022:
			sprite_id, = struct.unpack("<I", data[0x4:0x8])
			img_cnt, = struct.unpack("<H", data[0xa:0xc])
			place_object2_tags = []
			for i in xrange(img_cnt):	# handle each F023 tag
				data = rip_gim.seek_next_tag(data)
				img_fname_idx, flag = struct.unpack("<HH", data[0x44:0x48])
				
				# calculate upper left corner of the shape
				# in current texture sprite
				all_floats = struct.unpack("<"+"f"*16, 
					data[0x4:0x4+0x4*16])
				xs = all_floats[::4]
				ys = all_floats[1::4]
				xmin, xmax = min(xs), max(xs)
				ymin, ymax = min(ys), max(ys)
				
				if flag == 0x00:	# solid fill
					matrix = swf_helper.pack_matrix(None, None, (xmin, ymin))
					shape_id = shape_2_id[(sprite_id<<16) + i]
				else:	# clipped bitmap fill or repeating bitmap fill
					assert flag in (0x40, 0x41), "not supported (fill type) atm! %d" % flag
					img_fname = symbol_list[ori_pic_list[img_fname_idx][1]]
					shape_id = image_2_id[img_fname]
					matrix = swf_helper.pack_matrix(None, None, (xmin, ymin))
						
				place_object2_tag = swf_helper.make_place_object2_tag(swf_helper.PLACE_FLAG_HAS_CHARACTER | swf_helper.PLACE_FLAG_HAS_MATRIX, i+1, id=shape_id, matrix=matrix)
				place_object2_tags.append(place_object2_tag)
			show_frame_tag = swf_helper.make_show_frame_tag()
			
			control_tags = []
			control_tags.extend(place_object2_tags)
			control_tags.append(show_frame_tag)
			control_tags.append(swf_helper.make_end_tag())
			define_sprite_tag = swf_helper.make_define_sprite_tag(sprite_id, 1, control_tags)
			define_sprite_tags.append(define_sprite_tag)
		else:
			data = rip_gim.seek_next_tag(data)
	return define_sprite_tags
	
def get_define_sprite_tags(lm_data, action_constant_pool, action_record_list):
	# some tables for referrence
	color_list = rip_gim.list_tagF002_symbol(lm_data)
	point_list = rip_gim.list_tagF103_symbol(lm_data)
	matrix_list = rip_gim.list_tagF003_symbol(lm_data)
	symbol_list = rip_gim.get_symbol_list(lm_data[0x40:])
	define_sprite_tags = []
	
	data = lm_data[0x40:]
	data = rip_gim.seek_next_tag(data, (0x0027,))
	while True:
#		print len(data[:0x4])
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		if tag_type == 0xFF00:
			break
		if tag_type == 0x0027:

			control_tags = []
			sprite_id, = struct.unpack("<H", data[0x4:0x6])
			frame_count, = struct.unpack("<H", data[0xc:0xe])
#			print "frame_count %d" % frame_count
			
			frame_label_cnt, = struct.unpack("<H", data[0xa:0xc])
#			print "frame lable cnt %d" % frame_label_cnt
			frame_label_dict = {}
			for i in xrange(frame_label_cnt):
				data = rip_gim.seek_next_tag(data, (0x002b,))
				frame_label_idx, the_frame = struct.unpack("<HH", 
					data[0x4:0x8])
				frame_label = symbol_list[frame_label_idx]
				frame_label_dict[the_frame] = frame_label
				
			depth2matrix = {}
			depth2color_trans = {}
			
			
			for i in xrange(frame_count):
				data = rip_gim.seek_next_tag(data, (0x0001,))
				ptag_cnt, = struct.unpack("<H", data[0x6:0x8])
#				print "frame %d, placeobject%d" % (i, ptag_cnt)
				if i in frame_label_dict:
					control_tags.append(swf_helper.make_frame_label_tag(
						frame_label_dict[i]))
				for j in xrange(ptag_cnt):
					data = rip_gim.seek_next_tag(data, (0x0004, 0x0005, 0x000c))
					_type = struct.unpack("<H", data[:0x2])[0]
					if _type == 0x0005:
						depth, = struct.unpack("<H", data[0x6:0x8])
						control_tags.append(
							swf_helper.make_remove_object2_tag(depth+1))
						continue
					if _type == 0x000c:
						as_idx, = struct.unpack("<H", data[0x4:0x6])
						control_tags.append(
							swf_helper.make_do_action_tag([action_constant_pool
							, action_record_list[as_idx]]))
						continue
					elif _type != 0x0004:
						print "Ignore other tags ATM"
						continue
					
					_flags = struct.unpack("<H", data[0xc:0xe])[0]
					flags = 0
					if _flags & 1:
						flags |= swf_helper.PLACE_FLAG_HAS_CHARACTER
					if _flags & 2:
						flags |= swf_helper.PLACE_FLAG_MOVE
					id, = struct.unpack("<H", data[0x4:0x6])
					trans_idx = struct.unpack("<H", data[0x18:0x1a])[0]
					if trans_idx == 0xFFFF:
						pass
					elif (trans_idx & 0x8000) == 0:
						translate = (matrix_list[trans_idx][4],
							matrix_list[trans_idx][5])
						scale = (matrix_list[trans_idx][0],
							matrix_list[trans_idx][3])
						rotateskew = (matrix_list[trans_idx][1], 
							matrix_list[trans_idx][2])
						flags |= swf_helper.PLACE_FLAG_HAS_MATRIX
					else:
						trans_idx &= 0x7FFF
						translate = point_list[trans_idx]
						scale = rotateskew = None
						flags |= swf_helper.PLACE_FLAG_HAS_MATRIX
					if flags & swf_helper.PLACE_FLAG_HAS_MATRIX:
						matrix = swf_helper.pack_matrix(scale, rotateskew, 
							translate)
					else:
						matrix = None
					depth = struct.unpack("<H", data[0x10:0x12])[0] + 1
					name_idx = struct.unpack("<H", data[0xa:0xc])[0]
					name = symbol_list[name_idx]
					if name != "":
						flags |= swf_helper.PLACE_FLAG_HAS_NAME
					color_mul_idx = struct.unpack("<h", data[0x1a:0x1c])[0]
					color_add_idx = struct.unpack("<h", data[0x1c:0x1e])[0]
#					color_mul_idx = -1
#					color_add_idx = -1
					if color_mul_idx >= 0 or color_add_idx >= 0:
						flags |= swf_helper.PLACE_FLAG_HAS_COLOR_TRANSFORM
					if color_mul_idx < 0:
						color_mul = None
					else:
						color_mul = [c/256.0 for c in color_list[color_mul_idx]]
					if color_add_idx < 0:
						color_add = None
					else:
						color_add = color_list[color_add_idx]
					if flags & swf_helper.PLACE_FLAG_HAS_COLOR_TRANSFORM:
						color_trans = \
							swf_helper.pack_color_transform_with_alpha(
								color_add, color_mul)
					else:
						color_trans = None
						
					clip_depth, = struct.unpack("<H", data[0x12:0x14])
					if clip_depth > 0:
						flags |= swf_helper.PLACE_FLAG_HAS_CLIP_DEPTH
						
					ratio, = struct.unpack("<h", data[0x6:0x8])
					if ratio >= 0:
						flags |= swf_helper.PLACE_FLAG_HAS_RATIO

					if flags & swf_helper.PLACE_FLAG_HAS_CHARACTER and \
						flags & swf_helper.PLACE_FLAG_MOVE:
						control_tags.append(
							swf_helper.make_remove_object2_tag(depth))
						flags &= (0xFFFF - swf_helper.PLACE_FLAG_MOVE)
						if not (flags & swf_helper.PLACE_FLAG_HAS_MATRIX):
							flags |= swf_helper.PLACE_FLAG_HAS_MATRIX
							matrix = depth2matrix[depth]
						
					clip_action_cnt, = struct.unpack("<H", data[0x20:0x22])
					if clip_action_cnt > 0:
						clip_action_records = []
						for k in range(clip_action_cnt):
							data = rip_gim.seek_next_tag(data, (0xf014,))
							as_idx, event_flags, keycode = struct.unpack("<HIB", data[0x4:0xb])
							clip_action_records.append(swf_helper.pack_clip_action_record(event_flags, [action_constant_pool
							, action_record_list[as_idx]], keycode))
							
						flags |= swf_helper.PLACE_FLAG_HAS_CLIP_ACTIONS
						clip_actions = \
							swf_helper.pack_clip_actions(clip_action_records)
					else:
						clip_actions = None
							
					ptag = swf_helper.make_place_object2_tag(flags, depth, id, 
						name=name, matrix=matrix, color_trans=color_trans, clip_actions=clip_actions, ratio=ratio, clip_depth=clip_depth)
					control_tags.append(ptag)
					
					if matrix:
						depth2matrix[depth] = matrix
					if color_trans:
						depth2color_trans[depth] = color_trans
					
				show_frame_tag = swf_helper.make_show_frame_tag()
				control_tags.append(show_frame_tag)
			# append end tag
			control_tags.append(swf_helper.make_end_tag())
			# build define sprite tag
			define_sprite_tag = swf_helper.make_define_sprite_tag(sprite_id, frame_count, control_tags)
			define_sprite_tags.append(define_sprite_tag)
		else:		 
			data = rip_gim.seek_next_tag(data)
			
	return define_sprite_tags
		
def fix_action_record(data, symbol_list):
	ret = []
	while data:
		action_code, = struct.unpack("<B", data[:0x1])
		if action_code == 0x0:
			data = data[0x1:]
#			print "empty record trimmed"
			continue
		if action_code < 0x80:
			ret.append(data[0])
			data = data[0x1:]
		else:
			length, = struct.unpack("<H", data[0x1:0x3])
			record = data[:length + 0x3]
			
			# trim some record
			if action_code == 0x9B:
			
				func_name_idx, num_param = struct.unpack("<HH", 
					record[0x3:0x7])
				func_name = symbol_list[func_name_idx]
				func_name_len = len(func_name)

				param_names = []
				param_name_idxs = struct.unpack("<"+"H"*num_param, 
					record[0x7:0x7+0x2*num_param])
				for param_name_idx in param_name_idxs:
					param_names.append(symbol_list[param_name_idx])
					
				code_size, = struct.unpack("<H", record[-2:])
				sub = fix_action_record(data[length+0x3:length+0x3+code_size], 
					symbol_list)
					
				# build new record
				fixed_record = ""
				fixed_record += record[:0x3]
				fixed_record += swf_helper.pack_string(func_name)
				fixed_record += swf_helper.pack_uhalf(num_param)
				for param_name in param_names:
					fixed_record += swf_helper.pack_string(param_name)
				fixed_record += swf_helper.pack_uhalf(len(sub))
				fixed_record = fixed_record[:0x1] + \
					swf_helper.pack_uhalf(len(fixed_record)-0x3) + \
					fixed_record[0x3:]
				
				# fix code_size
				ret.append(fixed_record+sub)
				data = data[length + 0x3 + code_size:]
#				print "fixed_code_size %x %x" % (code_size, len(sub))
			elif action_code == 0x9D:
				branch_off, = struct.unpack("<h", record[-2:])
				
				if branch_off < 0:
					raise Exception("Do not support negative branch offset!")
					
				# -- fix if sub block
				sub_block = data[length + 0x3: length + 0x3 + branch_off]
				fixed_sub_block = fix_action_record(sub_block, symbol_list)
				fixed_branch_off = len(fixed_sub_block)
				
				# -- rebuild action record
				fixed_record = ""
				fixed_record += record[:0x3] + \
					struct.pack("<h", fixed_branch_off)
					
				ret.append(fixed_record)
				ret.append(fixed_sub_block)
				
				data = data[0x3 + length + branch_off:]

			elif action_code == 0x96:   # ActionPush
				fixed_record = ""
				
				raw_items = record[0x3:]
				fixed_record = record[:0x3]
				while raw_items != "":
					push_type, = struct.unpack("<B", raw_items[0x0:0x1])
					if push_type in (0x4, 0x5, 0x8):
						bytes = raw_items[0x1:0x2]
						off = 0x1
					elif push_type in (0x9,):
						bytes = raw_items[0x1:0x3]
						off = 0x2
					elif push_type in (0x1, 0x7):
						bytes = raw_items[0x1:0x5]
						off = 0x4
						
#						if push_type == 0x1:
#							print "push float %f" % struct.unpack("<f", bytes)
#						else:
#							print "push integer %f" % struct.unpack("<i", bytes)
							
					elif push_type in (0x6,):   # swap for double type
						bytes = raw_items[0x5:0x9] + raw_items[0x1:0x5]
						off = 0x8
					elif push_type == 0x0:  # look up raw string in symbol_list
						str_idx, = struct.unpack("<H", raw_items[0x1:0x3])
#						print "str_idx: %d" % str_idx
						_str = symbol_list[str_idx]
						bytes = swf_helper.pack_string(_str)

						off = 0x2
					elif push_type == 0x2:
						bytes = ""
						off = 0x0
					else:
						assert False, "not supported push type %x" % push_type
					fixed_record += raw_items[0x0:0x1] + bytes
					raw_items = raw_items[off + 1:]
				
#				assert len(fixed_record) == len(record)
				# fix the action push record size
				fixed_record = fixed_record[0x0:0x1] + struct.pack("<H", 
					len(fixed_record)-0x3) + fixed_record[0x3:]
				ret.append(fixed_record)
				data = data[length + 0x3:]
#TODO: fix branch_offset which points to a previous block that may be trimmed!
			elif action_code == 0x9D:
				branch_offset, = struct.unpack("<h", record[0x3:0x5])
				assert branch_offset >= 0, "not support negative branch offset"
				
				branch_false = record[0x5:0x5+branch_offset]
				fixed_branch_false = fix_action_record(branch_false)
				
				record = record[0x0:0x1] + \
					struct.pack("<h", len(fixed_branch_false))
				
				ret.append(record + branch_false)
				data = data[length + 0x3 + branch_offset:]				
			elif action_code == 0x8E:
				code_size, = struct.unpack("<H", record[-2:])
				code = data[0x3 + length: 0x3 + length + code_size]
				
				# fix function body
				fixed_code = fix_action_record(code, symbol_list)
				
				# replace string idx with string
				func_name_idx, = struct.unpack("<H", data[0x3:0x5])
				func_name = symbol_list[func_name_idx]
				func_name_len = len(func_name)
				
				# -- fix register params
				register_params = []
				num_params, = struct.unpack("<H", record[0x5:0x7])
				for i in xrange(num_params):
					register, param_name_idx = struct.unpack("<BH", 
						record[0xa+i*0x3: 0xa+i*0x3+0x3])
					param_name = symbol_list[param_name_idx]
					register_params.append((register, param_name))
				
				# -- rebuild record
				fixed_record = ""
				# action record header
				fixed_record += record[:0x3]
				# function name
				fixed_record += swf_helper.pack_string(func_name)
				fixed_record += record[0x5:0xa]
				# pack register params
				for register, param_name in register_params:
					fixed_record += swf_helper.pack_ubyte(register)
					fixed_record += swf_helper.pack_string(param_name)
				fixed_record += struct.pack("<H", len(fixed_code))
				# fix the whole record size
				fixed_record_length = len(fixed_record) - 0x3
				fixed_record = fixed_record[:0x1] + \
					swf_helper.pack_uhalf(fixed_record_length) + \
					fixed_record[0x3:]
				
				ret.append(fixed_record + fixed_code)
				data = data[length + 0x3 + code_size:]
				
			elif action_code == 0x8c:
				label_idx, = struct.unpack("<H", record[0x3:])
				label = symbol_list[label_idx]
				record = struct.pack("<BH", 0x8c, len(label) + 1) + \
					swf_helper.pack_string(label)
				ret.append(record)
				data = data[length + 0x3:]
	
			# default handler
			elif action_code in (0x87, 0x9F, 0x83, 0x81, 0x99):
				ret.append(record)
				data = data[length + 0x3:]
			else:
				assert False, "New Action Code = 0x%x" % action_code
			
			# align to 4 bytes
#			len_mod = (length+0x3) % 4
#			if len_mod:
#				data = data[4 - len_mod:]
#	print "===="
	return "".join(ret)
	
def test(fname, ID, label, pos, scale, fout, img_path, norecreate):
	global prefix_for_noname
	prefix_for_noname = os.path.splitext(os.path.split(fname)[1])[0]
	
	image_root = img_path or r"c:\png"
#	fname = "CHIBI_1P_BALLOON_01.LM"
	f = open(fname, "rb")
	lm_data = f.read()
	f.close()
	
	fout = fout or fname[:-3] + ".swf"
	if norecreate and os.path.exists(fout):
		return
	
	# init
	symbol_table = rip_gim.get_symbol_list(lm_data[0x40:])
	assert symbol_table[0] == ""
	constant_pool = "".join([str+"\x00" for str in symbol_table])
	action_constant_pool = struct.pack("<BHH", 0x88, 2+len(constant_pool), 
		len(symbol_table)) + constant_pool
	action_record_list = rip_gim.list_tagF005_symbol(lm_data)
	frame_label_dict = rip_gim.get_frame_label_dict(lm_data)
#	action_record_list = map(fix_action_record, action_record_list)
#	print len(action_record_list)
#	for i in ():
	for i in xrange(len(action_record_list)):
		print "fixing action record %d" % i
		action_record_list[i] = fix_action_record(action_record_list[i], symbol_table)
#	fix_action_record(action_record_list[2])
	
	max_characterID = rip_gim.get_max_characterID(lm_data)
	
	# image_dict: {filename : image_data}
	# image_dict2: {filename: (fill_style_type, shape_width, shape_height)}

	image_dict = get_image_dict(lm_data, image_root)
	shape_dict, image_dict2 = get_shape_dict(lm_data)
	# all tags append to this list
	all_tags = []
	
	# make FileAttributes tag
	all_tags.append(swf_helper.make_file_attributes_tag())
	
	# make SetBackgroundColor tag
	all_tags.append(swf_helper.make_set_background_color_tag(0xFF, 0xFF, 0xFF))
	
	# make all DefineBitsJPEG2 tags
	define_bits_JPEG2_tags = []
	image_2_id = {}
	id = max_characterID + 1
	for k, v in image_dict.iteritems():
		tag = swf_helper.make_define_bits_JPEG2_tag(id, v)
		define_bits_JPEG2_tags.append(tag)
		image_2_id[k] = id
		id += 1
	all_tags.extend(define_bits_JPEG2_tags)
	
	# make all DefineShape tags
	define_shape_tags = []
	image_2_shape_id = {}
	shape_2_shape_id = {}
	id = max_characterID + len(image_dict) + 1
	for k, v in image_dict.iteritems():
		img_data = image_dict[k]
		
		fill_style_type, shape_width, shape_height = image_dict2[k]
		tag = swf_helper.make_define_shape3_tag_bitmap_simple(id, 
			image_2_id[k], shape_width, shape_height, fill_style_type)
		image_2_shape_id[k] = id
		id += 1
		define_shape_tags.append(tag)
		
	for k, (color, size) in shape_dict.iteritems():
		tag = swf_helper.make_define_shape3_tag_solid_simple(id, size[0],
			size[1], swf_helper.pack_color(color))
		shape_2_shape_id[k] = id
		id += 1
		define_shape_tags.append(tag)
		
	all_tags.extend(define_shape_tags)

	# make all texture mc tags
	define_sprite_tags = get_texture_sprite_tags(lm_data, image_2_shape_id, shape_2_shape_id, image_dict)
	all_tags.extend(define_sprite_tags)

	# make all general mc tags
	define_sprite_tags_general = get_define_sprite_tags(lm_data, action_constant_pool, action_record_list)
	all_tags.extend(define_sprite_tags_general)
	
	# test basic display
	tmp_tags = []
	
	# INSTANCE ID(ratio) should be enough!
	id = ID or max_characterID
	tmp_tags.append(swf_helper.make_place_object2_tag(swf_helper.PLACE_FLAG_HAS_CHARACTER|swf_helper.PLACE_FLAG_HAS_MATRIX|swf_helper.PLACE_FLAG_HAS_NAME|swf_helper.PLACE_FLAG_HAS_RATIO, 1, id=id, matrix=swf_helper.pack_matrix(scale and (scale, scale) or None, None, pos or (0, 0), ),name="main",ratio=0xFFFF))

	if label is not None:
		action_records = []
		action_records.append("\x8B\x05\x00main\x00")   # ActionSetTarget "main"
		frame_idx = frame_label_dict[id][label]
		action_records.append("\x81\x02\x00" + struct.pack("<H", frame_idx))
		action_records.append("\x06")
		action_records.append("\x8B\x01\x00")
		tmp_tags.append(swf_helper.make_do_action_tag(action_records))		
	
	tmp_tags.append(swf_helper.make_show_frame_tag())
	
	all_tags.extend(tmp_tags)
	
	# make end tag
	end_tag = swf_helper.make_end_tag()
	all_tags.append(end_tag)
	
	# build swf header
	all_data = ""
	for tag in all_tags:
		all_data += tag
		
	swf_header = swf_helper.make_swf_header(0xa, 0, 480, 272, 60.0, 1)
	file_length = len(swf_header) + len(all_data)
	swf_header = swf_helper.make_swf_header(0xa, file_length, 480, 272, 60.0, 
		1)
	
	fout = open(fout, "wb")
	fout.write(swf_header + all_data)
	fout.close()
	
if __name__ == "__main__":
	parser = optparse.OptionParser()
	parser.add_option("-f", dest="filename", help="LM file path")
	parser.add_option("-o", dest="fout", help="output file path")
	parser.add_option("-t", dest="texture_root", help="where your png files are.")
	parser.add_option("-l", dest="label", help="framelabel of the sprite.")
	parser.add_option("-i", type="int", action="store", dest="characterID", help="ID of the character to be placed on the stage.")
	parser.add_option("-p", type="float", nargs=2, dest="pos", help="postion of the sprite. example: -p 128 128")
	parser.add_option("-s", type="float", dest="scale", help="the scale of the sprite")
	parser.add_option("-d", action="store_true", dest="dry_run", help="show all character IDs and their frame labels.")
	parser.add_option("-I", action="store_true", dest="norecreate", help="Ignore a file when the corresponding swf is already exists!")
	parser.add_option("-P", action="store", type="int", dest="platform", default=0, help="specify platform: %d for wii, %d for pspdx, default:%d" % (PLATFORM_WII, PLATFORM_PSPDX, 0))

	(options, args) = parser.parse_args(sys.argv)
	
	sys.path.append("../CLM")
	
	if options.platform == PLATFORM_WII:
		import rip_gim_wii as rip_gim
	elif options.platform == PLATFORM_PSPDX:
		import rip_gim_pspdx as rip_gim
	else:
		print "Unsupported platform!"
		os.exit()
		
	if os.path.isdir(options.filename):
		def is_LM(filename):
			return filename.upper().endswith(".LM")
		def join_filename(filename):
			return os.path.join(options.filename, filename)
		filenames = map(join_filename, filter(is_LM, os.listdir(options.filename)))
	else:
		filenames = (options.filename,)
	
	for filename in filenames:
		print "Doing %s:" % filename
		if options.dry_run:
			f = open(filename, "rb")
			lm_data = f.read()
			f.close()
			ret = rip_gim.get_frame_label_dict(lm_data)
			
			for id, dic in sorted(ret.items()):
				print "labels of %d" % id
				print dic.keys()		
		else:
			test(filename, options.characterID, options.label, options.pos, options.scale, options.fout, options.texture_root, options.norecreate)
