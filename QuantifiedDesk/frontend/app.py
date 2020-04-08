from flask import Flask, render_template, request
from pusher import Pusher

# configure pusher object
pusher = Pusher(
    app_id='975627',
    key = "9bef95b942738c125933",
    secret = "e5c2ebdfaaf632910f6f",
    cluster = "eu",
    ssl=True
)

app = Flask(__name__)

@app.route('/')
def index():
      return render_template('dashboard.html')

if __name__ == '__main__':
      app.run(debug=True)
     