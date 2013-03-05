import os
import struct

def iter_lumen(path):
	for filename in os.listdir(path):
		if filename.upper().endswith(".CLM"):
			f = open(os.path.join(path, filename), "rb")
			data = f.read()
			f.close()
			print filename
			yield data
		elif os.path.isdir(filename):
			for sub_lumen in iter_lumen(os.path.join(path, filename)):
				yield sub_lumen
				
			
prev_data = None
matched_off = []
for lumen in iter_lumen("."):

	data = lumen[:0x60]
	if prev_data is None:
		prev_data = data
		matched_off = range(0x60)
		
	for i, (a, b) in enumerate(zip(data, prev_data)):
		if i not in matched_off:
			continue
		if a != b:
			matched_off.remove(i)

for off in matched_off:
	print "off=0x%02x, v = 0x%02x" % (off, struct.unpack("<B", prev_data[off:off+1])[0])