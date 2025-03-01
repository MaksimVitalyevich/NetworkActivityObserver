import tkinter as tk
from tkinter import ttk
from tkinter import scrolledtext
import os
import sqlite3
import requests
import cv2
from PIL import Image, ImageTk
import re

# определяем путь к папке со скриншотами
UPLOAD_FOLDER = 'screenshots'
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

# Подключение к базе данных SQLite
DATABASE_FILE = 'clients.db'
conn = sqlite3.connect(DATABASE_FILE)
cursor = conn.cursor()

# Создание таблицы, если она не существует
create_table_query = """
CREATE TABLE IF NOT EXISTS client_activity (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id TEXT NOT NULL,
    hostname TEXT NOT NULL,
    ip TEXT NOT NULL,
    username TEXT NOT NULL,
    mouse_activity TEXT,
    keyboard_activity TEXT,
    screenshot_path TEXT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMPЫ
)
"""
cursor.execute(create_table_query)
conn.commit()

def is_valid_screenshot_name(filename):
    # Проверка правильности имени поступающего скриншота ( в формате YYYY-MM-DD_HH-MM-SS.bmp)
    pattern = r"^\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.png$"
    if re.match(pattern, filename):
        return True
    else:
        print(f"Invalid Screenshot name: {filename}")
        return False
    
def execute_query(query, params):
    try:
        cursor.execute(query, params)
        conn.commit()
        return cursor.fetchall()
    except sqlite3.Error as e:
        print(f"SQL Error: {e}")
        return None

# Функция для вставки нового клиента или обновления существующего
def insert_or_update_client(client_id, hostname, ip, username):
    select_query = "SELECT * FROM client_activity WHERE client_id = ?"
    existing_client = execute_query(select_query, (client_id,))
    if existing_client:
        update_query = """
        UPDATE client_activity
        SET hostname = ?, ip = ?, username = ?
        WHERE client_id = ?
        """
        execute_query(update_query, (client_id, hostname, ip, username))
    else:
        insert_query = """
        INSERT INTO client_activity (client_id, hostname, ip, username)
        VALUES (?, ?, ?, ?)
        """
        execute_query(insert_query, (client_id, hostname, ip, username))

# Функция для получения всех клиентов из базы данных
def get_clients():
    select_all_query = "SELECT * FROM client_activity"
    return execute_query(select_all_query, ())

# Функция для получения скриншотов клиента из папки
def get_screenshots(client_id):
    screenshots = [f for f in os.listdir(UPLOAD_FOLDER) if f.startswith(f"screenshot_{client_id}") and is_valid_screenshot_name(f)]
    return sorted(screenshots)

# Функция для отправки данных клиента на сервер Flask
def send_client_data_to_server():
    client_data = {
        "client_id": "MaksVit",
        "hostname": "HOME-PC",
        "ip": "192.168.56.1",
        "username": "MaksimVitalyevich",
        "keyboard_activity": "Activity log started",
        "mouse_activity": "Activity log started"
    }
    try:
        response = requests.post('http://127.0.0.1:5000/add_client', json=client_data)
        if response.status_code == 200:
            print("Client data sent successfully.")
        else:
            print(f"Failed to send client data. Status code: {response.status_code}")
    except requests.ConnectionError as e:
        print(f"Connection error: {e}")

# Создание основного окна приложения
root = tk.Tk()
root.title("Active Clients Viewer")
root.geometry("800x600")
root.resizable(False, False) # Фиксированный размер окна

# Отправка данных клиента при запуске приложения
send_client_data_to_server()

