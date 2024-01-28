import logging
import logging.handlers
import os
import sys
import paho.mqtt.client as mqtt
import pandas as pd 
pd.options.display.float_format = '{:,.2e}'.format
import numpy as np
import json
import time

# ------------------------------------------------------------------------------

LOG_PATH = './logs'
LOG_FILENAME = '/universe.log'
DATA_PATH = './data'

# Set up a specific logger with our desired output level
log = logging.getLogger(__name__)
log.setLevel(logging.DEBUG)
formatter = logging.Formatter("%(asctime)-19s \t %(levelname)-8s \t %(message)s")

# Set logger directory
if not os.path.exists(LOG_PATH):
    os.makedirs(LOG_PATH)
handler = logging.handlers.RotatingFileHandler(
              LOG_PATH+LOG_FILENAME, maxBytes=20000, backupCount=5)
handler.setFormatter(formatter)
log.addHandler(handler)

ch = logging.StreamHandler()
ch.setFormatter(formatter)
log.addHandler(ch)

# ------------------------------------------------------------------------------

broker_address = "raspberrypi"
log.info("MQTT: creating a new MQTT client instance")
client = mqtt.Client("Universe")

log.info("MQTT: connecting to broker - %s", broker_address)
client.connect(broker_address)

topic_quality = "spec/reply/quality"
log.info("Subscribing to topic %s" %topic_quality)
client.subscribe(topic_quality)

# ------------------------------------------------------------------------------

