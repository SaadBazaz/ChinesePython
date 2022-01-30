from preferences import *

# Names of Python resources
PREFNAME_NAME="PythonPreferenceFileName"

# Resource IDs in the preferences file
PATH_ID = 228
DIR_ID = 228
POPT_ID = 228
GUSI_ID = 10240

# Override IDs (in the applet)
OVERRIDE_PATH_ID = 229
OVERRIDE_DIR_ID = 229
OVERRIDE_POPT_ID = 229
OVERRIDE_GUSI_ID = 10241

# version
CUR_VERSION=7

preffilename = PstringLoader(AnyResLoader('STR ', resname=PREFNAME_NAME)).load()
pref_fss = preferencefile(preffilename, 'Pyth', 'pref')

class PoptLoader(VersionLoader):
	def __init__(self, loader):
		VersionLoader.__init__(self, "bbbbbbbbbbbbbb", loader)
		
	def versioncheck(self, data):
		if data[0] == CUR_VERSION:
			return data
		print 'old resource'
		raise Error, "old resource"
		
class GusiLoader:
	def __init__(self, loader):
		self.loader = loader
		self.data = None
		
	def load(self):
		self.data = self.loader.load()
		while self.data[10:14] != '0181':
			self.loader.delete(1)
			self.loader.load()
		tp = self.data[0:4]
		cr = self.data[4:8]
		flags = ord(self.data[9])
		return cr, tp
		
	def save(self, (cr, tp)):
		flags = ord(self.data[9])
		newdata = tp + cr + self.data[8:]
		self.loader.save(newdata)
		
popt_default_default = NullLoader(chr(CUR_VERSION) + 8*'\0')
popt_default = AnyResLoader('Popt', POPT_ID, default=popt_default_default)
popt_loader = ResLoader(pref_fss, 'Popt', POPT_ID, default=popt_default)
popt = PoptLoader(popt_loader)

dir_default = AnyResLoader('alis', DIR_ID)
dir = ResLoader(pref_fss, 'alis', DIR_ID, default=dir_default)

gusi_default = AnyResLoader('GU\267I', GUSI_ID)
gusi_loader = ResLoader(pref_fss, 'GU\267I', GUSI_ID, default=gusi_default)
gusi = GusiLoader(gusi_loader)

path_default = AnyResLoader('STR#', PATH_ID)
path_loader = ResLoader(pref_fss, 'STR#', PATH_ID, default=path_default)
path = StrListLoader(path_loader)

class PythonOptions:
	def __init__(self, popt=popt, dir=dir, gusi=gusi, path=path):
		self.popt = popt
		self.dir = dir
		self.gusi = gusi
		self.path = path
		
	def load(self):
		dict = {}
		dict['path'] = self.path.load()
		diralias = self.dir.load()
		dirfss, dummy = macfs.RawAlias(diralias).Resolve()
		dict['dir'] = dirfss
		dict['creator'], dict['type'] = self.gusi.load()
		flags = self.popt.load()
		dict['version'], dict['inspect'], dict['verbose'], dict['optimize'], \
			dict['unbuffered'], dict['debugging'], dummy, dict['keep_console'], \
			dict['nointopt'], dict['noargs'], dict['tabwarn'], \
			dict['nosite'], dict['nonavservice'], dict['delayconsole'] = flags
		return dict
		
	def save(self, dict):
		self.path.save(dict['path'])
		diralias = macfs.FSSpec(dict['dir']).NewAlias().data
		self.dir.save(diralias)
		self.gusi.save((dict['creator'], dict['type']))
		flags = dict['version'], dict['inspect'], dict['verbose'], dict['optimize'], \
			dict['unbuffered'], dict['debugging'], 0, dict['keep_console'], \
			dict['nointopt'], dict['noargs'], dict['tabwarn'], \
			dict['nosite'], dict['nonavservice'], dict['delayconsole']
		self.popt.save(flags)

def AppletOptions(file):
	fss = macfs.FSSpec(file)
	a_popt = PoptLoader(ResLoader(fss, 'Popt', OVERRIDE_POPT_ID, default=popt_loader))
	a_dir = ResLoader(fss, 'alis', OVERRIDE_DIR_ID, default=dir)
	a_gusi = GusiLoader(
			ResLoader(fss, 'GU\267I', OVERRIDE_GUSI_ID, default=gusi_loader))
	a_path = StrListLoader(
			ResLoader(fss, 'STR#', OVERRIDE_PATH_ID, default=path_loader))
	return PythonOptions(a_popt, a_dir, a_gusi, a_path)
	
def _test():
	import preferences
	preferences.debug = 1
	dict = PythonOptions().load()
	for k in dict.keys():
		print k, '\t', dict[k]
		
if __name__ == '__main__':
	_test()
	
