total = ''
for i in range(0xa0, 0xff):
   ch1 = chr(i)
   for j in range(0x40, 0x7f):
	ch2 = chr(j)
	#total = total+ch1+ch2 + '\t\n' 
	total = total+ch1+ch2 
   for j in range(0xa1, 0xff):
	ch2 = chr(j)
	#total = total + ch1 + ch2 + '\t\n'
	total = total + ch1 + ch2
print total
