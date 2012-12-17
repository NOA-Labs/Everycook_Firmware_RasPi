WebSite Part:
1.Build a LAMP sever in Linux system. 
  (Lnk: http://www.penguintutor.com/linux/raspberrypi-webserver)  
   //Here you can know how to configurate your server
  You need Download :
  Apache - websever software (support php)
  PHP5   - help to explain the php language

  If everything done, you can test to view the page: http://localhost, there will got a default page which
  can tell you the configuration was correct.

2.Copy web file
  <Attention:  Copy the file 'www' to /var, replaced the '/var/www' file>

C Code Part
1.That code was base on the lib "wiringPI", so you need install that lib frist.
  (Lnk: https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/)
  //Here you can know how to install the "wiringPI" library and build your lib in linux.
  Than your can use the C Code in file "everycook", the makefile was wrote already, you can use the command "make"
  to rebuild the program. Which named "all", than use the commond "sudo ./all" to run the program.
  
And please change the file limits of authority. Like: readfile.txt , writefile.txt need to change to 664.
Also the ohter file in the /var/www need takecare of the authority.

<Please use the firefox browser, my ajax code now just running well in the firefox browser>

[2012-12-13]
Please read the 'Change.txt'.A litte change just for the configuration register.
This will be more easier for you to change the IN-AMP of the AD7794 for every channel.

First LED Line:
LED1 = Small Power
LED5 = Middle Power
LED3 = Max Power

Second LED Line:
LED4 = Normal Cooking (here you can change between different power levels)
LED6 = Keep Warm (here it is always the smallest power Level)
LED2 = Power Indicator (always on when the device is on)

Button Line:
Button1 = Mode (switching between cooking mode and keeping warm)
Button2 = Power Down (works only in cooking mode)
Button3 = Power Up (works only in cooking mode)
Button4 = On/Off Button