import time 
import os
import simplepam	
from uuid import uuid4
import serial
import syslog
import subprocess

port = '/dev/ttyACM0'
print("port: ",port)
ard = None

if ard is None:
	try:
		print("acm2")
		ard = serial.Serial('/dev/ttyACM2',9600,timeout=5)
	except:
		ard = None

if ard is None:
	try:
		print("acm0")		
		ard = serial.Serial('/dev/ttyACM0',9600,timeout=5)
	except:
		ard = None

if ard is None:
	try:
		print("acm1")		
		ard = serial.Serial('/dev/ttyACM1',9600,timeout=5)
	except:
		ard = None


table = {
	"djangoman":{
		"password":"iwbibz112agp",
		"token_usb":"a74333b0-1b9a-4c9b-a068-b0274179a595",
		"token_arduino":"7705b351-d3af-4063-aa4f-9e2cb83248f9"
	}
}

asd = open("tokens",'r')
datas = [line.strip() for line in asd]
asd.close()


user = "djangoman"
table[user]['token_usb'] = datas[0]
table[user]['token_arduino'] = datas[1]

print("table: ", table)
print("user :", user)

def lock():
	print("locked...")
	os.system("gnome-screensaver-command -l")

def unlock():
	print("unlocked...")
	os.system("loginctl unlock-session")

def auth(user1, token_from_usb, token_from_arduino):
	if user1 in table:
		print(token_from_usb.strip(),table[user1]['token_usb'],token_from_arduino.strip(),table[user1]['token_arduino'])
		if token_from_usb.strip() == table[user1]['token_usb'] and token_from_arduino.strip() == table[user1]['token_arduino']: 
			print("auth success...")
			return True

	print("auth false...")
	return False



def usb_available():
	return True
	try:
		x = check_output(['lsusb','-d','8564:1000'])
		print("usb available...")		
	except:
		print("usb doesn't available...")				
		return False
		

run = True
lock_screen = False


	
	
try:
	while run:
		x11 = subprocess.check_output(['qdbus','org.gnome.ScreenSaver','/com/canonical/Unity/Session','com.canonical.Unity.Session.IsLocked']).decode()
		ard = None

		if ard is None:
			try:
				ard = serial.Serial('/dev/ttyACM2',9600,timeout=5)
			except:
				ard = None

		if ard is None:
			try:
				ard = serial.Serial('/dev/ttyACM0',9600,timeout=5)
			except:
				ard = None

		if ard is None:
			try:
				ard = serial.Serial('/dev/ttyACM1',9600,timeout=5)
			except:
				ard = None



		ard.flush()
		# print(ard)

		e = 1
		while e:
			ard.write('1'.encode())
			time.sleep(0.1)
			ans = ard.readline().decode('ascii').strip()
			if ans != '':
				e = 0


		if x11.strip() == "true" and not lock_screen:
			lock_screen = True
			ard.write('-L'.encode())
		elif x11.strip() == "false" and lock_screen:
			lock()
			ard.write('-L'.encode())
			

		if not usb_available() and not lock_screen:
			lock_screen = True
			ard.write('-L'.encode())
			lock()
			continue 

		elif not usb_available():
			continue


		time.sleep(0.1)
		token_u = None
		token_a = None


		ard.write('1'.encode())
		time.sleep(0.1)
		token_a = ard.readline().decode('ascii').strip()
		time.sleep(0.1)

		if token_a == "NO" and not lock_screen:
			lock()
			lock_screen = True
			continue

	
		token_u = subprocess.check_output(['cat','/home/djangoman/token']).decode('ascii').strip()
		access = auth("djangoman", token_u, token_a)


		if access and lock_screen:
			unlock()
			lock_screen = False
			table[user]['token_usb'] = str(uuid4())
			table[user]['token_arduino'] = str(uuid4())
			os.system("echo '"+table[user]['token_usb']+"' >/home/djangoman/token")
			ard.write(('2'+table[user]['token_arduino']).encode())
			time.sleep(0.1)			
			ard.write('-L'.encode())
			datas[0] = table[user]['token_usb']
			datas[1] = table[user]['token_arduino']
			wrt = open("tokens",'w')
			wrt.write('\n'.join(datas))
			wrt.close()


except Exception as ee:
	print(ee)
	print(datas)
	wrt = open("tokens",'w')
	wrt.write('\n'.join(datas))
	wrt.close()
