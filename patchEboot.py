#!/usr/bin/python3

from io import *
import sys

saveLvlPatch0="Lv %d\0\0"
saveLvlPatch1="Lv %d\n\0"
saveLvlOrigin="Lv%02d"
bSaveLvlOrigin=bytes(saveLvlOrigin, 'utf8')

# Patch the eboot's save level formatting
def patchEboot(path):
	try:
		with open(path, 'r+b') as fh:
			print('Patching ' + path + ' ...')
			data = fh.read()
			offset0 = data.find(bSaveLvlOrigin)
			offset1 = data.find(bSaveLvlOrigin, offset0 + len(saveLvlOrigin))

			if offset0 > 1000 and offset1 > 1000:
				print('First offset  : ' + hex(offset0))
				print('Second offset : ' + hex(offset1))

				fh.seek(offset0)
				fh.write(bytes(saveLvlPatch0, 'utf8'))
				fh.seek(offset1)
				fh.write(bytes(saveLvlPatch1, 'utf8'))
				print(path + ' patched')

			else:
				print("File does not require patching")

	except IOError:
		print(path + ' not found')


if __name__ == "__main__":
	patchEboot(sys.argv[1])
