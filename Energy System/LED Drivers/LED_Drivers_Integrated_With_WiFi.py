from machine import Pin, ADC, PWM
import urequests
import json
import network
import socket
from time import sleep
import utime 

# WiFi config
ssid = 'Vincenzo'
password = 'hello1234'

tried_connection = False
connected = False

time_interval = 2
start_time = utime.time()

def connect():
    #Connect to WLAN
    global tried_connection, connected
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(ssid, password)
    sleep(15)
    if wlan.isconnected() == False:
        connected = False
    else:
        connected = True
    tried_connection = True

vret_pin = ADC(Pin(26))
vout_pin = ADC(Pin(28))
vin_pin = ADC(Pin(27))
pwm = PWM(Pin(0))
pwm.freq(100000)
pwm_en = Pin(1, Pin.OUT)

voltage_ratio = 249/1249  # 0.19935949

count = 0
pwm_out = 0
pwm_ref = 0

led = Pin("LED", Pin.OUT)
led.on()

CURRENT_LIMIT = 0.29

# CURRENT PID CONTROL VARS
KPI = 0.0
KDI = 0.0
KII = 0.0

TS  = 0.001
# CURRENT VARS

CURRENT_ERROR = 0.0
CURRENT_DERIVATIVE_ERROR = 0.0
CURRENT_INTEGRAL_ERROR = 0.0
CURRENT_LAST_ERROR = 0.0
CURRENT_PID_OUTPUT = 0.0
PREVIOUS_CURRENT_PID_OUTPUT = 0.0

LAST_OUTPUT_CURRENT = 0.0


TARGET_CURRENT = 0.03

def saturate(duty):
    if duty > 62500:
        duty = 62500
    if duty < 100:
        duty = 100
    return duty
 

interrupt_flag = False

def interrupt_callback(timer):
    global interrupt_flag
    interrupt_flag = True
    
def post_to_server(power):
    power_data = {
        "red": power
    }
    try:
        response = urequests.post('https://balance-bug.5959pn4l16bde.eu-west-2.cs.amazonlightsail.com/api/datahub', data = json.dumps(power_data))
    except:
        pass
    
while True:
    
    # try for 15 seconds to connect to the sever
    if (tried_connection == False):
        connect()

    KPI = 45.0
    KDI = 0.0
    KII = 0.00054
    
    pwm_en.value(1)
    interrupt_flag = False
    
    vin   = 5*(vin_pin.read_u16()  * (3.3/65535))   # input voltage
    vout  = 5*(vout_pin.read_u16() * (3.3/65535))   # output voltage
    vret  = 0.7*(vret_pin.read_u16() * (3.3/65535))   # current reference pin (1.02ohm)
    current  = vret / 1.02
    
    if utime.time() - start_time >= time_interval:
        post_to_server(current*vout)
        start_time = utime.time()

    # PI(D) CONTROL
    # target current = 0.15A
    CURRENT_ERROR = TARGET_CURRENT - current
    CURRENT_INTEGRAL_ERROR += CURRENT_ERROR * TS
    CURRENT_DERIVATIVE_ERROR = (CURRENT_ERROR - CURRENT_LAST_ERROR) / TS
    CURRENT_LAST_ERROR = CURRENT_ERROR # set the last error to the current error (as it will now be the last)

    # antiwindup
    if (LAST_OUTPUT_CURRENT >= CURRENT_LIMIT):
        KII = 0.0
    elif (LAST_OUTPUT_CURRENT <= 0):        
        KII = 0.0

    # this output is used to control the duty cycle (adds to the last control signal)
    CURRENT_PID_OUTPUT += (KPI*CURRENT_ERROR) + (KDI*CURRENT_DERIVATIVE_ERROR) + (KII*CURRENT_INTEGRAL_ERROR)
    LAST_OUTPUT_CURRENT = current
    
    pwm_ref = int((CURRENT_PID_OUTPUT))
    pwm_out = saturate(pwm_ref)
    pwm.duty_u16(pwm_out)
