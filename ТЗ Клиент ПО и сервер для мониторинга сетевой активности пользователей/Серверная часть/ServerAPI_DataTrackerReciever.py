from flask import Flask, request, jsonify
import logging
import os
from werkzeug.utils import secure_filename
from datetime import datetime

app = Flask(__name__)

# Хранилище активных клиентов
active_clients = []
client_id_counter = 0

# Настройка логирования
logging.basicConfig(filename='activity.log', level=logging.INFO)

# Создание новой папки скриншотов (если не существует)
UPLOAD_FOLDER = 'screenshots'
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

# Переменные для хранения времени активности
keyboard_active_time = 0
mouse_active_time = 0
last_screenshot_path = None

# Обработчик POST запроса на логирование активности клавиатуры
@app.route('/keyboard_activity', methods=['POST'])
def keyboard_activity():
    global keyboard_active_time
    try:
        data = request.get_json()
        logging.info(f"Keyboard Activity: {data}")
        keyboard_active_time += 1

        response = {"Log_State": "Keyboard activity logged", "Status": 200, "Keyboard_Active_Time": keyboard_active_time}
        return jsonify(response), 200
    except Exception as e:
        logging.error(f"error logging keyboard activity: {e}")
        response = {"Log_State": "Failed to log keyboard activity", "Status": 500}
        return jsonify(response), 500

# Обработчик POST запроса на логирование активности мыши
@app.route('/mouse_activity', methods=['POST'])
def mouse_activity():
    global mouse_active_time
    try:
        data = request.get_json()
        logging.info(f"Mouse Activity: {data}")
        mouse_active_time += 1

        response = {"Log_State": "Mouse activity logged", "Status": 200, "Mouse_Active_Time": mouse_active_time}
        return jsonify(response), 200
    except Exception as e:
        logging.error(f"error logging mouse activity: {e}")
        response={"Log_State": "Failed to log mouse activity", "Status": 500}
        return jsonify(response), 500

# Обработчик POST запроса на сохранение скриншота
@app.route('/screenshot', methods=['POST'])
def screenshot():
    global last_screenshot_path
    try:
        if 'file' not in request.files:
            logging.error("No file part.")
            response = {"Error": "No file part", "Status": 400}
            return jsonify(response), 400
        
        file = request.files['file']
        if file.filename == '':
            logging.error("No selected file.")
            response = {"Error": "No selected file", "Status": 400}
            return jsonify(response), 400
        
        filepath = os.path.join(UPLOAD_FOLDER, secure_filename(file.filename))
        file.save(filepath)
        logging.info(f"Screenshot saved: {filepath}")
        response = {"Log_State": "Screenshot saved", "Status": 200}
        return jsonify(response), 200
    except Exception as e:
        logging.error(f"Error saving screenshot: {e}")
        return jsonify({"Log_State": "Failed to save screenshot", "status": 500}), 500

# Обработчик POST запроса на добавление клиента
@app.route('/add_client', methods=['POST'])
def add_client():
    global client_id_counter
    client_data = request.get_json()
    client_id_counter += 1
    client_data['id'] = client_id_counter
    active_clients.append(client_data)
    return jsonify({"Message": "Client added successfully", "client_id": client_id_counter}), 200

# Обработчик GET запроса на получение активных клиентов
@app.route('/get_active_clients', methods=['GET'])
def get_active_clients():
    try:
        return jsonify(active_clients), 200
    except Exception as e:
        logging.error(f"Error getting active clients: {e}")
        return jsonify({"Message": "Failed to get active clients", "status": 500}), 500
    
@app.route('/get_client', methods=['GET'])
def get_client(client_id):
    client = next((c for c in active_clients if c['id'] == client_id), None)
    if client is not None:
        return jsonify(client), 200
    else:
        return jsonify({"Error": "Client not found"}), 404

if __name__ == '__main__':
    # Лог для выявления ошибок сервера (режим Debug)
    logging.basicConfig(filename='flask_errors.log', level=logging.DEBUG)
    app.run(host='0.0.0.0', port=5000, debug=True)