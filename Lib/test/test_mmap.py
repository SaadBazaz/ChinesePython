from test_support import verify
import mmap
import os, re, sys

PAGESIZE = mmap.PAGESIZE

def test_both():
    "Test mmap module on Unix systems and Windows"

    # Create an mmap'ed file
    f = open('foo', 'w+')

    # Write 2 pages worth of data to the file
    f.write('\0'* PAGESIZE)
    f.write('foo')
    f.write('\0'* (PAGESIZE-3) )

    m = mmap.mmap(f.fileno(), 2 * PAGESIZE)
    f.close()

    # Simple sanity checks

    print type(m)  # SF bug 128713:  segfaulted on Linux
    print '  Position of foo:', m.find('foo') / float(PAGESIZE), 'pages'
    verify(m.find('foo') == PAGESIZE)

    print '  Length of file:', len(m) / float(PAGESIZE), 'pages'
    verify(len(m) == 2*PAGESIZE)

    print '  Contents of byte 0:', repr(m[0])
    verify(m[0] == '\0')
    print '  Contents of first 3 bytes:', repr(m[0:3])
    verify(m[0:3] == '\0\0\0')

    # Modify the file's content
    print "\n  Modifying file's content..."
    m[0] = '3'
    m[PAGESIZE +3: PAGESIZE +3+3]='bar'

    # Check that the modification worked
    print '  Contents of byte 0:', repr(m[0])
    verify(m[0] == '3')
    print '  Contents of first 3 bytes:', repr(m[0:3])
    verify(m[0:3] == '3\0\0')
    print '  Contents of second page:',  repr(m[PAGESIZE-1 : PAGESIZE + 7])
    verify(m[PAGESIZE-1 : PAGESIZE + 7] == '\0foobar\0')

    m.flush()

    # Test doing a regular expression match in an mmap'ed file
    match=re.search('[A-Za-z]+', m)
    if match is None:
        print '  ERROR: regex match on mmap failed!'
    else:
        start, end = match.span(0)
        length = end - start

        print '  Regex match on mmap (page start, length of match):',
        print start / float(PAGESIZE), length

        verify(start == PAGESIZE)
        verify(end == PAGESIZE + 6)

    # test seeking around (try to overflow the seek implementation)
    m.seek(0,0)
    print '  Seek to zeroth byte'
    verify(m.tell() == 0)
    m.seek(42,1)
    print '  Seek to 42nd byte'
    verify(m.tell() == 42)
    m.seek(0,2)
    print '  Seek to last byte'
    verify(m.tell() == len(m))

    print '  Try to seek to negative position...'
    try:
        m.seek(-1)
    except ValueError:
        pass
    else:
        verify(0, 'expected a ValueError but did not get it')

    print '  Try to seek beyond end of mmap...'
    try:
        m.seek(1,2)
    except ValueError:
        pass
    else:
        verify(0, 'expected a ValueError but did not get it')

    print '  Try to seek to negative position...'
    try:
        m.seek(-len(m)-1,2)
    except ValueError:
        pass
    else:
        verify(0, 'expected a ValueError but did not get it')

    # Try resizing map
    print '  Attempting resize()'
    try:
        m.resize( 512 )
    except SystemError:
        # resize() not supported
        # No messages are printed, since the output of this test suite
        # would then be different across platforms.
        pass
    else:
        # resize() is supported
        verify(len(m) == 512,
                "len(m) is %d, but expecting 512" % (len(m),) )
        # Check that we can no longer seek beyond the new size.
        try:
            m.seek(513,0)
        except ValueError:
            pass
        else:
            verify(0, 'Could seek beyond the new size')

    m.close()
    os.unlink("foo")
    print ' Test passed'

test_both()
