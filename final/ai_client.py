import tensorflow as tf
import keras
from keras._tf_keras.keras.models import load_model
from keras._tf_keras.keras.layers import TFSMLayer
import cv2  # Ensure you have opencv-python installed
import numpy as np
import os
import socket
import sys


from socket_file import socket_func, error

# Disable scientific notation for clarity
def model_func(sock : socket):
    np.set_printoptions(suppress=True)

    # Path to the model file
    model_path = "./model.savedmodel/"

    # Check if the model file exists
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Model file '{model_path}' not found. Please ensure the file is in the correct location.")

    # Load the model
    # model = keras.models.load_model(model_path, compile=False)
    # model = tf.keras.models.load_model(model_path)
    model = tf.saved_model.load("model.savedModel")

    # Load the labels
    labels_path = "labels.txt"

    if not os.path.exists(labels_path):
        raise FileNotFoundError(f"Labels file '{labels_path}' not found. Please ensure the file is in the correct location.")

    class_names = open(labels_path, "r").readlines()

    # CAMERA can be 0 or 1 based on default camera of your computer
    camera = cv2.VideoCapture(0)

    while True:
        # Grab the webcamera's image.
        ret, image = camera.read()

        if not ret:
            print("Failed to grab frame")
            continue

        # Resize the raw image into (224-height,224-width) pixels
        image_resized = cv2.resize(image, (224, 224), interpolation=cv2.INTER_AREA)

        # Show the image in a window
        cv2.imshow("Webcam Image", image_resized)

        # Make the image a numpy array and reshape it to the models input shape.
        image_array = np.asarray(image_resized, dtype=np.float32).reshape(1, 224, 224, 3)

        # Normalize the image array
        image_normalized = (image_array / 127.5) - 1

        # Predicts the model
        # prediction = model.predict(image_normalized)
        prediction = model(image_normalized)
        index = np.argmax(prediction)
        class_name = class_names[index][2]
        confidence_score = prediction[0][index]

        # Print prediction and confidence score
        # print(type(class_name))
        print(f"Class: {class_name.strip()} Confidence Score: {confidence_score:.2%}")

        # Listen to the keyboard for presses.
        keyboard_input = cv2.waitKey(1)

        # 27 is the ASCII for the esc key on your keyboard.
        if keyboard_input == 27:
            break

        buffer = class_name
        # print("--------------\nBUFFER")
        # print(buffer)
        # print(type(buffer))
        # print("--------------")
        if buffer not in ["1","2"]:
            buffer = "0"
        # Sendign socket
        try:
            # Enter priority
            # buffer = input("Enter priority (0-3): ")
            sock.sendall(buffer.encode())


        except socket.error as err:
            error(f"Error writing to socket: {err}")

    camera.release()
    cv2.destroyAllWindows()
    sock.close()



if __name__ == "__main__":
    sock = socket_func()
    model_func(sock=sock)