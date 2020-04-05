from flask import Flask, render_template, request
from pusher import Pusher

# configure pusher object
pusher = Pusher(
    app_id='YOUR_APP_ID',
    key='YOUR_APP_KEY',
    secret='YOUR_APP_SECRET',
    cluster='CLUSTER',
    ssl=True
)

app = Flask(__name__)

@app.route('/')
def index():
      return render_template('dashboard.html')

if __name__ == '__main__':
      app.run(debug=True)
     