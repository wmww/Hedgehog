env = Environment()

sources = Glob('src/*.cpp')

libs = [
	'X11',
	'GL',
	'GLU',
	'GLEW',
	'SOIL',
]

objects = []

for source in sources:
	obj_name = 'obj/' + source.path.split('/', 1)[1].rsplit('.', 1)[0] + '.o'
	obj = env.Object(target=obj_name, source=source)
	objects.append(obj)

program = env.Program(target='bin/main', source=objects, LIBS=libs)
