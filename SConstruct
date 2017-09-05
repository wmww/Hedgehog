import os

env = Environment()

obj_file_path = '.obj'
exe_bin_path = 'bin/main'
debug_symbols = True

env.Append(CCFLAGS=[
	'-g' if debug_symbols else None,
	'-Wall',
])

libs = [
	'X11',
	'GL',
	'GLU',
	'EGL',
	'GLEW',
	'SOIL',
	'wayland-server',
]

def get_contents_of_dir(base):
	return [ os.path.abspath(os.path.join(base, i)) for i in os.listdir(base) if not i.startswith('.') ]

def get_subdirs(base):
	return [ i for i in get_contents_of_dir(base) if os.path.isdir(i) ]

def get_all_subdirs(base):
	return [base] + [i for j in get_subdirs(base) for i in get_all_subdirs(j)]

def has_extension(base, extensions):
	if type(extensions) != type([]):
		raise TypeError('has_extension must be sent a path and an array of extensions')
	for extension in extensions:
		if base.endswith(extension):
			return True
	return False

def get_all_files(base):
	return [ path for subdir in get_all_subdirs(base) for path in get_contents_of_dir(subdir) if os.path.isfile(path) ]

def get_all_files_with_extension(base, extensions):
	return [ path for path in get_all_files(base) if has_extension(path, extensions) ]

def get_all_cpp_files():
	return get_all_files_with_extension('.', ['.cpp', '.c'])

sources = get_all_cpp_files()

objects = []

for source in sources:
	#obj_name = '.obj/' + source.path.split('/', 1)[1].rsplit('.', 1)[0] + '.o'
	obj_path = os.path.join(obj_file_path, os.path.relpath(source.rsplit('.', 1)[0] + '.o'))
	obj = env.Object(target=obj_path, source=source)
	objects.append(obj)

program = env.Program(target=exe_bin_path, source=objects, LIBS=libs)
