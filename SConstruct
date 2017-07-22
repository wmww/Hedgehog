env = Environment()
Export('env')
objects = SConscript('src/SConscript')
libs = [
	'X11',
	'GL',
	'GLU',
	'GLEW',
	'SOIL',
]
program = env.Program(target='bin/main', source=objects, LIBS=libs)
