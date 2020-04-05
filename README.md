# az-iot
Azure IoT Projects

## Quantified Desk
Uses arduino mkr1000 and supersonic sensor to measure standing desk height, sending information to Azure IoT Hub and visualizing it in a real-time Dashboard (Python application backend and Pusher/Javascript frontend).

It also detects state changes (sitting -> standing and standing -> sitting) and logs them in a table.

![Sample visualization](https://lh3.googleusercontent.com/-tp2-5dIozVsXVUMPe6DpJnaaKTMX99GvJYOQz4Sen3-1Klc61sHjapQ8OD54iu_Y333_7Hil2pSAra3D_iG-x_VNRvhBY-EXfzEPgB24f6asTIcTTBmk7uTF9-vqTsafs5L2qJwbaOiT-vNA16n7p7mSgegY69Yu7oLWdcrKtX9X9eB9oiglJz66Gul-Tjjrb_LyyU_0aw3Tbh9Aiyog4_kgzbmL7ag3ash57njI44Lj6_6-nTzCZx02YXIZQdCINgqX_qW1s3iolBXy8Ab28HZr64tDAmxhnRSgOjoD1tjzujMCOIJgxJC40JLGuCrqA6ApaU0X5bjpLNzDfmoxytB1EqAswwebDkbwvLgkkr6f5JXVlZea76Pclmcen_shk0WIDBEiIpJighX0bipgZqOtIYc_yspTlfc9Mg7_qQHDzLzfivEvhREeRsbCz7Xt0g_WRBXmgHb3PTlGJ6X7LSz7ha3kib2NNe3uECfv3OgScUHVB9YAATv4oYMLq1haAdi1mZS2bmqMKzAVMEakFKYVYwMjy-N974mVwMG2ZwUrW_OzNBEnF4wEvIb6VA3D25bN7sANW7YAGxluEbzm0E7YlBd8QoA4qVFccsqE1ugABwIvOO6fbYjncUDpFILaloXDJifY-c03SbPC06KYhKSXVGkAoXxLKNUBM5N2g8UGtdgQf8rPg5daczXA_AOKjmWJYNFNezgcHURZ7IQrZ2oRjXbQS7IldmY4IlifwYq-EnNF_wvk34=w1247-h931-no)



Arduino project setup is available in this link:

https://www.hackster.io/vmehmeri/quantified-desk-iot-6647bf

### Requirements
As a pre-requisite, an Azure IoT Hub must be setup and configured to route device messages to an Azure Service Bus queue.

#### Backend:

- azure-servicebus
- pusher

#### Frontend:

- pusher
- flask

All can be installed with pip

#### Arduino:
- Arduino ultrasonic library: https://github.com/JRodrigoTech/Ultrasonic-HC-SR04
- WiFi101 library: https://github.com/arduino-libraries/WiFi101

### Usage
First configure your connection details in 

Start frontend
```
    python frontend/app.py
```

On a separate terminal, start backend
```
    python device_handler.py
```