# Velleman k8055 C++
 Velleman K8055 C++ Replacement using Fox and HIDAPI.  This project is my effort to keep the vintage but still excellent P8055-1 Velleman development board.  This was my first development board back in 2003 - which was a kit - long before Arduino become ubiquitous.  Velleman never seemed to release an open source version of their DLL - K8055D.dll - not a problem I suppose but it would have been nice to be able to go native.  Read on....

![Velleman k8055 board](https://raw.github.com/rm-hull/k8055/master/k8055.jpg)

There was a "libusb" - Linux version done very nicely here [k8055](https://github.com/rm-hull/k8055) I did try this on Windows but it meant having to load up libusb and replace the driver for the board.  The P8055 will load a standard HID driver and as such on windows - the libsub solution didnt work. I did try to load up libusb with Zadig - but I couldn't get it to write  properly and anyway I am not too keen on using libusb on Windows - you should use native drivers on USB. 

So I had a look at the [hidapi](https://github.com/libusb/hidapi) suite which is actually part of the currently supported [libusb](https://github.com/libusb) suite which means it works quite well.  I also found that there was a test app -"testgui" which works well with Visual Studio. 

So I took that testgui and the excellent library components from [k8055](https://github.com/rm-hull/k8055) and built version using the equally excellent [Fox GUI Toolkit](https://www.fox-toolkit.org/) 

So you can use this approach to write a native application using HID for a Windows application. You can also use the Velleman DLL which works equally well.  This project was more of a challenge… Use it with caution... But its a good example of a C++ gui and the HID library.

![Fox GUI](https://github.com/daverobertson63/velleman-k8055/blob/0a35307c10ba09186a9dd99867634ad1f66a15f9/gui.png)


## Installation

This is a Visual Studio project so just use git to clone it and run and compile.  You should also get the hidapi from github and the fox toolkit version fox-1.7.85 - which seems to work ok.  I normally put everything into the git folder. so.... cd /users/username assuming you have a git folder

```bash

download from http://fox-toolkit.org/ftp/fox-1.7.84.zip
unzip fox-1.7.84.zip
git clone https://github.com/libusb/hidapi.git

git clone https://github.com/daverobertson63/velleman-k8055.git


```

## Usage

```c++
The k8055GUI.cpp is the Fox toolkit gui version
The libk8055.cpp can be used to wrap the HID library with all the details
```

## Contributing

Best idea just to fork and work away I wont be maintaining this going forward.  

## License

I see this as being open source - I have no problems in anyone using it without attribution. All other sources are acknowledged. 

[Apache](http://opensource.org/licenses/)
