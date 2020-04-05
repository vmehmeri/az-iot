from azure.servicebus import ServiceBusClient
from azure.servicebus import QueueClient, Message
from pusher import Pusher

import datetime

connect_str = "SERVICE-BUS_CONNECTIONSTRING"
queue_name = "SERVICE-BUS_QUEUENAME"
pusher = Pusher(
    app_id='YOUR_APP_ID',
    key='YOUR_APP_KEY',
    secret='YOUR_APP_SECRET',
    cluster='CLUSTER',
    ssl=True
)

def add_activity(action):
    print("Triggering SIT/STAND event")
    ## get current time minus 1 minute
    _event_time = datetime.datetime.now() - datetime.timedelta(minutes=1)
    year, month, day, hour, minute = _event_time.year, _event_time.month, _event_time.day, _event_time.hour, _event_time.minute
    event_action = "Sat Down" if action == "SIT" else "Stood Up"
    event_date = "%s-%02d-%02d" %(year, month, day)
    event_time = "%02d:%02d" %(hour, minute)
    pusher.trigger(u'activity', u'add', {
        u'date': event_date,
        u'event': event_action,
        u'time':event_time
    })

def update_graph(time_standing, time_sitting):
    print("Updating bar chart")
    pusher.trigger(u'graph', u'update', {
                    u'units': "%s,%s,0" % (time_standing, time_sitting)
    })

current_state = None
previous_time_sitting = previous_time_standing = 0

try:
    print("Connecting to queue", queue_name)

    # Create the QueueClient
    queue_client = QueueClient.from_connection_string(connect_str, queue_name)

    # Receive the message from the queue
    with queue_client.get_receiver() as queue_receiver:
        while True:
            print("Checking messages...")
            messages = queue_receiver.fetch_next(timeout=5)
            for message in messages:
                time_sitting = str(message).split(',')[0]
                time_standing = str(message).split(',')[1]
                print("Time sitting (min)", time_sitting)
                print("Time standing (min)", time_standing)
                print("Current state", current_state if current_state else "Unknown")
                sit_time_int = int(time_sitting)
                stand_time_int = int(time_standing)
                if not current_state and sit_time_int > stand_time_int:
                    current_state = "SIT"
                elif not current_state and stand_time_int > sit_time_int:
                    current_state = "STAND"
                else:
                    if current_state == "STAND" and time_sitting > previous_time_sitting:
                        print("State changed from standing to sitting")
                        current_state = "SIT"
                        add_activity("SIT")
                    elif current_state == "SIT" and time_standing > previous_time_standing:
                        add_activity("STAND")
                        current_state = "STAND"
                        print("State changed from sitting to standing")
                
                previous_time_sitting = time_sitting
                previous_time_standing = time_standing

                # Complete and remove message from queue
                message.complete()

                # Update graph
                update_graph(time_standing, time_sitting)
            
except Exception as ex:
    print('Exception:')
    print(ex)