env = Environment()

env.Append(CCFLAGS='-g')

libs = [
	'X11',
	'GL',
	'GLU',
	'EGL',
	'GLEW',
	'SOIL',
	'wayland-server',
]

def get_all_subdirs(base):
	out = [ i for i in os.listdir(base) if os.path.isdir(os.path.join(base, i)) and name[0] != '.' ]
	out.sort()
	return out

def get_all_subdirs_recursive(base):
	out = get_all_subdirs(base)
	others = []
	for i in out:
		others += get_all_subdirs(i)
	return out + others

def get_all_cpp_files(base):
	paths = get_all_subdirs_recursive(base)
	out = []
	for path in paths:
		for i in os.listdir(path):
			if i.endswith('.cpp'):
				out.append(i)
	return out

sources = get_all_cpp_files('.')

objects = []

for source in sources:
	#obj_name = '.obj/' + source.path.split('/', 1)[1].rsplit('.', 1)[0] + '.o'
	obj_path = os.path.join('.obj', source.path.rsplit('.', 1)[0] + '.o')
	obj = env.Object(target=obj_name, source=source)
	objects.append(obj)

program = env.Program(target='bin/main', source=objects, LIBS=libs)
