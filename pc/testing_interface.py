from interface import Interface
from api import read_meter

import time


class TestingInterface(Interface):
    def __init__(self, test_img_path):
        super().__init__()
        self.test_img_path = test_img_path

    def run(self):
        while not self.reader_running:
            time.sleep(1)
        image, results = read_meter(int(self.total_digits), int(self.fractional_digits), self.test_img_path)
        print("Meter readings:", results, "m^3")
        self._show_results(image, results)