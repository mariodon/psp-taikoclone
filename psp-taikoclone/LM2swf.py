import os
import struct
import swf_helper
import rip_gim

def get_max_characterID(lm_data):
	data = lm_data[0x40:]
	off = 0x40
	while True:
		tag_type, tag_size = struct.unpack("<HH", data[:0x4])
		tag_size_bytes = tag_size * 4 + 4
		if tag_type == 0xF00C:
		    return struct.unpack("<H", data[0xa:0xc])[0]
		if tag_type == 0xFF00:
			break
		off += tag_size_bytes
		data = data[tag_size_bytes:]
	assert False, "missing tagF00C"
    
def get_image_dict(lm_data, image_root):
    image_dict = {}
    symbol_list = rip_gim.get_symbol_list(lm_data[0x40:])
    for sb in symbol_list:
        if sb.endswith(".png") and image_dict.get(sb) is None:
            image_file = sb[:-4] + ".gim.png"
            f = open(os.path.join(image_root, image_file), "rb")
            image_data = f.read()
            f.close()
            image_dict[sb] = image_data
    return image_dict
    
def test():
    image_root = "C:\Users\delguoqing\Downloads\disasmTNT\png"
    fname = "LOADING_WIPE_00_EASY.LM"
    f = open(fname, "rb")
    lm_data = f.read()
    f.close()
    
    max_characterID = get_max_characterID(lm_data)
    assert max_characterID == 18
    
    image_dict = get_image_dict(lm_data, image_root)
    assert len(image_dict) == 8
    
    # all tags append to this list
    all_tags = []
    
    # make FileAttributes tag
    all_tags.append(swf_helper.make_file_attributes_tag())
    
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
    # make all texture mc tags
    
    # make end tag
    end_tag = swf_helper.make_end_tag()
    all_tags.append(end_tag)
    
    # build swf header
    all_data = ""
    for tag in all_tags:
        all_data += tag
        
    swf_header = swf_helper.make_swf_header(0x8, 0, 480, 272, 60.0, 1)
    file_length = len(swf_header) + len(all_data)
    swf_header = swf_helper.make_swf_header(0x8, file_length, 480, 272, 60.0, 
        1)
    
    fout = open("test.swf", "wb")
    fout.write(swf_header + all_data)
    fout.close()
    
if __name__ == "__main__":
    test()