""" Augusta Ada King, Countess of Lovelace (10 December 1815 - 27 November 1852) was an English mathematician and writer, chiefly known for her work on Charles Babbage's proposed mechanical general-purpose computer, the Analytical Engine. She was the first to recognise that the machine had applications beyond pure calculation. """
class AdaLovelace:
    def __init__(self):
        
        # Melanin
        file_path = r'./database/spectral/compounds/eumelanin.csv'
        data_melanin = pd.read_csv(file_path, sep=",", skiprows=1, names= ['wavelength', 'melanin'])

        # Hemoglobin
        file_path = r'./database/spectral/compounds/hemoglobin.csv'
        data_hemoglobin = pd.read_csv(file_path, sep="\t", skiprows=18, names= ['wavelength', 'hbo2', 'hb'])

        # Bilirubin
        file_path = r'./database/spectral/compounds/bilirubin.csv'
        data_bilirubin = pd.read_csv(file_path, sep="\t+", engine='python', skiprows=24, names= ['wavelength', 'bilirubin'])

        min_wavelength = 400
        max_wavelength = 700
        x = np.linspace(min_wavelength, max_wavelength, num=(max_wavelength-min_wavelength)+1, retstep=False)

        self._df_molar_extinction = pd.DataFrame()

        df = pd.DataFrame(
            index = x,
            data={
                'melanin': np.interp(x, data_melanin['wavelength'], data_melanin['melanin']),
                'desoxyhemoglobin': np.interp(x, data_hemoglobin['wavelength'], data_hemoglobin['hb']),
                'oxyhemoglobin': np.interp(x, data_hemoglobin['wavelength'], data_hemoglobin['hbo2']),
                'bilirubin': np.interp(x, data_bilirubin['wavelength'], data_bilirubin['bilirubin'])
            }
        )
        self._df_molar_extinction = pd.concat([self._df_molar_extinction, df], axis=1) 

        # ----------------------------------------------------------------------

        path_depth = r'./database/spectral/skin_penetration.csv'
        data_csv = pd.read_csv(path_depth, sep=",", names= ['wavelength', 'depth'])
        self._df_depth = pd.DataFrame()
        df = pd.DataFrame(
            index = x,
            data={
                'depth': np.interp(x, data_csv['wavelength'], data_csv['depth'])
            }
        )
        self._df_depth = pd.concat([self._df_depth, df], axis=1)

        # ----------------------------------------------------------------------

        self._lambda_1 = 445
        self._lambda_2 = 480
        self._lambda_3 = 515
        self._lambda_4 = 555
        self._lambda_5 = 590
        self._lambda_6 = 630
        # self._lambda_dict = [lambda_1, lambda_2, lambda_3, lambda_4, lambda_5, lambda_6]

        # optical path based on optical penetration on skin (in cm)
        self._d_lambda_1 = 2*self._df_depth['depth'].loc[self._lambda_1]/10 
        self._d_lambda_2 = 2*self._df_depth['depth'].loc[self._lambda_2]/10 
        self._d_lambda_3 = 2*self._df_depth['depth'].loc[self._lambda_3]/10 
        self._d_lambda_4 = 2*self._df_depth['depth'].loc[self._lambda_4]/10 
        self._d_lambda_5 = 2*self._df_depth['depth'].loc[self._lambda_5]/10
        self._d_lambda_6 = 2*self._df_depth['depth'].loc[self._lambda_6]/10

        # ----------------------------------------------------------------------

        self._factor_bili_1 = self._df_molar_extinction['bilirubin'][self._lambda_1]*self._lambda_1
        self._factor_bili_2 = self._df_molar_extinction['bilirubin'][self._lambda_2]*self._lambda_2
        self._factor_bili_3 = self._df_molar_extinction['bilirubin'][self._lambda_3]*self._lambda_3
        self._factor_bili_4 = self._df_molar_extinction['bilirubin'][self._lambda_4]*self._lambda_4
        self._factor_bili_5 = self._df_molar_extinction['bilirubin'][self._lambda_5]*self._lambda_5
        self._factor_bili_6 = self._df_molar_extinction['bilirubin'][self._lambda_6]*self._lambda_6

        self._factor_blood_desoxy_1 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_1]*self._d_lambda_1
        self._factor_blood_desoxy_2 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_2]*self._d_lambda_2
        self._factor_blood_desoxy_3 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_3]*self._d_lambda_3
        self._factor_blood_desoxy_4 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_4]*self._d_lambda_4
        self._factor_blood_desoxy_5 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_5]*self._d_lambda_5
        self._factor_blood_desoxy_6 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_6]*self._d_lambda_6

        self._factor_blood_oxy_1 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_1]*self._d_lambda_1
        self._factor_blood_oxy_2 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_2]*self._d_lambda_2
        self._factor_blood_oxy_3 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_3]*self._d_lambda_3
        self._factor_blood_oxy_4 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_4]*self._d_lambda_4
        self._factor_blood_oxy_5 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_5]*self._d_lambda_5
        self._factor_blood_oxy_6 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_6]*self._d_lambda_6

        self._factor_melanin_1 = self._df_molar_extinction['melanin'][self._lambda_1]*self._d_lambda_1
        self._factor_melanin_2 = self._df_molar_extinction['melanin'][self._lambda_2]*self._d_lambda_2
        self._factor_melanin_3 = self._df_molar_extinction['melanin'][self._lambda_3]*self._d_lambda_3
        self._factor_melanin_4 = self._df_molar_extinction['melanin'][self._lambda_4]*self._d_lambda_4
        self._factor_melanin_5 = self._df_molar_extinction['melanin'][self._lambda_5]*self._d_lambda_5
        self._factor_melanin_6 = self._df_molar_extinction['melanin'][self._lambda_6]*self._d_lambda_6

        log.debug("Class AdaLovelace created!")
    
    # --------------------------------------------------------------------------

    def compounds_calculation(self, spectra):
        log.debug("Compounds calculation Callback")
        super_equation = np.array([[self._factor_bili_2, self._factor_blood_desoxy_2, self._factor_blood_oxy_2, self._factor_melanin_2], [self._factor_bili_4, self._factor_blood_desoxy_4, self._factor_blood_oxy_4, self._factor_melanin_4], [self._factor_bili_5, self._factor_blood_desoxy_5, self._factor_blood_oxy_5, self._factor_melanin_5], [self._factor_bili_6, self._factor_blood_desoxy_6, self._factor_blood_oxy_6, self._factor_melanin_6]])

        # self._lambda_1 = 445
        # self._lambda_2 = 480
        # self._lambda_3 = 515
        # self._lambda_4 = 555
        # self._lambda_5 = 590
        # self._lambda_6 = 630

        super_absorption = np.array([spectra["data"][2], spectra["data"][4], spectra["data"][5], spectra["data"][6]-0.05])
        x = np.linalg.solve(super_equation, super_absorption)
        # print(x)
        bili_conc = x[0]
        desoxy_conc = x[1]
        oxy_conc = x[2]
        melanin_conc = x[3]
        log.info(f"CONCENTRATIONS in mol/L (M): Bilirubin {bili_conc}, Desoxy {desoxy_conc}, Oxy{oxy_conc}, Melanin {melanin_conc}")
        return x
    
# ----------------------------------------------------------------------------------------------------------------------

sensor = AdaLovelace()

def compounds_evaluation(client, userdata, message):
    global sensor
    msg = json.loads(message.payload.decode("utf-8"))
    log.info(f'Compounds Evaluation Callback: Message ({msg})')
    topic = "spec/cmd/compounds"
    reply = sensor.compounds_calculation(msg)
    log.info("MQTT: publishing message to topic %s", topic)
    client.publish(topic, json.dumps(reply.tolist()))
    
client.message_callback_add(topic_quality, compounds_evaluation)
client.loop_start()

# ----------------------------------------------------------------------------------------------------------------------

while True:

    try:
        time.sleep(0.1)
    except KeyboardInterrupt:
        sys.exit(0)
    except Exception as e: 
        log.error(e)
        log.error(f"System error {e}")