# Создание элементов интерфейса
tree = ttk.Treeview(root, columns = ('ID', 'Client ID', 'Hostname', 'IP', 'Username', 'Mouse Activity', 'Keyboard Activity', 'Screenshot Path', 'Timestamp'), show='headings')
tree.heading('ID', text = 'ID')
tree.heading('Client ID', text = 'Client ID')
tree.heading('Hostname', text = 'Hostname')
tree.heading('IP', text = 'IP')
tree.heading('Username', text = 'Username')
tree.heading('Mouse Activity', text = 'Mouse Activity')
tree.heading('Keyboard Activity', text = 'Keyboard Activity')
tree.heading('Screenshot Path', text = 'Screenshot Path')
tree.heading('Timestamp', text = 'Timestamp')
tree.pack(padx = 10, pady = 10, expand = True, fill = tk.BOTH)

def show_clients():
    try:
        response = requests.get('http://127.0.0.1:5000/get_active_clients')
        if response.status_code == 200:
            active_clients = response.json()
            tree.delete(*tree.get_children())  # очистка текущего списка клиентов
            for client in active_clients:
                # Проверка, что все ключи имеются в словаре клиента
                values = (
                    client.get('id', ''),
                    client.get('client_id', ''),
                    client.get('hostname', ''),
                    client.get('ip', ''),
                    client.get('username', ''),
                    client.get('mouse_activity', ''),
                    client.get('keyboard_activity', ''),
                    client.get('screenshot_path', ''),
                    client.get('timestamp', '')
                )
                tree.insert('', 'end', values = values)
        else:
            print(f"Error fetching active clients. Status code: {response.status_code}")
    except requests.ConnectionError as e:
        print(f"Connection error: {e}")

# Создание кнопки для отображения клиентов
show_clients_button = tk.Button(root, text="Show Clients", command = show_clients)
show_clients_button.pack(pady = 10)

image_window = None
screenshot_index = 0
current_screenshots = []
resize_image_job = None # Initialize the resize_image_job variable

# Создание окна для отображения изображений
def show_image():
    global image_window, screenshot_index, current_screenshots
    if image_window:
        image_window.destroy()

    image_window = tk.Toplevel(root)
    image_window.title("Screenshot Viewer")
    image_window.geometry("600x400")
    image_window.resizable(False, False) # Фиксируем размер окна

    img_label = tk.Frame(image_window)
    img_label.pack(expand=True, fill = tk.BOTH)

    def update_image():
        if not current_screenshots:
            print("No screenshots available for display.")
            return
    
        img_path = os.path.join(UPLOAD_FOLDER, current_screenshots[screenshot_index])
        image = cv2.cvtColor(cv2.imread(img_path), cv2.COLOR_BGR2RGB)
        width, height = img_label.winfo_width(), img_label.winfo_height()
        photo = ImageTk.PhotoImage(Image.fromarray(cv2.resize(image, (width, height))))
        img_label.config(image = photo)
        img_label.image = photo

    # Кнопки навигации
    def show_next_image():
        global screenshot_index
        if screenshot_index < len(current_screenshots) - 1:
            screenshot_index += 1
            update_image()

    def show_previous_image():
        global screenshot_index
        if screenshot_index > 0:
            screenshot_index -= 1
            update_image()

    button_frame = tk.Frame(image_window)
    button_frame.pack(side = tk.TOP, fill = tk.X)

    tk.Button(button_frame, text = "Previous", command = show_previous_image).pack(side=tk.LEFT, padx=10, pady=10)
    tk.Button(button_frame, text = "Next", command = show_next_image).pack(side=tk.RIGHT, padx=10, pady=10)

    img_label.bind('<Configure>', lambda event: update_image())

    update_image()

# Обработчик двойного клика по строке таблицы
def on_double_click(event):
    global screenshot_index, current_screenshots
    item = tree.selection()[0]
    if not item:
        print("No item selected.")
        return
    client_id = tree.item(item, 'values')[1]
    current_screenshots = get_screenshots(client_id)
    if current_screenshots:
        screenshot_index = 0
        show_image()
    else:
        print(f"No screenshots found for client_id: {client_id}")

tree.bind('<Double-1>', on_double_click)

root.mainloop()
