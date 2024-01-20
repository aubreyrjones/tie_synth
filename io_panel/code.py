import rotaryio
import digitalio
import keypad
import busio, adafruit_midi.control_change
from board import GP0, GP1, GP2, GP3, GP4, GP10, GP11, GP6, GP7, GP8, GP9, GP18, GP19, GP20, GP21

encoders = [rotaryio.IncrementalEncoder(GP10, GP11, 2), rotaryio.IncrementalEncoder(GP3, GP4), rotaryio.IncrementalEncoder(GP7, GP8)]
encoder_pos = [0] * len(encoders)

button_pins = [GP18, GP19, GP20, GP21, GP9,  GP2, GP6]

keys = keypad.Keys(button_pins, value_when_pressed=False, pull=True)

def get_offset(enc_index):
    return encoder_pos[enc_index] - encoders[enc_index].position

def update_pos(enc_index):
    encoder_pos[enc_index] = encoders[enc_index].position

uart = busio.UART(GP0, GP1, baudrate=1000000, timeout=0)  # open UART0 at 1Mbaud (hopefully?)

def send_cc(cc, value):
    msg = adafruit_midi.control_change.ControlChange(cc, value, channel=15)
    uart.write(bytes(msg))
    print("CC %s %s" % (cc, value))
    
def send_increments(cc, count):
    if count == 0: return
    val = 127 if count > 0 else 0 # 127 for positive clicks
    for i in range(0, abs(count)):
        send_cc(cc, val)

while True:
    while True:
        ev = keys.events.get()
        if ev is None: break
        #print("CC %s %s" % (ev.key_number + len(encoders), 127 if ev.pressed else 0))
        send_cc(ev.key_number + len(encoders), 127 if ev.pressed else 0)
    
    for i in range(0, len(encoders)):
        send_increments(i, get_offset(i))
        update_pos(i)
