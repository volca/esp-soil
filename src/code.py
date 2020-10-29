import busio
import pwmio
import analogio
import wifi
from board import *
from digitalio import DigitalInOut, Direction, Pull

pin_clk     = IO17

print(wifi.radio.connect("siyue2L-dong", "siyuexiongdi"))
print("IP", wifi.radio.ipv4_address)

pwm = pwmio.PWMOut(pin_clk)
pwm.duty_cycle = 2 ** 15

pin_soil = analogio.AnalogIn(IO9)
print(pin_soil.value)
pin_soil.deinit()

print("hey world")
