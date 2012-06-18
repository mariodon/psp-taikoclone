import optparse

parser = optparse.OptionParser()
parser.add_option("-d", "--directory", dest="path", default="..")
parser.add_option("-r", "--recursive", action="store_true", \
        dest="recursive", default=False)

if __name__ == "__main__":
    options, args = parser.parser_args()
