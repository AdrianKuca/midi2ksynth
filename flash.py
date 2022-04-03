import subprocess
from time import sleep

AVRDUDE = "D:\\avrdude\\avrdude.exe"
print("RESET IT!")
print("3")
sleep(1)
print("2")
sleep(1)
print("1")
sleep(0.5)
print("GO!")
subprocess.run([AVRDUDE, "-p", "atmega32u4", "-P",
               "usb", "-D", "-U", "flash:w:midi2ksynth.hex"])
