import subprocess
from time import sleep
import usb.core
import usb.util

AVRDUDE = "D:\\avrdude\\avrdude.exe"
vid = 0x03EB
pid = 0x2048
# dev = usb.core.find(find_all=1)
# for cfg in dev:
#     print(cfg)
# if dev is not None:
#     print("Found board: resetting")
#     dev = usb.core.find(idVendor=vid, idProduct=pid)
#     print(dev)
#     # dev.ctrl_transfer(usb.util.CTRL_TYPE_VENDOR | usb.util.CTRL_RECIPIENT_DEVICE, 0x01)
#     sleep(5)

subprocess.run(
    [
        AVRDUDE,
        "-p",
        "atmega32u4",
        "-c",
        "avr109",
        "-P",
        "usb:2341:0036",
        "-D",
        "-U",
        "flash:w:midi2ksynth.hex",
    ]
)
