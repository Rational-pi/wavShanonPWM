import sys
def main(argv):
	if len(argv)<2:
		print "usage : add_class.py ClassName [Namespace]"
		print "you typed :",argv
	elif len(argv)>=2:
		fileNames=[
		"include/{}.h".format(argv[1]),
		"src/{}.cpp".format(argv[1])
		]

		#chk if the file are existing
		for fileName in fileNames:
			try:
				file=open(fileName, 'r')

				if [file.readline()]!=['']:
					print "this class is existing or src/include dir are not existing!"
					return
			except:pass#not existing

		#creating them
		try:fileListe=[open(fileNames[0], 'w'),open(fileNames[1], 'w')]
		except:
			print "src/include dir are not existing!"
			return

		className=argv[1][0].upper()+argv[1][1:]
		fileListe[0].write(					'#ifndef {}_H\n'		.format(argv[1].upper())		)
		fileListe[0].write(					'#define {}_H\n'		.format(argv[1].upper())		)
		fileListe[0].write(					'\n'													)
		if len(argv)>2:fileListe[0].write(	'namespace {0} {1}\n\n'  	.format(argv[2],'{')				)
		fileListe[0].write(					'class {0}\n{1}\npublic:\n  {0}();\n{2};\n'.format(className,'{','}')	)
		if len(argv)>2:fileListe[0].write(	'\n}\n'													)
		fileListe[0].write(					'\n#endif'				.format(argv[1].upper())		)


		fileListe[1].write(					'#include "{}.h"\n'		.format(argv[1])				)
		if len(argv)>2:fileListe[1].write(	'\nnamespace {} {}\n'  	.format(argv[2],'{')				)
		fileListe[1].write(					'\n{0}::{0}(){1}\n\n{2}\n'		.format(className,'{','}')				)
		if len(argv)>2:fileListe[1].write(	'\n}'														)


		for file in fileListe: file.close()
		print "Remember to add {} to the project files".format(fileNames[1])



if __name__ == '__main__':
	main(sys.argv)