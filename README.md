University project we did for our "Real Time System Programing 2" and "Fundamentals of Computer Networks 1" courses


Platform : Raspberry Pi 2 - Model B 
OS : Raspbian 2017

Client Side:
1)An i2c module on Raspberry Pi(ARM architecture) that reads the input from nunchuk wii joystick.
2)a web client application that gets the read data from the joystick and sends it to the server via Ethernet cable.

Server Side:
3)a server application that recieves data from the client and forwards it to the fake input module
4)fake input module that simulates keyboard input based on data received from the server(joystick action)



