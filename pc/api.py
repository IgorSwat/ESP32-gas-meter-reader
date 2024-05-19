from ultralytics import YOLO
import cv2
import pytesseract
import numpy as np
import requests
import logging
import config


# OCR configs
custom_config = r"--oem 3 --psm 6 -c tessedit_char_whitelist=0123456789"

# Logging options
logging.basicConfig(level=logging.ERROR)
logger = logging.getLogger('ppocr')
logger.setLevel(logging.ERROR)


API_URL = f"http://{config.ESP32_API_ADDRESS}/data"

model = YOLO("../model/best.pt")


def read_meter(total_digits, fractional_digits, image_path=None):
    if image_path is None:
        device_response = requests.get(API_URL)

        if device_response.status_code != 200:
            print(f"Error: reading data from device on url {API_URL} failed")
            return None, None

        file_bytes = np.frombuffer(device_response.content, dtype=np.uint8)
        image = cv2.imdecode(file_bytes, 1)
    else:
        image = cv2.imread(image_path, cv2.IMREAD_COLOR)

    results = model.predict(source=image, save=False)
    result = results[0]
    coordinates = result.boxes.xyxy

    try:
        x1, y1, x2, y2 = map(int, coordinates[0])
    except IndexError:
        print("Index error?")
        return None, None

    cropped_image = image[y1:y2, x1:x2]
    cropped_image_rgb = cv2.cvtColor(cropped_image, cv2.COLOR_BGR2RGB)
    img = cv2.cvtColor(cropped_image, cv2.COLOR_BGR2GRAY)

    result = read_image(img, total_digits, adjust_contrast=config.USE_CONTRAST_ADJUSTMENT)
    result = result.replace('\n', '')
    result = result.replace(' ', '')
    print("Before interpretation method:", result)

    return cropped_image_rgb, interpret_readings(result, total_digits, fractional_digits)


def read_image(gray_image, total_digits, adjust_contrast=False):
    if not adjust_contrast:
        return pytesseract.image_to_string(gray_image, config=custom_config)
    else:
        lightness = 0.0
        min_contrast = 1.0
        best_result = ""    # Best result means the result, where number of detected digits is closest to total_digits

        # Try to find the best contrast settings by applying 0.1 difference steps
        for i in range(20):
            contrast = min_contrast + 0.1 * i
            adjusted = cv2.convertScaleAbs(gray_image, alpha=contrast, beta=lightness)
            result = pytesseract.image_to_string(adjusted, config=custom_config)
            if abs(len(best_result) - total_digits) > abs(len(result) - total_digits):
                best_result = result
        return best_result



def interpret_readings(result, total_digits, fractional_digits):
    integral_digits = total_digits - fractional_digits
    if len(result) < integral_digits:
        return None

    value = 0.0
    for i in range(integral_digits):
        value += int(result[i]) * 10 ** (integral_digits - i - 1)
    for i in range(fractional_digits):
        if integral_digits + i >= len(result):
            break
        value += int(result[integral_digits + i]) * 10 ** (-i - 1)

    return value

