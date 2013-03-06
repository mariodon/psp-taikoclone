import re
import struct

def _read_normal(fdef, tag, off, g):
	name, size, fmt = fdef
	if type(size) == type(str):
		size = eval(size)
		fmt = eval(fmt)
	assert size == struct.calcsize(fmt), "Size not match Format!"
	
	v = struct.unpack(fmt, tag[off: off + size])[0]
	g[name] = v
	
	return v, off + size
	
def _read_list(fdef, tag, off, g):
	name, size, fmt, arr = fdef
	assert fmt == "list", "Not A List"
	if type(size) == type(str):
		size = eval(size)
		
	v = []
	for i in xrange(size):
		for _fdef in arr:	# iter sub item
			name, size, fmt = _fdef
			if fmt == "list":
	
def _read_tag(fdef, tag, off, g):
	myv = {}
	for _fdef in fdef:
		name, size, fmt = _fdef
		if fmt == "list":
			v, tag, off = _read_list(_fdef, tag, off, g)
		else:
			v, tag, off = _read_normal(_fdef, tag, off, g)
		myv{name} = v
	return myv, tag, off
	
def read_tag(fdef, tag):
	off = 0
	g = {"off": 0}
	v, tag, off = _read_tag(fdef, tag, off, g)
	return v
	
	