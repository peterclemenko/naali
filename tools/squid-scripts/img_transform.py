#!/usr/bin/python

import sys, os

from PIL import Image
import getopt

def LOG(s):
    #print s
    return


def print_usage(appname):
    print "Usage:\n  %s\n  -i|--input=inputfile <file> -o|--output=outputfile <file>" % appname
    print "  -f|--format=outputformat -O|--overwrite -q|--quality=<0-100>"
    print "  -x|--scalex <value> -y|--scaley <value> -p|-scalep <1-100>"
    print "  -D|--dontkeepratio -h|--help"

def do_scaling(image, scalex, scaley, scalep, keepratio):
    #
    # scale by percentace is overriding factor even if scalex and scaley are defined
    #
    size = image.size
    ratio = float(size[0])/float(size[1])
    if scalep != -1:
        new_x = size[0] * scalep / 100
        new_y = size[1] * scalep / 100
    else:
        if scalex == -1 and scaley == -1: return image
        if scalex == -1: scalex = scaley/ratio
        if scaley == -1: scaley = scalex/ratio
        new_x = scalex
        new_y = scaley
        new_ratio = float(scalex)/float(scaley)
        if keepratio == True:
            if ratio != new_ratio:
                # Keep X, but force change to Y
                new_y = new_x / ratio
    if new_x < 1: new_x = 1
    if new_y < 1: new_y = 1
    LOG( "resizing from (%d,%d) -> (%d,%d)" % (size[0], size[1], new_x, new_y))
    if size[0]==new_x and size[1]==new_y: return image
    return image.resize((int(new_x), int(new_y)), Image.BICUBIC)

if __name__ == "__main__": # if run standalone
    try:
        opts, args = getopt.getopt(sys.argv[1:], "i:o:f:Oq:x:y:p:hD", ["input=",
                                                                       "output=",
                                                                       "format=",
                                                                       "overwrite",
                                                                       "quality=",
                                                                       "scalex=",
                                                                       "scaley=",
                                                                       "scalep=",
                                                                       "dontkeepratio",
                                                                       "help"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err)
        print_usage(sys.argv[0])
        sys.exit(2)

    inputfile = ""
    outputfile = ""
    format = ""
    overwrite = False
    qual = 75
    scalex = -1
    scaley = -1
    scalep = -1
    keepratio = True

    for o, a in opts:
        if o in ("-i", "--input"):
            inputfile = str(a)
        elif o in ("-o", "--output"):
            outputfile = str(a)
        elif o in ("-f", "--format"):
            format = str(a)
            format = format.lower()
        elif o in ("-O", "--overwrite"):
            overwrite = True
        elif o in ("-q", "--quality"):
            qual = int(a)
            if qual < 1: qual = 1
            if qual > 95: qual = 95
        elif o in ("-x", "--scalex"):
            scalex = int(a)
            if scalex < 1: scalex = 1
        elif o in ("-y", "--scaley"):
            scaley = int(a)
            if scaley < 1: scaley = 1
        elif o in ("-p", "--scalep"):
            scalep = int(a)
            if scalep < 1: scalep = 1
        elif o in ("-D", "--dontkeepratio"):
            keepratio = False
        elif o in ("-h", "--help"):
            print_usage(sys.argv[0])
            sys.exit(0)
        else:
            assert False, "unhandled option"

    if inputfile == "": LOG( "No input file given. Abort!"); print_usage(sys.argv[0]); sys.exit(2)
    if format == "": format = inputfile[len(inputfile)-3:len(inputfile)]
    if outputfile == "": outputfile = inputfile + "." + format
    if outputfile == inputfile:
        LOG( "Error: Input and output cannot be the same. Abort!")
        sys.exit(2)
    if os.path.exists(outputfile):
        if overwrite == False:
            LOG( "Outputfile `%s` exists. Aborting. Use --overwrite to force overwriting." % outputfile)
            sys.exit(2)
        else:
            LOG( "Removing existing outputfile `%s`" % outputfile)
            os.remove(outputfile)
    
    LOG( "input '%s', output '%s', format '%s', overwrite '%s', quality=%d" % (inputfile, outputfile, format, overwrite, qual))
    LOG( "scalex %d, scaley %d, scalep %d, keepdatio=%s" % (scalex, scaley, scalep, keepratio))
   
    # if scalep happens to be 100 and qual 75 (the default) and format == input file ending
    # then we simply make a copy of the file so save processing time
    #
    if (scalep == 100 or scalep == -1) and scalex==-1 and scaley==-1 and inputfile[len(inputfile)-3:len(inputfile)] == outputfile[len(outputfile)-3:len(outputfile)]:
        if format == "jpg" and qual != 75: pass
        else:
            import shutil
            # make copy
            LOG( "making a copy of original image")
            try: shutil.copyfile(inputfile, outputfile)
            except IOError: LOG( "Error: copy failed. Abort!"); sys.exit(2)
            sys.exit(0)

    #
    # file open with PIL
    #
    try: image = Image.open(inputfile)
    except IOError:
        LOG( "PIL: failed to open file `%s`. Abort!" % inputfile)
        sys.exit(2)
    pil_format = format
    if format == "jpg": pil_format = "jpeg"


    #
    # Do required manipulations
    #
    image = do_scaling(image, scalex, scaley, scalep, keepratio)


    #
    # Finally save the manipulated image
    #
    try:
        if pil_format != "jpeg":
            image.save(outputfile, pil_format)
        else:
            # Quality affects only JPEG format
            LOG( ("image.save (%s, %s, %d)" % (outputfile, pil_format, qual)))
            image.save(outputfile, pil_format, quality=qual)
    except IOError:
        LOG( "PIL: Image save operation failed! I/O Error.")

    LOG( "PIL: Output written into `%s`" % outputfile)
    LOG( "Done!")
