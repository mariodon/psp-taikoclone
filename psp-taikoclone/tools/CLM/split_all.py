import os
import sys
import rip_gim_wii

for dirname, _, filenames in os.walk(r"CLM_split"):
	for filename in filenames:
		if filename.endswith(".lm"):
			f = open(os.path.join(dirname, filename), "rb")
			data = f.read()
			f.close()
#			print filename
#			rip_gim_wii.list_tagF008_symbol(data)
#			rip_gim_wii.list_tagF009_symbol(data)
#			rip_gim_wii.list_tagF00A_symbol(data)
#			rip_gim_wii.list_tagF00B_symbol(data)	
#			rip_gim_wii.list_tagF00C_symbol(data)
#			rip_gim_wii.list_tagF00D_symbol(data)
#			rip_gim_wii.list_tagF022_symbol(data, filename)			
#			rip_gim_wii.list_tag0027_symbol(data, filename)
			rip_gim_wii.list_tagF024_img(data)			