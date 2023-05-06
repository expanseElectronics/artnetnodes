/*
dualETH Ethernet ArtNet Node 

Base Code Copyright (c) 2016, Matthew Tong
https://github.com/mtongnz/
Code Improvements Copyright (c) 2023, expanseElectronics Ltd
https://github.com/expanseElectronics/

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see http://www.gnu.org/licenses/
*/

void ajaxHandle() {
  JsonObject& json = jsonBuffer.parseObject(webServer.arg("plain"));
  JsonObject& jsonReply = jsonBuffer.createObject();
  
  String reply;
  
  if (json.containsKey("success") && json["success"] == 1 && json.containsKey("doUpdate")) {
    artRDM.end();
    
    jsonReply["success"] = 1;
    jsonReply["doUpdate"] = 1;
    
    jsonReply.printTo(reply);
    webServer.send(200, "application/json", reply);

    if (json["doUpdate"] == 1) {
      
      deviceSettings.doFirmwareUpdate = true;
      eepromSave();
      
      doReboot = true;
    }
    
  } else if (json.containsKey("success") && json["success"] == 1 && json.containsKey("page")) {
    if (ajaxSave((uint8_t)json["page"], json)) {
      ajaxLoad((uint8_t)json["page"], jsonReply);

      if (json.size() > 2)
        jsonReply["message"] = "Settings Saved";

    } else {
      jsonReply["success"] = 0;
      jsonReply["message"] = "Failed to save data.  Reload page and try again.";
    }
    
  } else if (json.containsKey("success") && json.containsKey("reboot") && json["reboot"] == 1) {
    jsonReply["success"] = 1;
    jsonReply["message"] = "Device Restarting.";
    
    
    doReboot = true;

  } 

  jsonReply.printTo(reply);
  webServer.send(200, "application/json", reply);
}

