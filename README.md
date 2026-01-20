# Security Lock System

The **Security Lock System** is an embedded solution designed to provide secure, convenient access to homes. Built around an **Arduino Mega microcontroller**, it integrates:

- A motion sensor for activity detection  
- A user-friendly keypad that only activates when motion is detected to save energy  
- An LCD for interaction  
- A servo motor controlling the sliding lock mechanism  
- Bluetooth communication via an **HC-05 module** linking the system to a **Flask web interface** for remote control and real-time notifications  

The system generates **unique activation codes** for secure re-entry, stored in **Redis**, and logs activities for added security. This smart solution offers peace of mind, eliminating the hassle of misplaced keys or unauthorized access.

---

## Features

- **Remote Control:** Control lock functions from a simple web interface built using Flask.  
- **Real-time Notifications:** Receive updates on activation codes, security events, and access logs via Socket.IO.  
- **Activation Code Storage:** Generated activation codes are securely stored in Redis for re-entry.  
- **Command Support:** Send commands to turn off the buzzer, light, or update passcodes.  
- **Integration with Arduino:** Powered by an Arduino, communicating via Bluetooth to the web interface.  

---

## Technologies Used

- **Arduino:** The embedded system managing the lock mechanism and sensors  
- **Flask:** Web framework for building the control panel  
- **Socket.IO:** Real-time communication for notifications and updates  
- **Redis:** Secure storage of activation codes  
- **HTML/CSS/JavaScript:** Frontend for the user interface  
- **HC-05 Bluetooth Module:** Communication between Arduino and Flask web app  

---

## Project Structure

```
security-lock-system/
 ├─ python/
 │    ├─ app.py
 │    └─ templates/
 │          └─ index.html
 ├─ arduino/
 │    └─ lock_system.ino
 └─ README.md
```

---

## Setup

### Prerequisites

- Python 3.x  
- Flask  
- Redis  
- Socket.IO (for real-time communication)  

### Installation

1. Clone the repository:

```bash
git clone https://github.com/USERNAME/security-lock-system.git
```

2. Navigate to the Python folder and install required Python libraries:

```bash
cd security-lock-system/python
pip install -r requirements.txt
```

> (Create `requirements.txt` with: `Flask`, `Flask-SocketIO`, `redis`)

3. Set up Redis server (local or remote).  
4. Upload the Arduino code to your Arduino board (see the `arduino/` folder).  
5. Run the Flask app:

```bash
python app.py
```

6. Open the website in your browser at: `http://127.0.0.1:5000/`

---

## Usage

- Access the control panel via the Flask web interface  
- Monitor activation codes and security events  
- Send commands to:
  - Turn off the buzzer  
  - Turn off the light  
  - Update passcodes  

---

## Notes

- Ensure the Arduino is connected via Bluetooth (HC-05) to the computer running Flask  
- Activation codes are stored securely in Redis and updated in real-time  
- Motion detection ensures energy-saving operation
