import os
import sys
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
    
def get_shape_dict(lm_data):
    data = lm_data[0x40:]
    color_list = rip_gim.list_tagF002_symbol(lm_data)
    off = 0x40
    ret = {}
    while True:
        tag_type, tag_size = struct.unpack("<HH", data[:0x4])
        tag_size_bytes = tag_size * 4 + 4
        if tag_type == 0xF022:
            sprite_id, = struct.unpack("<I", data[0x4:0x8])
            img_cnt, = struct.unpack("<H", data[0xa:0xc])
            for i in xrange(img_cnt):
                data = rip_gim.seek_next_tag(data)
                img_fname_idx, flag = struct.unpack("<HH", data[0x44:0x48])
                if flag == 0x00:
                    all_floats = struct.unpack("<"+"f"*16, 
                        data[0x4:0x4+0x4*16])
                    xs = all_floats[::4]
                    ys = all_floats[1::4]
                    xmin, xmax = min(xs), max(xs)
                    ymin, ymax = min(ys), max(ys)
                    width = xmax - xmin
                    height = ymax - ymin                
                    ret[(sprite_id << 16)+i] = (color_list[img_fname_idx], 
                        (int(width), int(height)))
            continue
        if tag_type == 0xFF00:
            break
        off += tag_size_bytes
        data = data[tag_size_bytes:]
    return ret
    
def get_texture_sprite_tags(lm_data, image_2_id, shape_2_id, image_dict):
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
            for i in xrange(img_cnt):    # handle each F023 tag
                data = rip_gim.seek_next_tag(data)
                img_fname_idx, flag = struct.unpack("<HH", data[0x44:0x48])
                if flag == 0x00:    # solid fill
                    all_floats = struct.unpack("<"+"f"*16, 
                        data[0x4:0x4+0x4*16])
                    xs = all_floats[::4]
                    ys = all_floats[1::4]
                    xmin, xmax = min(xs), max(xs)
                    ymin, ymax = min(ys), max(ys)
                    width = xmax - xmin
                    height = ymax - ymin
                    matrix = swf_helper.pack_matrix(None, None, (xmin, ymin))
                    shape_id = shape_2_id[(sprite_id<<16) + i]
                else:
                    assert flag == 0x41, "not supported (fill type) atm!"
                    img_fname = img_fname_list[img_fname_idx]
                    all_floats = struct.unpack("<"+"f"*16, 
                        data[0x4:0x4+0x4*16])
                    xs = all_floats[::4]
                    ys = all_floats[1::4]
                    xmin, xmax = min(xs), max(xs)
                    ymin, ymax = min(ys), max(ys)
                    width = xmax - xmin
                    height = ymax - ymin
                    img_data = image_dict[img_fname]
                    ori_width, ori_height = struct.unpack(">II",     
                        img_data[0x10:0x18])
                    scale_x = width / ori_width
                    scale_y = height / ori_height
                    shape_id = image_2_id[img_fname]
                    matrix = swf_helper.pack_matrix((scale_x, scale_y), None, 
                        (xmin, ymin))
                        
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
    
def get_define_sprite_tags(lm_data):
    # some tables for referrence
    color_list = rip_gim.list_tagF002_symbol(lm_data)
    point_list = rip_gim.list_tagF103_symbol(lm_data)
    matrix_list = rip_gim.list_tagF003_symbol(lm_data)
    symbol_list = rip_gim.get_symbol_list(lm_data[0x40:])
    define_sprite_tags = []
    
    data = lm_data[0x40:]
    data = rip_gim.seek_next_tag(data, (0x0027,))
    while True:
#        print len(data[:0x4])
        tag_type, tag_size = struct.unpack("<HH", data[:0x4])
        if tag_type == 0xFF00:
            break
        if tag_type == 0x0027:
            control_tags = []
            sprite_id, = struct.unpack("<H", data[0x4:0x6])
            frame_count, = struct.unpack("<H", data[0xc:0xe])
            print "frame_count %d" % frame_count
            for i in xrange(frame_count):
                data = rip_gim.seek_next_tag(data, (0x0001,))
                ptag_cnt, = struct.unpack("<H", data[0x6:0x8])
#                print "frame %d, placeobject%d" % (i, ptag_cnt)
                for j in xrange(ptag_cnt):
                    data = rip_gim.seek_next_tag(data, (0x0004, 0x0005, 0x000c))
                    _type = struct.unpack("<H", data[:0x2])[0]
                    if _type == 0x0005:
                        depth, = struct.unpack("<H", data[0x6:0x8])
                        control_tags.append(
                            swf_helper.make_remove_object2_tag(depth))    
                        continue
                    if _type == 0x000c:
                        control_tags.append(
                            swf_helper.make_do_action_tag(["\x07"]))
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
                        trans_idx &= 0xFF
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
#                    color_mul_idx = -1
#                    color_add_idx = -1
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
                    if flags & swf_helper.PLACE_FLAG_HAS_CHARACTER and \
                        flags & swf_helper.PLACE_FLAG_MOVE:
                        control_tags.append(
                            swf_helper.make_remove_object2_tag(depth))
                        flags &= (0xFFFF - swf_helper.PLACE_FLAG_MOVE)
                    ptag = swf_helper.make_place_object2_tag(flags, depth, id, 
                        name=name, matrix=matrix, color_trans=color_trans)
                    control_tags.append(ptag)
                    
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
            
def test(fname):
    image_root = r"C:\Users\delguoqing\Downloads\disasmTNT\png"
#    fname = "CHIBI_1P_BALLOON_01.LM"
    f = open(fname, "rb")
    lm_data = f.read()
    f.close()
    
    max_characterID = get_max_characterID(lm_data)
    
    image_dict = get_image_dict(lm_data, image_root)
    shape_dict = get_shape_dict(lm_data)
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
        width, height = struct.unpack(">II", img_data[0x10:0x18])
        tag = swf_helper.make_define_shape3_tag_bitmap_simple(id, 
            image_2_id[k], width, height)
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
    define_sprite_tags_general = get_define_sprite_tags(lm_data)
    all_tags.extend(define_sprite_tags_general)
    
    # test basic display
    tmp_tags = []
    tmp_tags.append(swf_helper.make_place_object2_tag(swf_helper.PLACE_FLAG_HAS_CHARACTER|swf_helper.PLACE_FLAG_HAS_MATRIX|swf_helper.PLACE_FLAG_HAS_NAME, 1, id=max_characterID, matrix=swf_helper.pack_matrix(None, None, (0, 0)), name="main"))

    action_records = []
    action_records.append("\x8B\x05\x00main\x00")   # ActionSetTarget "main"
    action_records.append("\x81\x02\x00\x01\x00")
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
    
    fout = open(fname[:-3] + ".swf", "wb")
    fout.write(swf_header + all_data)
    fout.close()
    
if __name__ == "__main__":
    test(sys.argv[1])