bool ajaxSave(uint8_t page, JsonObject& json) {
  if (json.size() == 2)
    return true;
  
  switch (page) {
    case 1:   
      return true;
      break;

    case 2:    
      json.get<String>("wifiSSID").toCharArray(deviceSettings.wifiSSID, 40);
      json.get<String>("wifiPass").toCharArray(deviceSettings.wifiPass, 40);
      json.get<String>("hotspotSSID").toCharArray(deviceSettings.hotspotSSID, 20);
      json.get<String>("hotspotPass").toCharArray(deviceSettings.hotspotPass, 20);
      deviceSettings.hotspotDelay = (uint8_t)json["hotspotDelay"];
      deviceSettings.standAloneEnable = (bool)json["standAloneEnable"];

      eepromSave();
      return true;
      break;

    case 3:  
      deviceSettings.ip = IPAddress(json["ipAddress"][0],json["ipAddress"][1],json["ipAddress"][2],json["ipAddress"][3]);
      deviceSettings.subnet = IPAddress(json["subAddress"][0],json["subAddress"][1],json["subAddress"][2],json["subAddress"][3]);
      deviceSettings.gateway = IPAddress(json["gwAddress"][0],json["gwAddress"][1],json["gwAddress"][2],json["gwAddress"][3]);
      deviceSettings.broadcast = uint32_t(deviceSettings.ip) | (~uint32_t(deviceSettings.subnet));
      json.get<String>("nodeName").toCharArray(deviceSettings.nodeName, 18);
      json.get<String>("longName").toCharArray(deviceSettings.longName, 64);

      if (!isHotspot && (bool)json["dhcpEnable"] != deviceSettings.dhcpEnable) {
        
        if ((bool)json["dhcpEnable"]) {
          deviceSettings.gateway = INADDR_NONE;
          
        }

        doReboot = true;
      }

      if (!isHotspot) {
        artRDM.setShortName(deviceSettings.nodeName);
        artRDM.setLongName(deviceSettings.longName);
      }
      
      deviceSettings.dhcpEnable = (bool)json["dhcpEnable"];
      
      eepromSave();
      return true;
      break;

    case 4:   
      {
        deviceSettings.portAprot = (uint8_t)json["portAprot"];
        bool e131 = (deviceSettings.portAprot == PROT_ARTNET_SACN) ? true : false;
        
        deviceSettings.portAmerge = (uint8_t)json["portAmerge"];
  
        if ((uint8_t)json["portAnet"] < 128)
          deviceSettings.portAnet = (uint8_t)json["portAnet"];
  
        if ((uint8_t)json["portAsub"] < 16)
        deviceSettings.portAsub = (uint8_t)json["portAsub"];
  
        for (uint8_t x = 0; x < 4; x++) {
          if ((uint8_t)json["portAuni"][x] < 16)
            deviceSettings.portAuni[x] = (uint8_t)json["portAuni"][x];
          
          if ((uint16_t)json["portAsACNuni"][x] > 0 && (uint16_t)json["portAsACNuni"][x] < 64000)
            deviceSettings.portAsACNuni[x] = (uint16_t)json["portAsACNuni"][x];

          artRDM.setE131(portA[0], portA[x+1], e131);
          artRDM.setE131Uni(portA[0], portA[x+1], deviceSettings.portAsACNuni[x]);
        }
  
        uint8_t newMode = json["portAmode"];
        uint8_t oldMode = deviceSettings.portAmode;
        bool updatePorts = false;
        
        if (newMode == TYPE_DMX_IN && json.containsKey("dmxInBroadcast"))
          deviceSettings.dmxInBroadcast = IPAddress(json["dmxInBroadcast"][0],json["dmxInBroadcast"][1],json["dmxInBroadcast"][2],json["dmxInBroadcast"][3]);
         
        
        if (newMode != oldMode) {
  
          deviceSettings.portAmode = newMode;

          doReboot = true;
        }
        
        artRDM.setNet(portA[0], deviceSettings.portAnet);
        artRDM.setSubNet(portA[0], deviceSettings.portAsub);
        artRDM.setUni(portA[0], portA[1], deviceSettings.portAuni[0]);
        artRDM.setMerge(portA[0], portA[1], deviceSettings.portAmerge);

        if (newMode == TYPE_WS2812 && !doReboot) {
          uint16_t newLen = (json.containsKey("portAnumPix")) ? (uint16_t)json["portAnumPix"] : deviceSettings.portAnumPix;
          if (newLen > 680)
            newLen = 680;
          
          uint16_t oldLen = deviceSettings.portAnumPix;
          bool lenChanged = false;
  
          if (newLen <= 680 && oldLen != newLen) {
            deviceSettings.portAnumPix = newLen;
  
            lenChanged = true;
            if (deviceSettings.portApixMode == FX_MODE_PIXEL_MAP)
              updatePorts = true;
          }
            
          if (deviceSettings.portApixMode == FX_MODE_12)
            oldLen = 12;
  
          if (deviceSettings.portApixMode != (uint8_t)json["portApixMode"])
            updatePorts = true;
  
          deviceSettings.portApixMode = (uint8_t)json["portApixMode"];
  
          if (deviceSettings.portApixMode == FX_MODE_12) {
            if ((uint16_t)json["portApixFXstart"] <= 501 && (uint16_t)json["portApixFXstart"] > 0)
              deviceSettings.portApixFXstart = (uint16_t)json["portApixFXstart"];
            newLen = 12;
          }

          if (updatePorts) {
            for (uint8_t x = 1, y = 2; x < 4; x++, y++) {
              uint16_t c = (x * 170);
              if (newLen > c)
                portA[y] = artRDM.addPort(portA[0], x, deviceSettings.portAuni[x], TYPE_DMX_OUT, deviceSettings.portAmerge);
              else if (oldLen > c)
                artRDM.closePort(portA[0], portA[y]);
            }
          }
  
          for (uint8_t x = 1, y = 2; x < 4; x++, y++) {
            if (newLen > (x * 170)) {
              artRDM.setUni(portA[0], portA[y], deviceSettings.portAuni[x]);
              artRDM.setMerge(portA[0], portA[y], deviceSettings.portAmerge);
            }
          }
        }
  
        artRDM.artPollReply();
  
        eepromSave();
        return true;
      }
      break;

    case 5:  
      break;

    case 6: 
      return true;
      break;

    case 7:
      break;

    default:
      return false;
  }
}

