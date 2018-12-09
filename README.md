# FT260 I2C Master Driver for Linux

## copy usb rule file:
copy 11-ft260.rules to /etc/udev/rules.d/  
```
sudo cp 11-ft260.rules /etc/udev/rules.d/
```

## compile:
```
g++ -o test test.c ft260_driver.c -ludev -I./
```