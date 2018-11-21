#!/usr/bin/env python3

import argparse
import serial
import struct

class XExcpetion(Exception):
	pass

class X(object):
	def __init__(self, args):
		super(X, self).__init__()
		self.serial = None
		self.port = args.port
		self.command = args.command
		self.address = args.address
		self.size = args.size
		self.file = args.file
		self.debug = args.debug
		self.echo = b""

	def __call__(self):
		# Validate address and size
		if self.address < 0 or self.address >= 0x2000:
			print("Invalid args: address must be in between 0x0000 and 0x2000")
			return
		if self.size < 1 or self.size > 0x2000:
			print("Invalid args: size must be in between 0x0001 and 0x2000")
			return
		if self.address + self.size > 0x2000:
			print("Invalid args: sum of address and size must be less than 0x2000")
			return
		# Open Serial port
		try:
			self.serial = serial.Serial(self.port)
		except serial.SerialException as e:
			print("Failed to open serial port. {}".format(e))
			return
		# Ping
		self.sWrite(b"P")
		if self.sRead(1) == b"P":
			print("Bridge is alive")
		else:
			print("No link or bridge is dead")
			return
		print("Executing {0.command} memory starting at {0.address}, size {0.size}".format(self))
		try:
			getattr(self, self.command)()
			print("Completed {0.command} memory starting at {0.address}, size {0.size}".format(self))
		except XExcpetion as e:
			print("Failure: {}".format(e))

	def read(self):
		self.sWrite(b"R")
		self.sWrite(struct.pack(">H", self.address))
		self.sWrite(struct.pack(">H", self.size))
		data = self.sRead(self.size)
		crc, = struct.unpack("B", self.sRead(1))
		end = self.sRead(1)
		comupted_crc = self.crc(data)
		if crc != comupted_crc:
			raise XExcpetion("CRC doesn't match {}!={}".format(crc, comupted_crc))
		if end != b"R":
			raise XExcpetion("Unexpected response from bridge")
		open(self.file, "wb").write(data)

	def write(self):
		self.sWrite(b"W")
		self.sWrite(struct.pack(">H", self.address))
		self.sWrite(struct.pack(">H", self.size))
		data = self.sRead(size)
		if self.sRead(1) != b"W":
			raise XExcpetion("Unexpected response from bridge")

	def sWrite(self, data):
		if self.debug:
			print("serial > {}".format(data))
		self.echo += data
		self.serial.write(data)

	def sRead(self, size):
		echo_len = len(self.echo)
		if echo_len > 0:
			data = self.serial.read(size + echo_len)
			if self.debug:
				print("serial < {}".format(data))
			if data[:echo_len] != self.echo:
				raise Exception("Unexpected data in the stream")
			self.echo = b""
			return data[echo_len:]
		else:
			data = self.serial.read(size)
			if self.debug:
				print("serial < {}".format(data))
			return data

	def crc(self, data):
		crc = 0
		for i in data:
			crc += i
			crc &= 0xFF
		return crc

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Xfer tool")
	parser.add_argument("-d", "--debug", action='store_true', help="Debug")
	parser.add_argument("--port", help="Serial port")
	parser.add_argument("command", choices=["read", "write"], help="Command")
	parser.add_argument("address", type=int, help="Memory address to read or write")
	parser.add_argument("size", type=int, help="Size to read or write")
	parser.add_argument("file", help="File to write or read")
	args = parser.parse_args()
	X(args)()
