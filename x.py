#!/usr/bin/env python3

import argparse

class X(object):
	def __init__(self, args):
		super(X, self).__init__()
		self.args = args
	
	def __call__(self):
		print("{0.command} memory starting at {0.address}, size {0.size}".format(self.args))
		getattr(self, self.args.command)()

	def read(self):
		print("Done")

	def write(self):
		print("Done")

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Xfer tool")
	parser.add_argument("--port", help="Serial port")
	parser.add_argument("command", choices=["read", "write"], help="Command")
	parser.add_argument("address", help="Memory address to read or write")
	parser.add_argument("size", help="Size to read or write")
	parser.add_argument("file", help="File to write or read")
	args = parser.parse_args()
	X(args)()