DATA = {

	0xF001: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("symbol_cnt", 0x4, ">I"),
		("symbol_list", "g['symbol_cnt']", "list",
			(
				("symbol_len", 0x4, ">I"),
				("symbol", "g['symbol_len']", "'>%ds'%g['symbol_len']"),
				("zero", 0x1, ">B"),
				("padding", "3-(g['off']+3)%4", "'>%ds'%(3-(g['off']+3)%4)"),
			)
		),
	),

	0xF002: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("color_cnt", 0x4, ">I"),
		("color_list", "g['color_cnt']", "list",
			(
				("R", 0x2, ">h"),
				("G", 0x2, ">h"),
				("B", 0x2, ">h"),
				("A", 0x2, ">h"),								
			)
		),
	),
	
	0xF003: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("mat_cnt", 0x4, ">I"),
		("mat_list", "g['mat_cnt']", "list",
			(
				("scale_x", 0x4, ">f"),
				("rotateskew_x", 0x4, ">f"),
				("rotateskew_y", 0x4, ">f"),
				("scale_y", 0x4, ">f"),
				("trans_x", 0x4, ">f"),
				("trans_y", 0x4, ">f"),				
			)
		),
	),
	
	0xF103: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("pos_cnt", 0x4, ">I"),
		("pos_list", "g['pos_cnt']", "list",
			(
				("x", 0x4, ">f"),
				("y", 0x4, ">f"),
			),
		),
	),

	0xF004: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("box_cnt", 0x4, ">I"),
		("box_list", "g['box_cnt']", "list",
			(
				("xmin", 0x4, ">f"),
				("ymin", 0x4, ">f"),
				("xmax", 0x4, ">f"),
				("ymax", 0x4, ">f"),				
			),
		),
	),

	0xF005: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("as_cnt", 0x4, ">I"),
		("as_list", "g['as_cnt']", "list",
			(
				("as_len", 0x4, ">I"),
				("bytecode", "g['as_len']", "'>%ds'%g['as_len']"),
				("padding", "3-(g['off']+3)%4", "'>%ds'%(3-(g['off']+3)%4)"),
			)
		),
	),

	0xF007: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("img_cnt", 0x4, ">I"),
		("img_list", "g['img_cnt']", "list",
			(
				("img_idx", 0x4, ">I"),
				("name_idx", 0x4, ">I"),
				("width", 0x4, ">f"),
				("height", 0x4, ">f"),
			)
		),
	),

	0xF008: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("unk_cnt", 0x4, ">I"),
		("unk_list", "g['unk_cnt']", "list",
			(
				("unk1", 0x4, ">I"),
				("unk2", 0x4, ">I"),
			)
		),
	),

	0xF009: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("unk", 0x4, ">I"),
	),

	0xF00A: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("unk_cnt", 0x4, ">I"),
		("unk_list", "g['unk_cnt']", "list",
			(
				("unk1", 0x4, ">I"),
				("unk2", 0x4, ">I"),
				("unk3", 0x4, ">I"),
				("unk4", 0x4, ">I"),
				("unk5", 0x4, ">I"),
			)
		),
	),
	
	0xF00B: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("unk", 0x4, ">I"),
	),
				
	0xF00C: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("v", 0x4, ">I"),
		("e", 0x4, ">I"),
		("r", 0x4, ">I"),
		("max_character_id", 0x4, ">I"),
		("reserved", 0x4, ">i"),
		("start_character_id", 0x4, ">I"),
		("unk", 0x2, ">H"),
		("reserved2", 0x2, ">H"),
		("fps", 0x4, ">f"),
		("width", 0x4, ">f"),
		("height", 0x4, ">f"),
		("x", 0x4, ">f"),
		("y", 0x4, ">f"),
	),

	0xF00D: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("f022_cnt", 0x4, ">I"),
		("0007_cnt", 0x4, ">I"),
		("0027_cnt", 0x4, ">I"),
		("000b_cnt", 0x4, ">I"),
		("0025_cnt", 0x4, ">I"),
		("const0_0", 0x4, ">I"),
		("const1_0", 0x4, ">I"),
		("const2_0", 0x4, ">I"),		
	),

	0xF022: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("character_id", 0x4, ">I"),
		("const0_0", 0x4, ">I"),
		("size_idx", 0x4, ">I"),
		("f023_cnt", 0x4, ">I"),
		("f024_cnt", "(g['tag_size']-4)*4", ">I"),
	),
			
	0xF023: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("x0", 0x4, ">f"),
		("y0", 0x4, ">f"),
		("u0", 0x4, ">f"),
		("v0", 0x4, ">f"),
		("x1", 0x4, ">f"),
		("y1", 0x4, ">f"),
		("u1", 0x4, ">f"),
		("v1", 0x4, ">f"),
		("x2", 0x4, ">f"),
		("y2", 0x4, ">f"),
		("u2", 0x4, ">f"),
		("v2", 0x4, ">f"),
		("x3", 0x4, ">f"),
		("y3", 0x4, ">f"),
		("u3", 0x4, ">f"),
		("v3", 0x4, ">f"),
		("unk", 0x4, ">I"),
		("fill_style", 0x2, ">H"),
		("fill_idx", 0x2, ">H"),
	),
		
	0x0027: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("character_id", 0x4, ">I"),
		("const0_0", 0x4, ">I"),
		("class_name_idx", 0x4, ">I"),
		("frame_label_cnt", 0x4, ">I"),
		("0001_cnt", 0x4, ">I"),
		("key_frame_cnt", 0x4, ">I"),
		("max_depth", 0x2, ">H"),
		("const1_0", 0x2, ">H"),
	),
		
	0xFF00: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
	),
}