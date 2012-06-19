# $Id$
# -*- coding:gbk -*-
import codecs
import optparse
import os
import os.path
import sys
import struct
import unicodedata

parser = optparse.OptionParser()
parser.add_option("-d", "--directory", dest="path", default="..")
parser.add_option("-r", "--recursive", action="store_true", \
        dest="recursive", default=False)
parser.add_option("-D", "--debug", action="store_true", dest="debug", default=False)

# scan root (recursively) to get a list of tja_files
def get_tja_files(root, recursive):
    ret = []
    for path, folders, filenames in os.walk(root):
        tja_filenames = filter(lambda x:x.endswith(".tja"), filenames)
        tja_full_filenames = map(lambda x:os.path.join(path, x), tja_filenames)
        ret.extend(tja_full_filenames)
        if not recursive:
            break
    return ret

# guess the encoding of a tja_file
def get_tja_encoding(filename):
    # open file
    f = None
    try:
        f = open(tja_file)
    except IOError, ValueError:
        return None
    
    # parse titile, subtitle, wave
    header = {"SUBTITLE":"", "TITLE": "", "WAVE":""}
    for line in f:
        if line.startswith("#START"):
            break
        for header_name in header.iterkeys():
            if line.startswith(header_name):
                header[header_name] = line.split(":")[1].strip()
    f.close()

    # encoding priority
    encoding_list = ["CP932", "GBK", "Big5", "UTF-8"]        
    
    # limit to encoding_list to which can make you open the file
    if header["WAVE"] == "":
        return None

    encoding_list_valid_open = []
    dirname = os.path.dirname(filename).decode("mbcs")
    
    for encoding in encoding_list:
        try:
            wave_file = os.path.join(dirname, header["WAVE"].decode(encoding))
            assert isinstance(wave_file, unicode)
            if os.path.lexists(wave_file):
                encoding_list_valid_open.append(encoding)
        except:
            continue
        
    # check if any encoding fits all headers
    if len(encoding_list_valid_open) == 0:
        return None
    else:
        ret = None
        for encoding in encoding_list_valid_open:
            all_ok = True

            for v in header.itervalues():
                try:
                    v.decode(encoding).encode("gbk")
                except:
                    all_ok = False
                    break
            if all_ok:
                ret = encoding
                break
        if ret:
#            print
#            print ret        
            for k, v in header.iteritems():
                v = v.decode(ret)
#                print k, ":", v
                # check if ucs2 can handle this on psp
                if len(v) > 0 and ord(v[0]) > 0xFFFF:
                    return None
        return ret
                
def make_tja_info(tja_file, encoding):
    info = {"tja_file": tja_file.decode(sys.getfilesystemencoding()), "wave_file":"", "title":"", "subtitle":"",
            "course_cnt":0, "course_levels":[], "seek_pos":[], }

    f = None
    try:
        f = open(tja_file, "r")
    except IOError, ValueError:
        return None

    header = {"SUBTITLE":"", "TITLE": "", "WAVE":""}
    course_header = {"COURSE":"Oni", "LEVEL":'9',}
    last_seek_pos = 0
    while True:
        line = f.readline()
        if line == "": break

        if line.startswith("#START"):
            info["course_cnt"] += 1
            info["course_levels"].append(int(course_header["LEVEL"]))
            info["seek_pos"].append(last_seek_pos)
        elif line.startswith("#END"):
            last_seek_pos = f.tell()
            course_header = {"COURSE":"Oni", "LEVEL":'9'}

        for header_name in header.iterkeys():
            if line.startswith(header_name+':'):
                header[header_name] = line.split(":")[1].strip()
        for header_name in course_header.iterkeys():
            if line.startswith(header_name+':'):
                course_header[header_name] = line.split(":")[1].strip()
    f.close()

    info["title"] = header["TITLE"].decode(encoding)
    info["subtitle"] = header["SUBTITLE"].decode(encoding)
    info["wave_file"] = os.path.join(os.path.dirname(tja_file),
            header["WAVE"]).decode(encoding)

    return info

bad_data = "\0" * (200 * 2 + 100 * 2 + 1 + 8 * (4 + 4))
    
def pack_tja_info(info):

    ret = ""
    
    # pack all data in utf8
    
    # limit filenames to 100
    # padding if not enough
    tja_file = info["tja_file"].encode("utf-8")
    if len(tja_file) < 200:
        tja_file += "\0" * (200 - len(tja_file))
    else:
        return bad_data
        
    wave_file = info["wave_file"].encode("utf-8")
    if len(wave_file) < 200:
        wave_file += "\0" * (200 - len(wave_file))
    else:
        return bad_data
    
    # limit title and subtitle to 50 
    title = info["title"].encode("utf-8")
    if len(title) < 100:
        title += "\0" * (100 - len(title))
    else:
        title = title[:99] + "\0"
        
    subtitle = info["subtitle"].encode("utf-8")
    if len(subtitle) < 100:
        subtitle += "\0" * (100 - len(subtitle))
    else:
        subtitle = subtitle[:99] + "\0"    
        
    # limit to 8 course
    ret += struct.pack(">200s200s100s100sB", tja_file, wave_file, title, subtitle, info["course_cnt"])
    course_cnt = min(8, info["course_cnt"])
    for i in xrange(8):
        if i < course_cnt:
            ret += struct.pack(">II", info["course_levels"][i], info["seek_pos"][i])
        else:
            ret += struct.pack(">II", 0, 0)
    
    assert len(ret) == 200 * 2 + 100 * 2 + 1 + 8 * (4 + 4), len(ret)
    
    return ret
        
    
if __name__ == "__main__":
    options, args = parser.parse_args()
    
    tja_files = get_tja_files(options.path, options.recursive)

    data = []
    
    # guessing encoding test
    if options.debug:
        failed_cnt = 0
        tot = len(tja_files)
        failed_files = []
        
#        sys.stdout = codecs.open("a.txt", "w", encoding="gbk")
        for tja_file in tja_files:
            encoding = get_tja_encoding(tja_file)
            if encoding is None:
                failed_files.append(tja_file)
                failed_cnt += 1
            else:
                data.append(make_tja_info(tja_file, encoding))
        
        print        
        print "all done"
        print failed_cnt, "/", tot

        print
        print "failed files:"
        for failed_file in failed_files:
            print failed_file, os.path.isfile(failed_file)
        
        if failed_cnt > 0:
            print "Sorry,"    
            print "these files may contain some characters in them or filenames, python can't handle."
            print "Or, wave file is missing. Please check or rename"
            
    print "注意，会忽略路径名过长的文件"
    
#    max_str_len = 0
#    for datum in data:
#        max_str_len = max(max_str_len, len(datum["title"]))
#        max_str_len = max(max_str_len, len(datum["subtitle"]))
#        max_str_len = max(max_str_len, len(datum["tja_file"]))
#        max_str_len = max(max_str_len, len(datum["wave_file"]))                
#        print datum
#    print max_str_len
    f = open("fumen.lst", "wb")
    f.write(struct.pack(">I", len(data)))
    for datum in data:
        binary_data = pack_tja_info(datum)
        f.write(binary_data)
    f.close()