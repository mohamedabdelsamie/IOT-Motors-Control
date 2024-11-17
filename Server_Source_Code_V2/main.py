# app.py

import threading
import time
import json
import os
from flask import Flask, render_template, jsonify
from flask_socketio import SocketIO, emit
import paho.mqtt.client as mqtt

# Flask application
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")  # Use threading mode

# MQTT Settings
BROKER = '78.111.89.85'
PORT = 1883
Subscribtion_topic_Intensity = 'flask/mqtt/intensity'
Subscribtion_topic_Position = 'flask/mqtt/position'
Subscribtion_topic_Looping = 'flask/mqtt/looping'
Subscribtion_topic_Status = 'status'
topic_Intensity = "esp32/intensity"
topic_Position = "esp32/position"
topic_Looping = "esp32/looping"
topic_Stop = "esp32/stop"

# Data file for persistent storage
DATA_FILE = 'data.json'

# Global flag and variable for connection status
connected_flag = False
broker_status = 'disconnected'


# Initialize MQTT client
client = mqtt.Client()
client.username_pw_set("IOT", "IOT_Steve")
# Load saved data if available or initialize default values
def load_data():
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE, 'r') as f:
            return json.load(f)
    return {'intensity_Value': 0, 'position_Value': 50, 'looping_Flag': 0, 'controller_Connection_Flag': 0}

# Save data to JSON file
def save_data(data):
    with open(DATA_FILE, 'w') as f:
        json.dump(data, f, indent=4)

# Load initial values
data = load_data()
intensity_Value = data.get('intensity_Value', 0)
position_Value = data.get('position_Value', 50)
looping_Flag = data.get('looping_Flag', 0)
controller_Connection_Flag = data.get('controller_Connection_Flag', 0)

# MQTT Callbacks
def on_connect(client, userdata, flags, rc):
    global connected_flag, broker_status
    if rc == 0:
        print("Connected to MQTT Broker!")
        connected_flag = True
        broker_status = 'connected'
        client.subscribe(Subscribtion_topic_Intensity)
        client.subscribe(Subscribtion_topic_Position)
        client.subscribe(Subscribtion_topic_Looping)
        client.subscribe(Subscribtion_topic_Status)
        socketio.emit('broker_status', {'status': broker_status})
    else:
        print(f"Failed to connect, return code {rc}")
        connected_flag = False
        broker_status = 'disconnected'
        socketio.emit('broker_status', {'status': broker_status})

def on_message(client, userdata, msg):
    global intensity_Value, position_Value, looping_Flag, controller_Connection_Flag
    message = msg.payload.decode()
    
    if msg.topic == Subscribtion_topic_Intensity:
        intensity_Value = int(message)
        Update_Sockets_Values()
    if msg.topic == Subscribtion_topic_Position:
        position_Value = int(message)
        socketio.emit('position_Value', {'value': position_Value})
    if msg.topic == Subscribtion_topic_Looping:
        looping_Flag = int(message)
        socketio.emit('looping_Flag', {'value': looping_Flag})
    if msg.topic == Subscribtion_topic_Status:
        controller_Connection_Flag = int(message)
        socketio.emit('Controller_Connection_Flag', {'value': int(controller_Connection_Flag)})

        

        
    
    save_data({'intensity_Value': intensity_Value, 'position_Value': position_Value, 'looping_Flag': looping_Flag, 'controller_Connection_Flag': controller_Connection_Flag})
    print(f"Received message: {message} on topic {msg.topic}")

def on_disconnect(client, userdata, rc):
    global connected_flag, broker_status
    connected_flag = False
    broker_status = 'disconnected'
    print("Disconnected from MQTT Broker")
    socketio.emit('broker_status', {'status': broker_status})

# Reconnect to the MQTT broker if disconnected
def reconnect_loop():
    global connected_flag
    while True:
        if not connected_flag:
            print("Attempting to reconnect to the broker...")
            try:
                client.reconnect()  # Attempt reconnection
            except Exception as e:
                print(f"Reconnection attempt failed: {e}")
            time.sleep(5)
        else:
            time.sleep(1)  # Sleep briefly to prevent tight loop

# MQTT Thread
def mqtt_thread():
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect
    try:
        client.connect(BROKER, PORT, 60)
        client.loop_start()
    except Exception as e:
        print(f"Initial MQTT connection failed: {e}")

# Flask Routes
@app.route('/')
def index():
    return render_template('index.html')

# Handle WebSocket connection with added exception handling
@socketio.on('connect')
def handle_websocket_connect():
    try:
        print('WebSocket client connected')
        emit('broker_status', {'status': broker_status})
        emit('intensity_Value', {'value': intensity_Value})
        emit('position_Value', {'value': position_Value})
        emit('looping_Flag', {'value': looping_Flag})
        emit('Controller_Connection_Flag', {'value': controller_Connection_Flag})
    except ConnectionResetError:
        print("Connection reset by the client")

@socketio.on('disconnect')
def handle_websocket_disconnect():
    print('WebSocket client disconnected')

# Handle looping Mode for position Motor
@socketio.on('looping_Flag')
def handle_looping_mood(data): 
    global looping_Flag
    looping_Flag = int(data['value'])
    client.publish(topic_Looping, looping_Flag)
    socketio.emit('looping_Flag', {'value': looping_Flag})
    save_data({'intensity_Value': intensity_Value, 'position_Value': position_Value, 'looping_Flag': looping_Flag, 'controller_Connection_Flag': controller_Connection_Flag})

# Handle Stop Mode for position Motor
@socketio.on('stop_Flag')
def handle_stop_mood(data): 
    client.publish(topic_Stop, 1)


# Handle slider value updates
@socketio.on('slider_value')
def handle_slider_value(data):
    global intensity_Value, position_Value
    slider_id = data['id']
    value = int(data['value'])
    
    if slider_id == 'slider2':
        intensity_Value = value
        client.publish(topic_Intensity, intensity_Value)
        socketio.emit('intensity_Value', {'value': intensity_Value})
    elif slider_id == 'slider1':
        position_Value = value
        client.publish(topic_Position, position_Value)
        socketio.emit('position_Value', {'value': position_Value})

    save_data({'intensity_Value': intensity_Value, 'position_Value': position_Value, 'looping_Flag': looping_Flag, 'controller_Connection_Flag': controller_Connection_Flag})
    print(f"Published: {slider_id} - {value}")

def Update_Sockets_Values():
    socketio.emit('intensity_Value', {'value': intensity_Value})

# Main function to start Flask and MQTT threads
if __name__ == '__main__':
    mqtt_thread_instance = threading.Thread(target=mqtt_thread)
    mqtt_thread_instance.daemon = True
    mqtt_thread_instance.start()

    reconnect_thread_instance = threading.Thread(target=reconnect_loop)
    reconnect_thread_instance.daemon = True
    reconnect_thread_instance.start()

    # Run Flask app directly with SocketIO
    socketio.run(app, host='0.0.0.0', port=5000,allow_unsafe_werkzeug=True)
