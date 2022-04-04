import subprocess
from time import sleep
import usb.core
import usb.util
from sys import platform
if platform == "linux" or platform == "linux2":
    AVRDUDE = "avrdude"
    DEVICE = "/dev/ttyACM0"
elif platform == "win32":
    AVRDUDE = "D:\\avrdude\\avrdude.exe"
    DEVICE = "usb:2341:0036"


vid = 0x2341
pid = 0x0036
dev = usb.core.find(idVendor=vid, idProduct=pid)

if dev is not None:
    print("Found board: resetting")
    dev = usb.core.find(idVendor=vid, idProduct=pid)
    print(dev)
    dev.ctrl_transfer(usb.util.CTRL_TYPE_VENDOR | usb.util.CTRL_RECIPIENT_DEVICE, 0x01)
    sleep(5)

subprocess.run(
    [
        AVRDUDE,
        "-p",
        "atmega32u4",
        "-c",
        "avr109",
        "-P",
        DEVICE,
        "-D",
        "-U",
        "flash:w:midi2ksynth.hex",
    ]
)
