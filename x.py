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
			self.serial = serial.Serial(self.port, 9600)
			self.serial.flushInput()
			self.serial.flushOutput()
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
		# op code
		self.sWrite(b"R")
		# address and size
		self.sWrite(struct.pack(">H", self.address))
		self.sWrite(struct.pack(">H", self.size))
		# Read data
		total_to_read = self.size
		data = b""
		while total_to_read != 0:
			# Read block size
			to_read, = struct.unpack("B", self.sRead(1))
			# Read block data
			block_data =  self.sRead(to_read)
			# Read block crc
			crc, = struct.unpack("B", self.sRead(1))
			# Compare CRC and confirm or reject
			comupted_crc = self.crc(block_data)
			if crc != comupted_crc:
				raise XExcpetion("Block CRC doesn't match {}!={}".format(crc, comupted_crc))
			total_to_read -= to_read
			data += block_data
			print("Transfered {} bytes, {} left to read".format(to_read, total_to_read))
		crc, = struct.unpack("B", self.sRead(1))
		end = self.sRead(1)
		comupted_crc = self.crc(data)
		if crc != comupted_crc:
			raise XExcpetion("Data CRC doesn't match {}!={}".format(crc, comupted_crc))
		if end != b"R":
			raise XExcpetion("Unexpected response from bridge")
		open(self.file, "wb").write(data)

	def write(self):
		# op code
		self.sWrite(b"W")
		# address and size
		self.sWrite(struct.pack(">H", self.address))
		self.sWrite(struct.pack(">H", self.size))
		# Write
		data = open(self.file, "rb").read()
		total_to_write = self.size
		while total_to_write != 0:
			# Read block size
			to_write, = struct.unpack("B", self.sRead(1))
			chunk = data[:to_write]
			self.sWrite(chunk)
			self.sWrite(struct.pack("B", self.crc(chunk)))
			ret = self.sRead(1)
			if ret == 'O':
				# Read block crc
				data = data[to_write:]
				total_to_write -= to_write
				print("Transfered {} bytes, {} left to write".format(to_write, total_to_write))
			elif ret == b'C':
				raise XExcpetion("Transfered block CRC Error")
			else:
				raise XExcpetion("Garbage in the stream")

		if self.sRead(1) != b"W":
			raise XExcpetion("Unexpected response from bridge")

	def erase(self):
		# op code
		self.sWrite(b"E")
		# address and size
		self.sWrite(struct.pack(">H", self.address))
		self.sWrite(struct.pack(">H", self.size))
		# confirmation
		if self.sRead(1) != b"E":
			raise XExcpetion("Unexpected response from bridge")

	def sWrite(self, data):
		if self.debug:
			print("serial > {}".format(data))
		self.echo += data
		self.serial.write(data)
		self.serial.flush()

	def sRead(self, size):
		echo_len = len(self.echo)
		if echo_len > 0:
			data = self.serial.read(size + echo_len)
			if self.debug:
				print("serial < {}".format(data))
			if data[:echo_len] != self.echo:
				raise Exception("Unexpected data in the stream: was expecting {}, got {} ".format(self.echo, data))
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
	parser.add_argument("command", choices=["read", "write", "erase"], help="Command")
	parser.add_argument("address", type=int, help="Memory address to read or write")
	parser.add_argument("size", type=int, help="Size to read or write")
	parser.add_argument("file", help="File to write or read")
	args = parser.parse_args()
	X(args)()
