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
    
def get_texture_sprite_tags(lm_data, image_2_id, image_dict):
    data = lm_data[0x40:]
    
    symbol_list = rip_gim.get_symbol_list(data)
    img_fname_list = [symbol for symbol in symbol_list \
        if symbol.endswith(".png")]
    define_sprite_tags = []
    while True:
        tag_type, tag_size = struct.unpack("<HH", data[:0x4])
        if tag_type == 0xFF00:
            break
        if tag_type == 0xF022:
            sprite_id, = struct.unpack("<I", data[0x4:0x8])
            img_cnt, = struct.unpack("<H", data[0xa:0xc])
            place_object2_tags = []
            for i in xrange(img_cnt):    # handle each F022 tag
                data = rip_gim.seek_next_tag(data)
                img_fname_idx, flag = struct.unpack("<HH", data[0x44:0x48])
                assert flag == 0x41, "not supported (Shape?) atm!"
                img_fname = img_fname_list[img_fname_idx]
                all_floats = struct.unpack("<"+"f"*16, data[0x4:0x4+0x4*16])
                xs = all_floats[::4]
                ys = all_floats[1::4]
                xmin, xmax = min(xs), max(xs)
                ymin, ymax = min(ys), max(ys)
                width = xmax - xmin
                height = ymax - ymin
                img_data = image_dict[img_fname]
                ori_width, ori_height = struct.unpack(">II", img_data[0x10:0x18])
                scale_x = width / ori_width
                scale_y = height / ori_height
                matrix = swf_helper.pack_matrix((scale_x, scale_y), None, (xmin, ymin))
                place_object2_tag = swf_helper.make_place_object2_tag(swf_helper.PLACE_FLAG_HAS_CHARACTER | swf_helper.PLACE_FLAG_HAS_MATRIX, i, id=image_2_id[img_fname], matrix=matrix)
                place_object2_tags.append(place_object2_tag)
            show_frame_tag = swf_helper.make_show_frame_tag()
            
            control_tags = []
            control_tags.extend(place_object2_tags)
            control_tags.append(show_frame_tag)
            define_sprite_tag = swf_helper.make_define_sprite_tag(sprite_id, 1, control_tags)
            define_sprite_tags.append(define_sprite_tag)
        else:
            data = rip_gim.seek_next_tag(data)
    return define_sprite_tags
    
def get_define_sprite_tags(lm_data):
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
            
def test():
    image_root = "D:\\tmp_dl\\disasmTNT\\GimConv\\png"
    fname = "LOADING_WIPE_00_EASY.LM"
    f = open(fname, "rb")
    lm_data = f.read()
    f.close()
    
    max_characterID = get_max_characterID(lm_data)
    
    image_dict = get_image_dict(lm_data, image_root)
    
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
    all_tags.append(swf_helper.make_define_shape_tag_bitmap_simple(
        max_characterID + len(image_dict) + 1, max_characterID + 1, 128, 128))

    # make all texture mc tags
    define_sprite_tags = get_texture_sprite_tags(lm_data, image_2_id, image_dict)
#    all_tags.extend(define_sprite_tags)

    # test basic display
    tmp_tags = []
    tmp_tags.append(swf_helper.make_place_object2_tag(swf_helper.PLACE_FLAG_HAS_CHARACTER|swf_helper.PLACE_FLAG_HAS_NAME|swf_helper.PLACE_FLAG_HAS_MATRIX, 1, name="test", id=0x1B, matrix=swf_helper.pack_matrix(None, None, (0, 0))))
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
    
    fout = open("test.swf", "wb")
    fout.write(swf_header + all_data)
    fout.close()
    
if __name__ == "__main__":
    test()