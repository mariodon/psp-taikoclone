import struct

TWIPS_PER_PIXEL = 20
PLACE_FLAG_HAS_CLIP_ACTIONS = 0x80
PLACE_FLAG_HAS_CLIP_DEPTH = 0x40
PLACE_FLAG_HAS_NAME = 0x20
PLACE_FLAG_HAS_RATIO = 0x10
PLACE_FLAG_HAS_COLOR_TRANSFORM = 0x8
PLACE_FLAG_HAS_MATRIX = 0x4
PLACE_FLAG_HAS_CHARACTER = 0x2
PLACE_FLAG_MOVE = 0x1
	
def calc_nbits(v):
	v = abs(v)
	if v == 0:
		nbits = 1
	else:
		nbits = 0
		while v != 0:
			v >>= 1
			nbits += 1
		nbits += 1
	return nbits
	
def pack_fixed(v):
	a = int(v)
	b = int((v - a) * 0xFFFF)
	return struct.pack("<bb", b, a)
	
def pack_fixed16(v):
	a = int(v)
	b = int((v - a) * 0xFF)
	return struct.pack("<hh", b, a)
	
def pack_ubyte(v):
	return struct.pack("<B", v)
	
def pack_uword(v):
	return struct.pack("<I", v)
	
def pack_uhalf(v):
	return struct.pack("<H", v)

def pack_byte(v):
	return struct.pack("<b", v)
	
def pack_word(v):
	return struct.pack("<i", v)
	
def pack_half(v):
	return struct.pack("<h", v)
		
def pack_bits(vs, bits):
	totbits = sum(bits)
	padbits = (totbits + 7) / 8 * 8 - totbits
	vs = list(vs)
	bits = list(bits)
	if padbits > 0:
		bits.append(padbits)
		vs.append(0)
	ret = ""	
	cur_bits = 0
	cur_byte = 0
	idx = 0
	while idx < len(bits):
		if cur_bits < 8:
			cur_byte <<= bits[idx]
			cur_byte |= vs[idx] & ((1 << bits[idx]) - 1)
			cur_bits += bits[idx]
			idx += 1
		if cur_bits >= 8:
			ret += pack_ubyte(cur_byte >> cur_bits - 8)
			cur_bits -= 8
			cur_byte &= (1 << cur_bits) - 1
	return ret	
	
def pack_rect(xmin, ymin, xmax, ymax):
	vmax = max(abs(xmin), abs(ymin), abs(xmax), abs(ymax)) * TWIPS_PER_PIXEL
	nbits = calc_nbits(vmax)
	assert nbits <= 0x1F, "TOO LARGE VALUE %d" % nbits
	return pack_bits((nbits, xmin * TWIPS_PER_PIXEL, xmax * TWIPS_PER_PIXEL, 
		ymin * TWIPS_PER_PIXEL, ymax * TWIPS_PER_PIXEL),
		(5, nbits, nbits, nbits, nbits))
	
def pack_matrix(scale, rotate, translate):
	has_scale = (scale is not None)
	has_rotate = (rotate is not None)
	has_translate = (translate is not None)
	
	vs = []
	bits = []
	if has_scale:
		scale_x = pack_fixed(scale[0])
		scale_y = pack_fixed(scale[1])
		vmax = max(abs(scale_x, scale_y))
		nbits = calc_nbits(vmax)
		assert nbits <= 0xF, "TOO LARGE VALUE %d" % nbits
		vs.extend((1, nbits, scale_x, scale_y))
		bits.extend((1, 5, nbits, nbits))
	else:
		vs.append(0)
		bits.append(1)
		
	if has_rotate:
		rotate_x = pack_fixed(rotate[0])
		rotate_y = pack_fixed(rotate[1])
		vmax = max(abs(rotate_x, rotate_y))
		nbits = calc_nbits(vmax)
		assert nbits <= 0xF, "TOO LARGE VALUE %d" % nbits
		vs.extend((1, nbits, rotate_x, rotate_y))
		bits.extend((1, 5, nbits, nbits))
	else:
		vs.append(0)
		bits.append(1)	
		
	if has_translate:
		translate_x = pack_fixed(translate[0])
		translate_y = pack_fixed(translate[1])
		vmax = max(abs(translate_x, translate_y))
		nbits = calc_nbits(vmax)
		assert nbits <= 0xF, "TOO LARGE VALUE %d" % nbits
		vs.extend((1, nbits, translate_x, translate_y))
		bits.extend((1, 5, nbits, nbits))
	else:
		vs.append(0)
		bits.append(1)
		
	return pack_bits(vs, bits)		

def pack_color_transform_with_alpha(color_add, color_mul):
	has_add_items = (color_add is not None)
	has_mul_items = (color_mul is not None)
	vs = []
	bits = []
	
	vs.extend((int(has_add_items), int(has_mul_items)))
	bits.extend((1, 1))
	
	nbits = 0
	if has_add_items:
		for v in color_add:
			nbits = max(nbits, calc_nbits(v))
			vs.append(v)
	if has_mul_items:
		for v in color_mul:
			v = pack_fixed16(v)
			nbits = max(nbits, calc_nbits(v))
			vs.append(v)
	assert nbits <= 0xF, "TOO LARGE VALUE %d" % nbits
	added_cnt = len(vs) - len(bits)
	
	vs.append(nbits)
	bits.append(4)
	
	bits.extend([nbits] * added_cnt)
	
	return pack_bits(vs, bits)
	
def make_record_header(tag_type, length):
	if length >= 63:
		ret = pack_uhalf(tag_type << 10 + 63) + pack_word(length)
	else:
		ret = pack_uhalf(tag_type << 10 + length)
	return ret
	
def make_swf_header(version, file_length, frame_width, frame_height, 
	frame_rate, frame_count):
	ret = "FWS"
	ret += pack_ubyte(version)
	ret += pack_uword(file_length)
	ret += pack_rect(0, 0, frame_width, frame_height)
	ret += pack_fixed16(frame_rate)
	ret += pack_uhalf(frame_count)
	return ret
	
def make_define_bits_JPEG2_tag(id, image_data):
	ret = make_record_header(21, 2+len(image_data))
	ret += pack_uhalf(id)
	ret += image_data
	
def make_define_sprite_tag(id, frame_count, control_tags):
	data = pack_uhalf(id)
	data += pack_uhalf(frame_count)
	data += "".join(control_tags)
	return make_record_header(39, len(data)) + data
	
def make_show_frame_tag():
	return make_record_header(1, 0)
	
# WARNING: to be completed
def make_place_object2_tag(flags, depth, id=None, matrix=None, 
	color_trans=None, ratio=None, name=None, clip_depth=None, 
	clip_actions=None):
	data = ""
	data += pack_ubyte(flags)
	data += pack_uhalf(depth)
	if flags & PLACE_FLAG_HAS_CHARACTER:
		data += pack_uhalf(id)	
	if flags & PLACE_FLAG_HAS_MATRIX:
		data += matrix	
	if flags & PLACE_FLAG_HAS_COLOR_TRANSFORM:
		data += color_trans		
	if flags & PLACE_FLAG_HAS_NAME:
		data += name
	return make_record_header(26, len(data)) + data
	
def make_end_tag():
	return make_record_header(0, 0)
	
if __name__ == "__main__":
	f, = struct.unpack("<f", "\x00\x00\x70\x42")
	print repr(pack_fixed16(f))