void ajaxLoad(uint8_t page, JsonObject& jsonReply) {

  JsonArray& ipAddress = jsonReply.createNestedArray("ipAddress");
  JsonArray& subAddress = jsonReply.createNestedArray("subAddress");
  JsonArray& gwAddress = jsonReply.createNestedArray("gwAddress");
  JsonArray& bcAddress = jsonReply.createNestedArray("bcAddress");
  JsonArray& portAuni = jsonReply.createNestedArray("portAuni");
  JsonArray& portBuni = jsonReply.createNestedArray("portBuni");
  JsonArray& portAsACNuni = jsonReply.createNestedArray("portAsACNuni");
  JsonArray& portBsACNuni = jsonReply.createNestedArray("portBsACNuni");
  JsonArray& dmxInBroadcast = jsonReply.createNestedArray("dmxInBroadcast");

  char MAC_char[30] = "";
  sprintf(MAC_char, "%02X", MAC_array[0]);
  for (int i = 1; i < 6; ++i)
    sprintf(MAC_char, "%s:%02X", MAC_char, MAC_array[i]);
  
  jsonReply["macAddress"] = String(MAC_char);

  switch (page) {
    case 1:   
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portAuni");
      jsonReply.remove("portBuni");
      jsonReply.remove("portAsACNuni");
      jsonReply.remove("portBsACNuni");
      jsonReply.remove("dmxInBroadcast");
      
      jsonReply["nodeName"] = deviceSettings.nodeName;
      jsonReply["wifiStatus"] = wifiStatus;
      
      if (isHotspot) {
        jsonReply["ipAddressT"] = deviceSettings.hotspotIp.toString();
        jsonReply["subAddressT"] = deviceSettings.hotspotSubnet.toString();
      } else {
        jsonReply["ipAddressT"] = deviceSettings.ip.toString();
        jsonReply["subAddressT"] = deviceSettings.subnet.toString();
      }

      if (isHotspot && !deviceSettings.standAloneEnable) {
        jsonReply["portAStatus"] = "Disabled in hotspot mode";
        jsonReply["portBStatus"] = "Disabled in hotspot mode";
      } else {
        switch (deviceSettings.portAmode) {
          case TYPE_DMX_OUT:
            jsonReply["portAStatus"] = "DMX output";
            break;
  
          case TYPE_RDM_OUT:
            jsonReply["portAStatus"] = "DMX/RDM output";
            break;
  
          case TYPE_DMX_IN:
            jsonReply["portAStatus"] = "DMX input";
            break;
  
          case TYPE_WS2812:
            jsonReply["portAStatus"] = "WS2812 mode";
            break;
        }
        switch (deviceSettings.portBmode) {
          case TYPE_DMX_OUT:
            jsonReply["portBStatus"] = "DMX output";
            break;
  
          case TYPE_RDM_OUT:
            jsonReply["portBStatus"] = "DMX/RDM output";
            break;
  
          case TYPE_DMX_IN:
            jsonReply["portBStatus"] = "DMX input";
            break;
  
          case TYPE_WS2812:
            jsonReply["portBStatus"] = "WS2812 mode";
            break;
        }
      }
      
      jsonReply["sceneStatus"] = "Not outputting<br />0 Scenes Recorded<br />0 of 250KB used";
      jsonReply["firmwareStatus"] = FIRMWARE_VERSION;

      jsonReply["success"] = 1;
      break;

    case 2:   
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portAuni");
      jsonReply.remove("portBuni");
      jsonReply.remove("portAsACNuni");
      jsonReply.remove("portBsACNuni");
      jsonReply.remove("dmxInBroadcast");
      
      jsonReply["wifiSSID"] = deviceSettings.wifiSSID;
      jsonReply["wifiPass"] = deviceSettings.wifiPass;
      jsonReply["hotspotSSID"] = deviceSettings.hotspotSSID;
      jsonReply["hotspotPass"] = deviceSettings.hotspotPass;
      jsonReply["hotspotDelay"] = deviceSettings.hotspotDelay;
      jsonReply["standAloneEnable"] = deviceSettings.standAloneEnable;
      
      jsonReply["success"] = 1;
      break;

    case 3:   
      jsonReply.remove("portAuni");
      jsonReply.remove("portBuni");
      jsonReply.remove("portAsACNuni");
      jsonReply.remove("portBsACNuni");
      jsonReply.remove("dmxInBroadcast");
      
      jsonReply["dhcpEnable"] = deviceSettings.dhcpEnable;

      for (uint8_t x = 0; x < 4; x++) {
        ipAddress.add(deviceSettings.ip[x]);
        subAddress.add(deviceSettings.subnet[x]);
        gwAddress.add(deviceSettings.gateway[x]);
        bcAddress.add(deviceSettings.broadcast[x]);
      }

      jsonReply["nodeName"] = deviceSettings.nodeName;
      jsonReply["longName"] = deviceSettings.longName;

      jsonReply["success"] = 1;
      break;

    case 4:   
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portBuni");
      jsonReply.remove("portBsACNuni");
      
      jsonReply["portAmode"] = deviceSettings.portAmode;

      if (deviceSettings.portAmode == TYPE_DMX_IN)
        jsonReply["portAprot"] = PROT_ARTNET;
      else
        jsonReply["portAprot"] = deviceSettings.portAprot;
 
      jsonReply["portAmerge"] = deviceSettings.portAmerge;
      jsonReply["portAnet"] = deviceSettings.portAnet;
      jsonReply["portAsub"] = deviceSettings.portAsub;
      jsonReply["portAnumPix"] = deviceSettings.portAnumPix;
      
      jsonReply["portApixMode"] = deviceSettings.portApixMode;
      jsonReply["portApixFXstart"] = deviceSettings.portApixFXstart;

      for (uint8_t x = 0; x < 4; x++) {
        portAuni.add(deviceSettings.portAuni[x]);
        portAsACNuni.add(deviceSettings.portAsACNuni[x]);
        dmxInBroadcast.add(deviceSettings.dmxInBroadcast[x]);
      }

      jsonReply["success"] = 1;
      break;

    case 5:    
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portAuni");
      jsonReply.remove("portAsACNuni");
      
      jsonReply["portBmode"] = deviceSettings.portBmode;
      jsonReply["portBprot"] = deviceSettings.portBprot;
      jsonReply["portBmerge"] = deviceSettings.portBmerge;
      jsonReply["portBnet"] = deviceSettings.portBnet;
      jsonReply["portBsub"] = deviceSettings.portBsub;
      jsonReply["portBnumPix"] = deviceSettings.portBnumPix;
      
      jsonReply["portBpixMode"] = deviceSettings.portBpixMode;
      jsonReply["portBpixFXstart"] = deviceSettings.portBpixFXstart;

      for (uint8_t x = 0; x < 4; x++) {
        portBuni.add(deviceSettings.portBuni[x]);
        portBsACNuni.add(deviceSettings.portBsACNuni[x]);
        dmxInBroadcast.add(deviceSettings.dmxInBroadcast[x]);
      }
      
      jsonReply["success"] = 1;
      break;

    case 6:    
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portAuni");
      jsonReply.remove("portBuni");
      jsonReply.remove("portAsACNuni");
      jsonReply.remove("portBsACNuni");
      jsonReply.remove("dmxInBroadcast");
      

      jsonReply["success"] = 1;
      break;

    case 7:  
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portAuni");
      jsonReply.remove("portBuni");
      jsonReply.remove("portAsACNuni");
      jsonReply.remove("portBsACNuni");
      jsonReply.remove("dmxInBroadcast");
      
      jsonReply["firmwareStatus"] = FIRMWARE_VERSION;
      jsonReply["success"] = 1;
      break;

    default:
      jsonReply.remove("ipAddress");
      jsonReply.remove("subAddress");
      jsonReply.remove("gwAddress");
      jsonReply.remove("bcAddress");
      jsonReply.remove("portAuni");
      jsonReply.remove("portBuni");
      jsonReply.remove("portAsACNuni");
      jsonReply.remove("portBsACNuni");
      jsonReply.remove("dmxInBroadcast");
      
      jsonReply["success"] = 0;
      jsonReply["message"] = "Invalid or incomplete data received.";
  }
}
