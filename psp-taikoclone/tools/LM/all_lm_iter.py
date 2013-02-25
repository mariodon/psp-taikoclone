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
		if tag_type in _type_set:
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
			v, = struct.unpack("<H", tag[0xe:0x10])
			values.setdefault(v, 0)
			values[v] += 1
			if v == 3:
				v2 += 1
				if v2 == 2:
					return
			
	
	for k, v in values.iteritems():
		print "%d: %d" % (k, v)

#0xe:0x10	
#0: 373658
#1: 91
#3: 2
#4: 2
#8: 1123
#9: 4				
		
checkers = [
#	check_F00C, 
#	check_F004,
#	check_F007,
	check_0004,
	]
for checker in checkers:
	checker()