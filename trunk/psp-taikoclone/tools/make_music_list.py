# $Id$
# -*- coding:gbk -*-
import codecs
import optparse
import os
import os.path
import sys
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

    # encoding priority
    encoding_list = ["CP932", "GBK", "Big5", "UTF-8"]        
    
    # limit to encoding_list to which can make you open the file
    if header["WAVE"] == "":
        return None

    encoding_list_valid_open = []
    for encoding in encoding_list:        
        wave_file = os.path.join(os.path.dirname(filename), header["WAVE"])
        try:
            wave_file = wave_file.decode(encoding)
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
                
    
    
if __name__ == "__main__":
    options, args = parser.parse_args()
    
    tja_files = get_tja_files(options.path, options.recursive)
    
    # guessing encoding test
    if options.debug:
        failed_cnt = 0
        tot = len(tja_files)
        failed_files = []
        
#        sys.stdout = codecs.open("a.txt", "w", encoding="gbk")
        for tja_file in tja_files:
            if get_tja_encoding(tja_file) is None:
                failed_files.append(tja_file)
                failed_cnt += 1
        
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
            
    # make music list offline test
    if options.debug:
        pass