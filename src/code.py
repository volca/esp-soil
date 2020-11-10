import busio
import pwmio
import analogio
import wifi
from board import *
from digitalio import DigitalInOut, Direction, Pull

MAX_SOIL_VAL = 3515
MIN_SOIL_VAL = 3414

pin_clk     = IO17

def valmap(value, istart, istop, ostart, ostop):
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart))

#print(wifi.radio.connect("siyue2L-dong", "siyuexiongdi"))
#print("IP", wifi.radio.ipv4_address)

pwm = pwmio.PWMOut(pin = pin_clk, frequency = 40000)
pwm.duty_cycle = 2 ** 15

pin_soil = analogio.AnalogIn(IO9)
soil_val = pin_soil.value
print(soil_val)
print(valmap(soil_val, MIN_SOIL_VAL, MAX_SOIL_VAL, 100, 0))
pin_soil.deinit()
pwm.deinit()

print("hey world")
