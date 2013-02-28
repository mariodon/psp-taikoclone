import os
import struct
import rip_gim

def iter():
	ROOT = ".\LM"
	for filename in os.listdir(ROOT):
		if not filename.upper().endswith(".LM"):
			continue
		print filename
		yield open(os.path.join(ROOT, filename), "rb").read()
		
def iter_tag(lumen, type_set=None):
	lumen = lumen[0x40:]
	
	_type_set = type_set or ()
		
	while lumen:
		tag_type, = struct.unpack("<H", lumen[:0x2])
		if tag_type == 0xFF00:
			break
		if not _type_set or tag_type in _type_set:
			yield lumen
		lumen = rip_gim.seek_next_tag(lumen)
	
def check_F00C():
	the_ver0 = None
	the_ver1 = None
	the_ver2 = None
	the_maxFrame = None
	the_minDepth = None
	the_centerX = None
	the_centerY = None
	for lumen in iter():
		ver0, ver1, ver2, maxCID, maxFrame, initCID, maxDepth, minDepth, fps, width, height, centerX, centerY = rip_gim.list_tagF00C_symbol(lumen)
		
		if the_ver0 is None: the_ver0 = ver0
		if the_ver1 is None: the_ver1 = ver1
		if the_ver2 is None: the_ver2 = ver2
		if the_maxFrame is None: the_maxFrame = maxFrame
		if the_minDepth is None: the_minDepth = minDepth
		if the_centerX is None: the_centerX = centerX
		if the_centerY is None: the_centerY = centerY		
		
		# check if ver0, ver1, ver2 is constant
		if the_ver0 != ver0 or the_ver1 != ver1 or the_ver2 != ver2:
			print "Not Version!"
			return False
			
		# check if maxCID == initCID
		if maxCID != initCID:
			print "Not always maxCID go as initial!"
			print maxCID, initCID
			return False
			
		# check if 'maxFrame' is constant
		if maxFrame != the_maxFrame:
			print "Not maxFrame constant!"
			return False
			
		# check if 'minDepth' is constant
		if minDepth != the_minDepth:
			print "Not minDepth constant!"
			return False
			
		# check if 'centerX' and 'centerY' is constant
		if centerX != the_centerX or centerY != the_centerY:
			print "Not center Pos constant!"
			return False
	return True
			
def check_F004():
	for lumen in iter():
		rip_gim.list_tagF023_img(lumen)
		
def check_F007():
	for lumen in iter():
		image_info_list = rip_gim.list_tagF007_symbol(lumen)
		for i in xrange(len(image_info_list)):
			if i != image_info_list[i][0]:
				print "Image Idx not consistent!"
				return False
	return True
			
def check_0004():
	values = {}
	v2 = 0
	for lumen in iter():
		for tag in iter_tag(lumen, (0x0004, )):
			v, = struct.unpack("<h", tag[0x1e:0x20])
			values.setdefault(v, 0)
			values[v] += 1
	
#			if v == 663:
#				v2 += 1
#				if v2 == 1:
#					return 
					
	for k, v in values.iteritems():
		print "%d: %d" % (k, v)

def check_F00D():
	values = {}
	v2 = 0

	f018_count = 0
		
	for lumen in iter():
		texture_count = 0
		sprite_count = 0	
		texture_num = None
		sprite_num = None			
		
		for tag in iter_tag(lumen, (0xF00D, 0xF022, 0x0027, 0xF018)):
			
			tag_type, = struct.unpack("<H", tag[:0x2])
			if tag_type == 0xF022:
				texture_count += 1
			elif tag_type == 0x0027:
				sprite_count += 1
			elif tag_type == 0xF00D:				
				texture_num, unk1, sprite_num, unk2, unk3, unk4 = struct.unpack("<HHHHII", 
				tag[0x4:0x14])

				if unk3 == 25:
					v2 += 1
					if v2 == 1:
						return 			
						
				values.setdefault(unk3, 0)
				values[unk3] += 1
				break
			
#		assert texture_num == texture_count, "%d %d" % (texture_num, 
#			texture_count)
#		assert sprite_num == sprite_count, "%d %d" % (sprite_num, sprite_count)
		


	
	for k, v in values.iteritems():
		print "%d: %d" % (k, v)
		
def check_002B():
	values = {}
	v2 = 0
	for lumen in iter():
		for tag in iter_tag(lumen, (0x002b, )):
			v, = struct.unpack("<I", tag[0x8:0xc])
			values.setdefault(v, 0)
			values[v] += 1
	
#			if v == 663:
#				v2 += 1
#				if v2 == 1:
#					return 
					
	for k, v in values.iteritems():
		print "%d: %d" % (k, v)
		
		
def check_unknown_tag():
	all_tag_types = range(0xF001, 0xF006) + range(0xF007, 0xF00F) + [0xF105, 0xF014, 0xF103, 0xFF00, 0xF022, 0xF023, 0xF018] + [0x1, 0x4, 0x5, 0xc, 0x7, 0x2b, 0x25, 0x27, 0xa, 0xb]
	
	# new tag: 0x000b, 0x0007,
	for lumen in iter():
		for tag in iter_tag(lumen):
			tag_type, tag_size = struct.unpack("<HH", tag[:0x4])
			if tag_type not in all_tag_types:
				print "New Tag 0x%04x, size=0x%x" % (tag_type, (tag_size+1)*4)
				return
			
checkers = [
#	check_F00C, 
#	check_F004,
#	check_F007,
#	check_0004,
#	check_F00D,
	check_unknown_tag,
	]
for checker in checkers:
	checker()