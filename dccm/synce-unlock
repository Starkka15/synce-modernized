#!/usr/bin/env python

# A simple script to provide a password to a locked Windows
# Mobile device from the command line

import dbus
import os
import sys
import getopt

DBUS_DBUS_BUSNAME	= "org.freedesktop.DBus"
DBUS_DBUS_IFACE		= "org.freedesktop.DBus"
DBUS_DBUS_OBJPATH	= "/org/freedesktop/DBus"

DBUS_DCCM_BUSNAME         = "org.synce.dccm"
DBUS_DCCM_MANAGER_IFACE   = "org.synce.dccm.DeviceManager"
DBUS_DCCM_MANAGER_OBJPATH = "/org/synce/dccm/DeviceManager"
DBUS_DCCM_DEVICE_IFACE    = "org.synce.dccm.Device"

DCCM_DEVICE_PASSWORD_FLAG_UNSET             = "unset"
DCCM_DEVICE_PASSWORD_FLAG_PROVIDE           = "provide"
DCCM_DEVICE_PASSWORD_FLAG_PROVIDE_ON_DEVICE = "provide-on-device"
DCCM_DEVICE_PASSWORD_FLAG_CHECKING          = "checking"
DCCM_DEVICE_PASSWORD_FLAG_UNLOCKED          = "unlocked"

# returns
# 0 unlocked
# 1 needs a password
# 2 other problem

def unlock_device(device_path, password):

	device = dbus.Interface(dbus.SystemBus().get_object(DBUS_DCCM_BUSNAME, device_path), DBUS_DCCM_DEVICE_IFACE)

	deviceName = device.GetName()
	print "Unlocking device connected at path %s with name %s" % (device_path, deviceName)

	pw_flags = device.GetPasswordFlags()
	if pw_flags == DCCM_DEVICE_PASSWORD_FLAG_UNSET or pw_flags == DCCM_DEVICE_PASSWORD_FLAG_UNLOCKED:
		print "No password required"
		return 0

	if pw_flags == DCCM_DEVICE_PASSWORD_FLAG_PROVIDE_ON_DEVICE:
		print "WM6 device requires password on device"
		return 2

	if pw_flags == DCCM_DEVICE_PASSWORD_FLAG_CHECKING:
		print "aError: an authorization attempt is already in progress"
		return 2

	print "Authorization required for device %s." % deviceName

	rc = device.ProvidePassword(password)
	if (rc == True):
		print "Authorization succeeded"
		return 0

	print "Authorization failed"
	return 1


def usage():
	print "Usage: %s [ -d <device-name> ] [ -p ] [ <password> ]" % sys.argv[0]


if __name__ == "__main__":

	try:
		opts, args = getopt.getopt(sys.argv[1:], "d:p", ["device-name="])

	except getopt.GetoptError:
		usage()
		sys.exit(2)

	device_name = None
	password_prompt = False
	password = None

	for o, a in opts:
		if o in ("-d", "--device-name"):
			device_name = a
		elif o == "-p":
			password_prompt = True
			password = raw_input("Enter password: ")
		else:
			usage()
			sys.exit(2)

	if len(args) > 1:
		print "Too many arguments"
		usage()
		sys.exit(2)

	if password_prompt == True and len(args) > 0:
		print "Too many arguments"
		usage()
		sys.exit(2)

	if password_prompt == False and len(args) < 1:
		usage()
		sys.exit(2)

	if password == None:
		password = args[0]

	device_manager = dbus.Interface(dbus.SystemBus().get_object(DBUS_DCCM_BUSNAME, DBUS_DCCM_MANAGER_OBJPATH), DBUS_DCCM_MANAGER_IFACE)
	obj_paths = device_manager.GetConnectedDevices()
	if len(obj_paths) == 0:
		print "No connected devices found"
		sys.exit(2)

	found_device = None
	for device_path in obj_paths:
		device = dbus.Interface(dbus.SystemBus().get_object(DBUS_DCCM_BUSNAME, device_path), DBUS_DCCM_DEVICE_IFACE)

		if device_name == None:
			found_device = device_path
			break

		deviceName = device.GetName()
		if deviceName == device_name:
			found_device = device_path
			break

	if found_device == None:
		print "Device not found"
		sys.exit(2)

	rc = unlock_device(found_device, password)

	sys.exit(rc)
