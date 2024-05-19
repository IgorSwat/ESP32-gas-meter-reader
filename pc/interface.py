import time

import streamlit as st

import config
from api import read_meter


class Interface:
    reader_running = False

    def __init__(self):
        st.title("ESP32-cam Gas meter reading")

        self.total_digits = st.number_input('Enter total number of digits on gas meter:')
        self.fractional_digits = st.number_input('Enter number of fractional digits on gas meter:')

        start_button_col, stop_button_col, state_col = st.columns(3)
        with start_button_col:
            self.run_button = st.button('Start')
        with stop_button_col:
            self.stop_button = st.button('Stop')
        with state_col:
            self.state_field = st.markdown('<p style="color: red;">Reader off</p>', unsafe_allow_html=True)
        if self.run_button:
            if not self.reader_running and self.__check_input():
                self.reader_running = True
                self.state_field.markdown('<p style="color: green;">Reader on</p>', unsafe_allow_html=True)
        if self.stop_button:
            if self.reader_running:
                self.reader_running = False
                self.state_field.markdown('<p style="color: red;">Reader off</p>', unsafe_allow_html=True)

        image_col, text_col = st.columns(2)
        with image_col:
            self.image_field = st.empty()
        with text_col:
            self.results_field = st.text("No results just yet")

        self.test_button = None

    def run(self):
        while True:
            if not self.reader_running:
                time.sleep(1)
                continue

            image, results = read_meter(int(self.total_digits), int(self.fractional_digits))
            print("Meter readings:", results, "m^3")
            self._show_results(image, results)

            time.sleep(config.REQUEST_DELAY)

    def _show_results(self, image, results):
        if image is not None:
            self.image_field.image(image, channels="RGB", caption="Meter readings")
            # self.image_field.image(image, caption="Meter readings")
        else:
            self.image_field.empty()
        if results is not None:
            self.results_field.text(f"Meter readings: {results} m^3")
        else:
            self.results_field.text("Unable to read value from meter")

    def __check_input(self):
        if not self.total_digits or self.total_digits <= 0:
            st.write("Invalid input: total number of digits")
            return False
        if not self.fractional_digits or self.fractional_digits < 0 or self.fractional_digits > self.total_digits:
            st.write("Invalid input: number of fractional digits")
            return False
        return True