# FT260 I2C-Master Driver for Linux

## 1. Copy usb rules file:
copy 11-ft260.rules to /etc/udev/rules.d/  
```
sudo cp 11-ft260.rules /etc/udev/rules.d/
```

## 2. Compile:
```
g++ -o test test.c ft260_driver.c -ludev -I./
```

## 3. Run test
```
./test
```

## 4. Useful links
DataSheets: https://www.ftdichip.com/Support/Documents/DataSheets/ICs/DS_FT260.pdf  
Application Note:https://www.ftdichip.com/Support/Documents/AppNotes/AN_394_User_Guide_for_FT260.pdf