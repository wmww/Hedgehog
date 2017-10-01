#! /bin/python3
print('generating code from xml protocols')
from os import path, listdir;
from sys import argv;
from subprocess import run;
if len(argv) < 2:
	print('please specify the path for the protocols')
	exit(1)
elif len(argv) > 2:
	print('too many arguments')
base_path = argv[1]
xml_files = [ path.join(base_path, xml) for xml in listdir(base_path) if xml.endswith('.xml') ]
for xml in xml_files:
	run(['wayland-scanner', '--include-core-only', 'server-header', xml, xml.rsplit('.', 1)[0] + '.h'])
	run(['wayland-scanner', '--include-core-only', 'code', xml, xml.rsplit('.', 1)[0] + '.c'])
