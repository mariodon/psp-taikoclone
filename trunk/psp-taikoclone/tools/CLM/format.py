tagFormat = [

	0xF001: (
		("tag_type", 0x4, ">I"),
		("tag_size", 0x4, ">I"),
		("symbol_cnt", 0x4, ">I"),
		("symbol_list", "g['symbol_cnt']", "list",
			(
				("symbol_len", 0x4, ">I"),
				("symbol", "g['symbol_len']", "'>%ds'%g['symbol_len']"),
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
				("R", 0x4, ">h"),
				("G", 0x4, ">h"),
				("B", 0x4, ">h"),
				("A", 0x4, ">h"),								
			)
		),
	),
	
	
]