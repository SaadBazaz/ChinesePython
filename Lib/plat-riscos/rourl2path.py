"""riscos specific module for conversion between pathnames and URLs.
Based on macurl2path.
Do not import directly, use urllib instead."""

import string
import urllib
import os

def url2pathname(pathname):
    "Convert /-delimited pathname to mac pathname"
    #
    # XXXX The .. handling should be fixed...
    #
    tp = urllib.splittype(pathname)[0]
    if tp and tp <> 'file':
	raise RuntimeError, 'Cannot convert non-local URL to pathname'
    components = string.split(pathname, '/')
    # Remove . and embedded ..
    i = 0
    while i < len(components):
	if components[i] == '.':
	    del components[i]
	elif components[i] == '..' and i > 0 and \
				  components[i-1] not in ('', '..'):
	    del components[i-1:i+1]
	    i = i-1
	elif components[i] == '' and i > 0 and components[i-1] <> '':
	    del components[i]
	else:
	    if components[i]<>'..' and string.find(components[i], '.')<>-1 :
	        components[i] = string.join(string.split(components[i],'.'),'/')
	    i = i+1
    if not components[0]:
	# Absolute unix path, don't start with colon
	return string.join(components[1:], '.')
    else:
	# relative unix path, start with colon. First replace
	# leading .. by empty strings (giving ::file)
	i = 0
	while i < len(components) and components[i] == '..':
	    components[i] = '^'
	    i = i + 1
	return string.join(components, '.')

def pathname2url(pathname):
    "convert mac pathname to /-delimited pathname"
    if '/' in pathname:
	raise RuntimeError, "Cannot convert pathname containing slashes"
    components = string.split(pathname, ':')
    # Replace empty string ('::') by .. (will result in '/../' later)
    for i in range(1, len(components)):
	if components[i] == '':
	    components[i] = '..'
    # Truncate names longer than 31 bytes
    components = map(lambda x: x[:31], components)

    if os.path.isabs(pathname):
	return '/' + string.join(components, '/')
    else:
	return string.join(components, '/')

def test():
    for url in ["index.html",
                "/SCSI::SCSI4/$/Anwendung/Comm/Apps/!Fresco/Welcome",
                "../index.html",
		"bar/index.html",
		"/foo/bar/index.html",
		"/foo/bar/",
		"/"]:
	print `url`, '->', `url2pathname(url)`
    for path in ["drive:",
		 "drive:dir:",
		 "drive:dir:file",
		 "drive:file",
		 "file",
		 ":file",
		 ":dir:",
		 ":dir:file"]:
	print `path`, '->', `pathname2url(path)`

if __name__ == '__main__':
    test()
