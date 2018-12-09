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