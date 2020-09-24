import os
import sys
import pathlib
import re

try:
	name = str(sys.argv[1])
except IndexError:
	name = input('File name: ')

if pathlib.Path(name).exists() and os.stat(name).st_size: # Chech if the file is there and if not empty, in which case prompt whether to overwrite it
	if input('File exists and is not empty, overwrite it? [no] ') == 'y' or 'Y' or 'yes' or 'Yes':
		print('Overwriting file.\n')
	else:
		print('Not overwriting file.')
		exit()

with open(name, 'wb+') as file:
	symbol = name.replace('include/', '').upper().replace('.', '_').replace('/', '_')
	contents = bytes(F"#pragma once\n\n#ifndef {symbol}\n#define {symbol}\n\n#endif\n", encoding='utf8')
	file.write(contents)
