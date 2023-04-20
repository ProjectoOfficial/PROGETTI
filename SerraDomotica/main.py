import serial
import serial.tools.list_ports

from flask import Flask, render_template, jsonify, request
import socketserver
import random
from math import log10

global ser 
ADDRESS = 10
FREQUENCY = 470000000
CRLF = "\r\n"

ports = serial.tools.list_ports.comports()
assert ports != [], "Ports list is empty"
port = random.choice(ports)
ser = serial.Serial(
    port=port.device,\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
        timeout=0)
print("connected to: " + ser.portstr)

app = Flask(__name__, template_folder='templates')

# HOME PAGE
@app.route("/")
def home():
    return render_template("index.html")


@app.route('/update')
def update():
    # Aggiorniamo la variabile
    global temp, hum, press, thum1, thum2, ppm
    (temp, hum, press, thum1, thum2, ppm) = get_data(ser)

    # Restituiamo la nuova variabile come JSON
    data = {'temp': temp, 'hum': hum, 'press': press, 'thum1': thum1, 'thum2': thum2, 'ppm': ppm}
    return jsonify(data)

def serial_setup(ser):
    print("setting up serial device")
    command = "AT+BAND=" + str(FREQUENCY) + CRLF
    ser.write(command.encode())
    while(not "OK" in read_data(ser)):
        pass 
    
    command = "AT+BAND?" + CRLF
    ser.write(command.encode())
    while(not "BAND" in read_data(ser)):
        pass 
    
    command = "AT+ADDRESS=" + str(ADDRESS) + CRLF
    ser.write(command.encode())
    while(not "OK" in read_data(ser)):
        pass
    
    command = "AT+ADDRESS?" + CRLF
    ser.write(command.encode())
    while(not "ADDRESS" in read_data(ser)):
        pass 
    
def LoRaReset(ser):
    command = "AT+RESET" + CRLF
    print(command.encode())
    ser.write(command.encode())
    while(not "OK" in read_data(ser) or not not "RESET" in read_data(ser)):
        pass
    
def read_data(ser):
    line = []
    while True:
        c = ser.read()
        line.append(c.decode("UTF-8"))
        if c == b'\n':
            line = ''.join(line)
            break
    print(line)
    
    return line

def get_MQ135(sensor_volt):
    RL_VALUE = 20; # Impostazione della resistenza del carico del sensore in Kohm
    RO_CLEAN_AIR_FACTOR = 9.83; # Impostazione del fattore RO per aria pulita
    calibration = -0.4; # Calibrazione del sensore (da modificare in base alle esigenze)
    
    sensor_volt = sensor_volt * 5.0 / 4096
    RS_gas = (5.0 - sensor_volt) / sensor_volt
    ratio = RS_gas / RO_CLEAN_AIR_FACTOR
    ppm_log = (log10(ratio) - calibration) / (-0.307)
    ppm = pow(10, ppm_log)
    return int(ppm)


def get_data(ser):
    data = read_data(ser)  
    
    try:
        data.split(",")[2]
    except:
        print(data)
        return (0, 0, 0, 0, 0, 0)
    
    if not "RCV" in data:
        return (0, 0, 0, 0, 0, 0)
    
    if "ERR" in data:
        LoRaReset(ser)
        
    data = data.split(",")[2]
    temp = data.split("#")[0] + "°C"
    hum = data.split("#")[1] + "%"
    press = str(int(data.split("#")[2])/100000) + " BAR"
    thum1 = data.split("#")[3] + "%"
    thum2 = data.split("#")[4] + "%"    
    ppm = str(get_MQ135(int(data.split("#")[5])))
    return (temp, hum, press, thum1, thum2, ppm)


if __name__ == "__main__":
    serial_setup(ser)
    try:
        with socketserver.TCPServer(('localhost', 5385), socketserver.BaseRequestHandler) as httpd:
            app.run(host='localhost', port=9876, threaded=True, debug=False, use_reloader=False)
            httpd.server_close()
    except KeyboardInterrupt:
        print("Programma interrotto dall'utente")
    finally:
        ser.close()
    