# expanseElectronics soloWiFi

![3rd Generation Node](https://user-images.githubusercontent.com/63847434/156920937-ab59d553-d830-4a00-8578-9451ea7b3224.jpg)

### **About:**
As a lighting technician, I often have to run cables across walkways, moving props, or only awkward places to run a wire. Wouldn't it be nice to have a wireless solution? While there are many commercial units available using various protocols, they are either expensive or unreliable. With our product, you could have any show set up within hours! With its slim and compact design, it fits right in your pocket! With the implementation of Wi-Fi, there is no need to worry about cables. Stored scenes allow for instant playback through the web interface with nearly no latency. 

### **Some demos:**
Before selling these on Tindie, I have used them in many productions thus far. Most recently on a Cambridge Theatre Company production (Calendar Girls) in my local school's theatre ([Leys Great Hall](https://www.theleys.net/591/venue-hire/great-hall)) where we used two of our nodes to control some Robe T1s that were mounted above the circle balcony (no dmx signal there). It took us only five minutes to have both nodes talk to each other and patched them into Artnet. The show went smoothly!

![Photograph of 'Chicago' by CTC, performed at Leys Great Hall.](https://alexanderhoppe.com/images/20221026_121229605_iOS.jpg "Photograph of 'Chicago' by CTC, performed at Leys Great Hall.")

*Photograph of 'Chicago' by CTC, performed at Leys Great Hall.*

### Credits:
A lot of this project is forked from Matt's ESP8266 based WiFi ArtNet to DMX, RDM and LED Pixels project found [here](https://github.com/mtongnz/ESP8266_ArtNetNode_v2). We thank Matt as a team for creating his amazing project with all it's features. 

# Instructions
### **General Info:**
**-	First Boot**
On your first boot, the device will start a hotspot called "expanseNode", followed by the node's ID (which looks something like "00178"). The hotspot should be created without a password; however, the default password is  "expanse2021" (case sensitive) if it is unable to do so. 
Once you have connected to the node, log in to the hotspot and head to the IP address: 2.0.0.1 in a web browser.

**-	Web UI**
If used in conjunction with another network: Enter your desired SSID and password in the Wi-Fi tab. Click save (the button should change to green and say "Settings Saved"). Now click "Reboot" in the side menu, and the device should reboot and connect to your desired Wi-Fi network.
If used alone or as a master connection for multiple modes: In the Wi-Fi tab, head over to the bottom of the tab and check the "Stand Alone" checkbox. Click save (the button should change to green and say "Settings Saved"). Now click "Reboot" in the side menu, and the device should reboot and begin it's hotspot once again, but this time will allow both input and output via the hotspot. 
Extra help or notes: If the device can't connect to the Wi-Fi or is unable to be assigned a DHCP assigned address within 15 seconds, it will start the setup hotspot once again and wait for 30 seconds for you to connect. If a client doesn't connect to the hotspot in time, the device will restart and try again.

**-	Configuring DMX Ports**
Everything is customisable via the web, head over to the "Port A" tab. If you will be using the node as an output device, select "DMX Output with RDM" or "DMX Output", and then choose which universe and subnet the node should listen on. If you are using the node as an input device, select "DMX Input" from the dropdown list. Change the settings below according to which universe/subnet the node should send the data to. Make sure to correct the "Broadcast Address" to whatever your console/visualiser is listening to. The default is "2.255.255.255"; however, many programs listen to "255.255.255.255" instead. 

**-	Customising Node name and DHCP / STATIC settings**
Head over to the "IP & Name" tab. Here you will be able to change the "short name" (hotspot SSID) and "long name" (Artnet broadcasted name). The settings that follow are for DHCP or STATIC IP assignments. If the node should connect to a network using a STATIC IP address, make sure to un-check the "DHCP" checkbox and then configure the IP, Subnet Address, and Gateway Address accordingly. 

### **Status LED Indicators:**
The first LED (the one closest to the micro-controller) is the main status indicator, and the two nearer to the DMX connector are the Port Status LEDs. 
-	Main Status Indicator:
PINK – The node is still booting / configuration network is being hosted. 
GREEN flashing – Node status is OK; there are no errors. 
RED flashing – There is an internal error. A WDT reset generally causes this, and the error code will be sent over Artnet. 
-	Port Status Indicators:
PURPLE – Port Initialised in DMX Output mode, but nothing is being out-putted. 
BLUE – DMX received from Artnet, and the output is active. 
WHITE – Port Initialised in DMX Input mode, nothing has been detected as a valid DMX signal.
CYAN - DMX Input is being received and is being broadcasted to Artnet.
GREEN - DMX is being received from Artnet; the port is now outputting WS2812 data. 

### **Complete List of Features:**
-	sACN and ArtNet V4 support **
-	One full universe of DMX output with full RDM support **
-	Up to 1360 ws2812(b) pixels - 8 full universes **
-	DMX in capability - send any incoming data to any ArtNet universe/subnet **
-	Web UI using AJAX & JSON to minimise network traffic used & decrease latency **
-	Full DMX Workshop, ETC, and Chamsys MagicQ support 
-	Pixel FX output - a 12 channel mode for ws2812 LED pixel control (requires extra addon PCB) **
- I2C Expansion for OLED and LCD Displays
- Seamless WiFi meshing for large venues

** - With thanks to [Matt's project.](https://github.com/mtongnz/ESP8266_ArtNetNode_v2)

I hope this helps you get the nodes up and running. 
All the source code and newer firmware releases will be put onto our website (https://expanseelectronics.com) and the GitHub repository (https://github.com/expanseelectronics).

