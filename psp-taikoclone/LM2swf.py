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
#        print len(data[:0x4])
        tag_type, tag_size = struct.unpack("<HH", data[:0x4])
        if tag_type == 0xFF00:
            break
        if tag_type == 0x0027:

            control_tags = []
            sprite_id, = struct.unpack("<H", data[0x4:0x6])
            frame_count, = struct.unpack("<H", data[0xc:0xe])
            print "frame_count %d" % frame_count
            
            frame_label_cnt, = struct.unpack("<H", data[0xa:0xc])
            print "frame lable cnt %d" % frame_label_cnt
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
#                print "frame %d, placeobject%d" % (i, ptag_cnt)
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
                        name=name, matrix=matrix, color_trans=color_trans, clip_actions=clip_actions, ratio=ratio)
                    control_tags.append(ptag)
                    
                    if matrix:
                        depth2matrix[depth] = matrix
                    if color_trans:
                        depth2color_trans[depth] = color_trans
                    
                if i in frame_label_dict:
                    control_tags.append(swf_helper.make_frame_label_tag(
                        frame_label_dict[i]))
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
            print "empty record trimmed"
            continue
        if action_code < 0x80:
            ret.append(data[0])
            data = data[0x1:]
        else:
            length, = struct.unpack("<H", data[0x1:0x3])
            record = data[:length + 0x3]
            
            # trim some record
            if action_code == 0x9B:
                func_name_len = 0
                while data[0x3 + func_name_len] != '\x00':
                    func_name_len += 1
                func_name = data[0x3:0x3+func_name_len]
                # off:0x4+func_name_len
                num_param, = struct.unpack("<H", 
                    data[0x4+func_name_len:0x6+func_name_len])
                # off:0x6+func_name_len
                str_cnt = 0
                idx = 0x6 + func_name_len
                while str_cnt < num_param:
                    if data[idx] == '\x00':
                        str_cnt += 1
                    else:
                        idx += 1
                if num_param > 0:
                    idx += 1

                # off: idx
                
                code_size, = struct.unpack("<H", data[idx+1:idx+3])
                print "code size = %x" % code_size
                # off:idx+2
                
                assert idx+2 <= length+0x3
                # fix record size
                record = record[0x0:0x1] + struct.pack("<H", length-1) + \
                    record[0x3:0x3+length-3] + record[-2:]
                print "define func trimmed %d" % (length - idx + 1)
                    
                # trim sub block
                print "sub=======, %x" % \
                    len(data[length+0x3:length+0x3+code_size])
                sub = fix_action_record(data[length+0x3:length+0x3+code_size], 
                    symbol_list)
                # fix code_size
                record = record[:-2] + struct.pack("<H", len(sub))
                ret.append(record+sub)
                data = data[length + 0x3 + code_size:]
                print "fixed_code_size %x %x" % (code_size, len(sub))
            elif action_code == 0x9D:
                branch_off, = struct.unpack("<H", record[-2:])
                record = record[:0x3] + struct.pack("<H", branch_off-1)
                ret.append(record)
                data = data[length + 0x3:]
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
                    elif push_type in (0x6,):
                        bytes = raw_items[0x5:0x9] + raw_items[0x1:0x5]
                        off = 0x8
                    elif push_type == 0x0:
                        str_idx, = struct.unpack("<H", raw_items[0x1:0x3])
                        _str = symbol_list[str_idx]
                        bytes = swf_helper.pack_string(_str)
                        off = len(bytes)
                    elif push_type == 0x2:
                        bytes = ""
                        off = 0x0
                    else:
                        assert False, "not supported push type %x" % push_type
                    fixed_record += raw_items[0x0:0x1] + bytes
                    raw_items = raw_items[off + 1:]
                
#                assert len(fixed_record) == len(record)
                # fix the action push record size
                fixed_record = fixed_record[0x0:0x1] + struct.pack("<H", 
                    len(fixed_record)-0x3) + fixed_record[0x3:]
                ret.append(fixed_record)
                data = data[length + 0x3:]
#TODO: fix ActionIf branchoffset
            else:
                ret.append(record)
                data = data[length + 0x3:]
            
    print "===="
    return "".join(ret)
    
def test(fname):
    image_root = r"D:\tmp_dl\disasmTNT\GimConv\png"
#    fname = "CHIBI_1P_BALLOON_01.LM"
    f = open(fname, "rb")
    lm_data = f.read()
    f.close()
    
    # init
    symbol_table = rip_gim.get_symbol_list(lm_data[0x40:])
    assert symbol_table[0] == ""
    constant_pool = "".join([str+"\x00" for str in symbol_table])
    action_constant_pool = struct.pack("<BHH", 0x88, 2+len(constant_pool), 
        len(symbol_table)) + constant_pool
    action_record_list = rip_gim.list_tagF005_symbol(lm_data)
#    action_record_list = map(fix_action_record, action_record_list)
    print len(action_record_list)
    for i in xrange(len(action_record_list)):
        action_record_list[i] = fix_action_record(action_record_list[i], symbol_table)
#    fix_action_record(action_record_list[2])
    
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
    define_sprite_tags_general = get_define_sprite_tags(lm_data, action_constant_pool, action_record_list)
    all_tags.extend(define_sprite_tags_general)
    
    # test basic display
    tmp_tags = []
    
    # INSTANCE ID(ratio) should be enough!
    tmp_tags.append(swf_helper.make_place_object2_tag(swf_helper.PLACE_FLAG_HAS_CHARACTER|swf_helper.PLACE_FLAG_HAS_MATRIX|swf_helper.PLACE_FLAG_HAS_NAME|swf_helper.PLACE_FLAG_HAS_RATIO, 1, id=max_characterID, matrix=swf_helper.pack_matrix(None, None, (0, 0), ),name="main",ratio=0xFFFF))

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