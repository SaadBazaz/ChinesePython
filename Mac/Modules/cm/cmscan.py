# Scan an Apple header file, generating a Python file of generator calls.

import sys
import os
BGENDIR=os.path.join(sys.prefix, ':Tools:bgen:bgen')
sys.path.append(BGENDIR)
from scantools import Scanner
from bgenlocations import TOOLBOXDIR

LONG = "Components"
SHORT = "cm"

def main():
	input = "Components.h"
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
			#
			# FindNextComponent is a special case, since it call also be called
			# with None as the argument. Hence, we make it a function
			#
			if t == "Component" and m == "InMode" and name != "FindNextComponent":
				classname = "Method"
				listname = "c_methods"
			elif t == "ComponentInstance" and m == "InMode":
				classname = "Method"
				listname = "ci_methods"
		return classname, listname

	def writeinitialdefs(self):
		self.defsfile.write("def FOUR_CHAR_CODE(x): return x\n")

	def makeblacklistnames(self):
		return [
			"OpenADefaultComponent",
			"GetComponentTypeModSeed",
			"OpenAComponentResFile",
			"CallComponentUnregister",
			"CallComponentTarget",
			"CallComponentRegister",
			"CallComponentVersion",
			"CallComponentCanDo",
			"CallComponentClose",
			"CallComponentOpen",
			"OpenAComponent",
			"GetComponentPublicResource", # Missing in CW Pro 6
			"CallComponentGetPublicResource", # Missing in CW Pro 6
			]

	def makegreylist(self):
		return [
			('#if !TARGET_API_MAC_CARBON', [
				'SetComponentInstanceA5',
				'GetComponentInstanceA5',
			])]

	def makeblacklisttypes(self):
		return [
			"ResourceSpec",
			"ComponentResource",
			"ComponentPlatformInfo",
			"ComponentResourceExtension",
			"ComponentPlatformInfoArray",
			"ExtComponentResource",
			"ComponentParameters",
			
			"ComponentRoutineUPP",
			"ComponentMPWorkFunctionUPP",
			"ComponentFunctionUPP",
			"GetMissingComponentResourceUPP",
			]

	def makerepairinstructions(self):
		return [
			([('ComponentDescription', 'looking', 'OutMode')],
			 [('ComponentDescription', '*', 'InMode')]),
			]
			
if __name__ == "__main__":
	main()
