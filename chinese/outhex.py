line = open('allb5.done').read().split('\t\n')
print len(line)
print 'start'
print line[627]
print line[628]
print line[629]
print 'done'
for a in line: 
    if len(a) == 2:
    	ch1 = a[0]
    	ch2 = a[1]
    	print '"\\'+hex(ord(ch1))[1:] + '\\' + hex(ord(ch2))[1:] + '",'
    else:
	print '"\\x20\\x20",'
