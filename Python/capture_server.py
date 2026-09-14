from flask import Flask, request
import requests

app = Flask(__name__)

# 🔴 REPLACE THESE 3 VALUES
BOT_TOKEN = "8731055636:AAGoRnLZmzppr0u4GQ1sxuIWwPHeMLUusKA"
CHAT_ID = "5258488910"
ESP32_IP = "http://10.65.167.215" 

# 📸 CAPTURE (photo trigger)
@app.route("/capture")
def capture():
    try:
        cam_url = "http://10.65.167.25//capture"
        
        print("Requesting image from camera...")
        response = requests.get(cam_url)

        print("Camera response status:", response.status_code)

        if response.status_code != 200:
            return "Camera not responding"

        img_data = response.content

        print("Image size:", len(img_data))

        url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendPhoto"

        files = {
            "photo": ("image.jpg", img_data)
        }

        data = {
            "chat_id": CHAT_ID
        }

        tg_response = requests.post(url, files=files, data=data)

        print("Telegram response:", tg_response.text)

        return "Photo Sent"

    except Exception as e:
        print("ERROR:", e)
        return "Error"
# 🔑 SEND OTP TO TELEGRAM
@app.route("/send_otp/<otp>")
def send_otp(otp):
    try:
        url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"

        requests.post(url, data={
            "chat_id": CHAT_ID,
            "text": f"Your OTP is: {otp}"
        })

        return "OTP Sent"
    except Exception as e:
        print(e)
        return "Error"

# 🔘 SEND BUTTON

@app.route("/start")
def start():
    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"

    keyboard = {
        "inline_keyboard": [
            [{"text": "Request OTP", "callback_data": "get_otp"}]
        ]
    }

    requests.post(url, json={
        "chat_id": CHAT_ID,
        "text": "Click to get OTP",
        "reply_markup": keyboard
    })

    return "Button Sent"

# 🔥 HANDLE BUTTON CLICK
@app.route("/callback", methods=["POST"])
def callback():
    data = request.json
    print("Received:", data)

    if "callback_query" in data:
        query = data["callback_query"]

        if query["data"] == "get_otp":
            print("OTP button clicked")

            try:
                # 🔥 CALL ESP32
                response = requests.get(f"{ESP32_IP}/generate")
                print("ESP32 response:", response.status_code)
            except Exception as e:
                print("ESP32 error:", e)

    return "OK"


# 🚀 RUN SERVER
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)