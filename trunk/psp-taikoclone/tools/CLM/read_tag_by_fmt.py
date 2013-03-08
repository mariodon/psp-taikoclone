import re
import struct

def _read_normal(fdef, tag, off, g):
	name, size, fmt = fdef
	if type(size) == type("") and "g['" in size:
		size = eval(size)
	if type(fmt) == type("") and "g['" in fmt:
		fmt = eval(fmt)
	
	if size == 0:
		v = None
	else:	
		assert size == struct.calcsize(fmt), "Size not match Format!"
		v = struct.unpack(fmt, tag[off: off + size])[0]
		
	g[name] = v
	g['off'] = off + size
	
#	print "%s = %r" % (name, v)
	
	return v, tag, off + size
	
def _read_list(fdef, tag, off, g):
	name, size, fmt, arr = fdef
	assert fmt == "list", "Not A List"
	if type(size) == type("") and "g['" in size:
		size = eval(size)
	
	v = []
	for i in xrange(size):
		_v, tag, off = _read_tag(arr, tag, off, g)
		v.append(_v)
#		print _v
	g[name] = v
	return v, tag, off
	
def _read_tag(fdef, tag, off, g):
	myv = {}
	for _fdef in fdef:
#		print _fdef
		name, size, fmt = _fdef[:3]
		if fmt == "list":
			v, tag, off = _read_list(_fdef, tag, off, g)
		else:
			v, tag, off = _read_normal(_fdef, tag, off, g)
		myv[name] = v
	return myv, tag, off
	
def read_tag(fdef, tag):
	off = 0
	g = {"off": 0}
	v, tag, off = _read_tag(fdef, tag, off, g)
	
	# TODO: Remove This!
	assert off == v["tag_size"] * 4 + 8
	
	return v
	
#f = open(r"CLM_split\result.lm", "rb")
#data = f.read()
#f.close()
#
#import format
#import rip_gim_wii
#
#the_type = 0xF003
#for off, tag_type, tag_size_bytes, tag in rip_gim_wii.iter_tag(data, (the_type,)):
#	print read_tag(format.DATA[the_type], tag)