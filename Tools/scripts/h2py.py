#! /usr/bin/env cpython

# Read #define's and translate to Python code.
# Handle #include statements.
# Handle #define macros with one argument.
# Anything that isn't recognized or doesn't translate into valid
# Python is ignored.

# Without filename arguments, acts as a filter.
# If one or more filenames are given, output is written to corresponding
# filenames in the local directory, translated to all uppercase, with
# the extension replaced by ".py".

# By passing one or more options of the form "-i regular_expression"
# you can specify additional strings to be ignored.  This is useful
# e.g. to ignore casts to u_long: simply specify "-i '(u_long)'".

# XXX To do:
# - turn trailing C comments into Python comments
# - turn C Boolean operators "&& || !" into Python "and or not"
# - what to do about #if(def)?
# - what to do about macros with multiple parameters?

import sys, regex, regsub, string, getopt, os

p_define = regex.compile('^[\t ]*#[\t ]*define[\t ]+\([a-zA-Z0-9_]+\)[\t ]+')

p_macro = regex.compile(
  '^[\t ]*#[\t ]*define[\t ]+'
  '\([a-zA-Z0-9_]+\)(\([_a-zA-Z][_a-zA-Z0-9]*\))[\t ]+')

p_include = regex.compile('^[\t ]*#[\t ]*include[\t ]+<\([a-zA-Z0-9_/\.]+\)')

p_comment = regex.compile('/\*\([^*]+\|\*+[^/]\)*\(\*+/\)?')
p_cpp_comment = regex.compile('//.*')

ignores = [p_comment, p_cpp_comment]

p_char = regex.compile("'\(\\\\.[^\\\\]*\|[^\\\\]\)'")

filedict = {}

try:
    searchdirs=string.splitfields(os.environ['include'],';')
except KeyError:
    try:
        searchdirs=string.splitfields(os.environ['INCLUDE'],';')
    except KeyError:
        try:
            if string.find( sys.platform, "beos" ) == 0:
                searchdirs=string.splitfields(os.environ['BEINCLUDES'],';')
            else:
                raise KeyError
        except KeyError:
            searchdirs=['/usr/include']

def main():
    global filedict
    opts, args = getopt.getopt(sys.argv[1:], 'i:')
    for o, a in opts:
        if o == '-i':
            ignores.append(regex.compile(a))
    if not args:
        args = ['-']
    for filename in args:
        if filename == '-':
            sys.stdout.write('# Generated by h2py from stdin\n')
            process(sys.stdin, sys.stdout)
        else:
            fp = open(filename, 'r')
            outfile = os.path.basename(filename)
            i = string.rfind(outfile, '.')
            if i > 0: outfile = outfile[:i]
            outfile = string.upper(outfile)
            outfile = outfile + '.py'
            outfp = open(outfile, 'w')
            outfp.write('# Generated by h2py from %s\n' % filename)
            filedict = {}
            for dir in searchdirs:
                if filename[:len(dir)] == dir:
                    filedict[filename[len(dir)+1:]] = None  # no '/' trailing
                    break
            process(fp, outfp)
            outfp.close()
            fp.close()

def process(fp, outfp, env = {}):
    lineno = 0
    while 1:
        line = fp.readline()
        if not line: break
        lineno = lineno + 1
        n = p_define.match(line)
        if n >= 0:
            # gobble up continuation lines
            while line[-2:] == '\\\n':
                nextline = fp.readline()
                if not nextline: break
                lineno = lineno + 1
                line = line + nextline
            name = p_define.group(1)
            body = line[n:]
            # replace ignored patterns by spaces
            for p in ignores:
                body = regsub.gsub(p, ' ', body)
            # replace char literals by ord(...)
            body = regsub.gsub(p_char, 'ord(\\0)', body)
            stmt = '%s = %s\n' % (name, string.strip(body))
            ok = 0
            try:
                exec stmt in env
            except:
                sys.stderr.write('Skipping: %s' % stmt)
            else:
                outfp.write(stmt)
        n =p_macro.match(line)
        if n >= 0:
            macro, arg = p_macro.group(1, 2)
            body = line[n:]
            for p in ignores:
                body = regsub.gsub(p, ' ', body)
            body = regsub.gsub(p_char, 'ord(\\0)', body)
            stmt = 'def %s(%s): return %s\n' % (macro, arg, body)
            try:
                exec stmt in env
            except:
                sys.stderr.write('Skipping: %s' % stmt)
            else:
                outfp.write(stmt)
        if p_include.match(line) >= 0:
            regs = p_include.regs
            a, b = regs[1]
            filename = line[a:b]
            if not filedict.has_key(filename):
                filedict[filename] = None
                inclfp = None
                for dir in searchdirs:
                    try:
                        inclfp = open(dir + '/' + filename, 'r')
                        break
                    except IOError:
                        pass
                if inclfp:
                    outfp.write(
                            '\n# Included from %s\n' % filename)
                    process(inclfp, outfp, env)
                else:
                    sys.stderr.write('Warning - could not find file %s' % filename)

main()
