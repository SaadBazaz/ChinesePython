# Scan an Apple header file, generating a Python file of generator calls.

import sys
import os
BGENDIR=os.path.join(sys.prefix, ':Tools:bgen:bgen')
sys.path.append(BGENDIR)
from scantools import Scanner
from bgenlocations import TOOLBOXDIR

LONG = "Icons"
SHORT = "icn"
OBJECT = "NOTUSED"

def main():
	input = LONG + ".h"
	output = SHORT + "gen.py"
	defsoutput = TOOLBOXDIR + LONG + ".py"
	scanner = MyScanner(input, output, defsoutput)
	scanner.scan()
	scanner.close()
	print "=== Done scanning and generating, now importing the generated code... ==="
	exec "import " + SHORT + "support"
	print "=== Done.  It's up to you to compile it now! ==="

class MyScanner(Scanner):

	def destination(self, type, name, arglist):
		classname = "Function"
		listname = "functions"
		if arglist:
			t, n, m = arglist[0]
			# This is non-functional today
			if t == OBJECT and m == "InMode":
				classname = "Method"
				listname = "methods"
		return classname, listname

	def makeblacklistnames(self):
		return [
			"GetIconCacheData",
			"SetIconCacheData",
			# Constants with funny definitions
			"kSelectorAllHugeData",
			"kSelectorAllAvailableData",
			"svAllAvailableData",
			# Something in a comment accidentally seen as a const definition
			"err",
			]

	def makegreylist(self):
		return [
			('#if !TARGET_API_MAC_CARBON', [
				'IconServicesTerminate',
			]),
			('#if TARGET_API_MAC_CARBON', [
				'WriteIconFile',
				'ReadIconFile',
				'RegisterIconRefFromIconFile',
				'GetIconRefVariant',
			])]

	def makeblacklisttypes(self):
		return [
			"IconActionUPP",
			"IconGetterUPP",
			"CFragInitBlockPtr",
			]

	def makerepairinstructions(self):
		return [
			]

	def writeinitialdefs(self):
		self.defsfile.write("def FOUR_CHAR_CODE(x): return x\n")
			
if __name__ == "__main__":
	main()
