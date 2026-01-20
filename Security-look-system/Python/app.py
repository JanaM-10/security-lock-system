
from flask import Flask, render_template, request, jsonify
import serial
import threading
import time
import redis
from flask_socketio import SocketIO, emit
import random
import string

app = Flask(__name__)
app.config['REDIS_URL'] = 'redis://172.22.107.73'  
socketio = SocketIO(app)

ports=["COM4","COM9","COM8","COM5"]
bluetooth=None
for i in ports:
    try:
        bluetooth = serial.Serial(i, 9600, timeout=1)
        print(f"Connected to {i}")
        break 
    except Exception as e:
        print("Exception on",i)

redis_client = redis.StrictRedis(host='172.22.107.73', port=6379, db=0)  # Connect to Redis 

@app.route('/')
def home():
    activation_code = redis_client.get("activation_code")
    return render_template('index.html', activation_code=activation_code.decode('utf-8') if activation_code else "No code generated yet.")

def generate_activation_code():
    char_pool = string.digits + string.ascii_uppercase[:4] + '*#'
    return ''.join([str(random.choice(char_pool)) for _ in range(4)])
    
def listen_to_arduino():
    while True:
        # if bluetooth.in_waiting > 0:
        message = bluetooth.readline().decode('utf-8').strip()
        message = message.upper()
        if message:
            print(f"Received: {message.encode()} , {len(message)}")
            if"GRANTED" in message:
                socketio.emit("access", {"message": "Access has been granted, door unlocked."})
            if "WRONG" in message:
                socketio.emit("security", {"message": "Wrong password attempt detected!"})
            if "SYSTEM LOCKED" in message:
                code = generate_activation_code()
                print(f"Activation Code: {code}")
                redis_client.set("activation_code",code)
                bluetooth.write(code.encode() + b'\n')

def update_activation_code():
    activation_code=None
    while True:
        if activation_code is None or activation_code != redis_client.get("activation_code"):
            activation_code = redis_client.get("activation_code").decode("utf-8")
            socketio.emit("notifications", {"success": True, "activation_code":activation_code, "message":"Activation code has just been updated"})
        time.sleep(1)

@app.route('/stop_buzzer', methods=['POST'])
def stop_buzzer():
    bluetooth.write("B\n".encode())
    return jsonify({"status": "Buzzer turned off"})

@app.route('/turn_light_off', methods=['GET', 'POST'])
def turn_light_off():
    print("Called turn_light_off")
    print("Bluetooth connection is open, sending 'L' to turn off light.")
    bluetooth.write("L\n".encode()) 
    return jsonify({"status": "Light turned off"})

@app.route('/change_passcode', methods=['POST'])
def change_passcode():
    new_passcode = request.json.get('new_passcode')
    if new_passcode and len(new_passcode) == 4 and all(c.isdigit() or c in "ABCD*#" for c in new_passcode):
        global passcode
        passcode = new_passcode
        bluetooth.write(f"PASS:{new_passcode}\n".encode())
        print(f"PASS:{new_passcode}\n".encode())
        return jsonify({"status": "Passcode updated successfully"})
    else:
        return jsonify({"status": "Invalid passcode. Must be 4 characters (A~D or digits)."}), 400

web_socket_thread = threading.Thread(target=update_activation_code)
web_socket_thread.daemon = True
web_socket_thread.start()

listener_thread = threading.Thread(target=listen_to_arduino)
listener_thread.daemon = True
listener_thread.start() 

if __name__ == '__main__':
    app.run(debug=False,port=80,host="127.0.0.1")