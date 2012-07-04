Knock-Lock
==========

Arduino sketch to allow users to open a door using a 'secret' knock/tap sequence. The project uses capacitive touch and so the door handle becomes the electrode which the user taps their hand against. Access control is provided in the same manner as the RFID-Lock Project (https://github.com/robhemsley/RFID-Door-Lock).  
The code has been written for use with the MIT Media Lab door lock created by Valentin Heun (http://colorsaregood.com/door/)  

## Installation ##

* Install the following Library Dependencies:  
	CapSense - http://www.arduino.cc/playground/Main/CapSense  
	SoftwareSerial - http://arduino.cc/hu/Reference/SoftwareSerial  
	Servo - http://arduino.cc/it/Reference/Servo  

* Restart Arduino IDE
* Open KNOCK_LOCK.ino
* Set the following variables between lines 7-10:  
	SERVO_PIN 		- The logic pin the servo is connected to  
	CAP_SWITCH_SEND		- The capacitive signal send pin  
	CAP_SWITCH_RECIEVE	- The capacitive signal receive pin  
	ADMIN_TIMEOUT		- The number of milliseconds till admin mode starts  

* Users can be managed via the serial monitor (Type h for cmd listing)

hello at robhemsley.co.uk
