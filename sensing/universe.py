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
from scipy.optimize import curve_fit
import numpy as np
import math

COMPOUNDS_PATH = 'C:/Fernando/Github/phd_code/sensing/database/spectral/compounds/'

# ------------------------------------------------------------------------------

LOG_PATH = './logs'
LOG_FILENAME = '/universe.log'

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

# ------------------------------------------------------------------------------

""" Augusta Ada King, Countess of Lovelace (10 December 1815 - 27 November 1852) was an English mathematician and writer, chiefly known for her work on Charles Babbage's proposed mechanical general-purpose computer, the Analytical Engine. She was the first to recognise that the machine had applications beyond pure calculation. """
class AdaLovelace:
    def __init__(self):
        
        # Melanin
        # file_path = r'./database/spectral/compounds/eumelanin.csv'
        file_path = COMPOUNDS_PATH + 'eumelanin.csv'
        data_melanin = pd.read_csv(file_path, sep=",", skiprows=1, names= ['wavelength', 'melanin'])

        # Hemoglobin
        # file_path = r'./database/spectral/compounds/hemoglobin.csv'
        file_path = COMPOUNDS_PATH + 'hemoglobin.csv'
        data_hemoglobin = pd.read_csv(file_path, sep="\t", skiprows=18, names= ['wavelength', 'hbo2', 'hb'])

        # Bilirubin
        # file_path = r'./database/spectral/compounds/bilirubin.csv'
        file_path = COMPOUNDS_PATH + 'bilirubin.csv'
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
        self._df_molar_extinction['hemoglobin'] = 0.5*self._df_molar_extinction['desoxyhemoglobin'] + 0.5*self._df_molar_extinction['oxyhemoglobin']

        # ----------------------------------------------------------------------

        # path_depth = r'./database/spectral/skin_penetration.csv'
        path_depth = COMPOUNDS_PATH + '../skin_penetration.csv'
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

        self._lambda_0 = 415
        self._lambda_1 = 445
        self._lambda_2 = 480
        self._lambda_3 = 515
        self._lambda_4 = 555
        self._lambda_5 = 590
        self._lambda_6 = 630
        self._lambda_7 = 680
        self._wavelengths = np.array([self._lambda_1, self._lambda_2, self._lambda_3, self._lambda_4, self._lambda_5, self._lambda_6, self._lambda_7])

        # optical path based on optical penetration on skin (in cm)
        optical_path_factor = 10
        self._d_lambda_0 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_0]/10 
        self._d_lambda_1 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_1]/10 
        self._d_lambda_2 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_2]/10 
        self._d_lambda_3 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_3]/10 
        self._d_lambda_4 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_4]/10 
        self._d_lambda_5 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_5]/10
        self._d_lambda_6 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_6]/10
        self._d_lambda_7 = optical_path_factor*self._df_depth['depth'].loc[self._lambda_7]/10
        
        # Differential pathlength factor
        self._DPF = np.array([self._d_lambda_1, self._d_lambda_2, self._d_lambda_3, self._d_lambda_4, self._d_lambda_5, self._d_lambda_6, self._d_lambda_7])

        # ----------------------------------------------------------------------

        # molar_extinction * 1/Mcm * nm
        self._factor_bili_0 = self._df_molar_extinction['bilirubin'][self._lambda_0]*self._d_lambda_0
        self._factor_bili_1 = self._df_molar_extinction['bilirubin'][self._lambda_1]*self._d_lambda_1
        self._factor_bili_2 = self._df_molar_extinction['bilirubin'][self._lambda_2]*self._d_lambda_2
        self._factor_bili_3 = self._df_molar_extinction['bilirubin'][self._lambda_3]*self._d_lambda_3
        self._factor_bili_4 = self._df_molar_extinction['bilirubin'][self._lambda_4]*self._d_lambda_4
        self._factor_bili_5 = self._df_molar_extinction['bilirubin'][self._lambda_5]*self._d_lambda_5
        self._factor_bili_6 = self._df_molar_extinction['bilirubin'][self._lambda_6]*self._d_lambda_6
        self._factor_bili_7 = self._df_molar_extinction['bilirubin'][self._lambda_7]*self._d_lambda_7

        self._factor_blood_desoxy_0 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_0]*self._d_lambda_0
        self._factor_blood_desoxy_1 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_1]*self._d_lambda_1
        self._factor_blood_desoxy_2 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_2]*self._d_lambda_2
        self._factor_blood_desoxy_3 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_3]*self._d_lambda_3
        self._factor_blood_desoxy_4 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_4]*self._d_lambda_4
        self._factor_blood_desoxy_5 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_5]*self._d_lambda_5
        self._factor_blood_desoxy_6 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_6]*self._d_lambda_6
        self._factor_blood_desoxy_7 = self._df_molar_extinction['desoxyhemoglobin'][self._lambda_7]*self._d_lambda_7

        self._factor_blood_oxy_0 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_1]*self._d_lambda_0
        self._factor_blood_oxy_1 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_1]*self._d_lambda_1
        self._factor_blood_oxy_2 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_2]*self._d_lambda_2
        self._factor_blood_oxy_3 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_3]*self._d_lambda_3
        self._factor_blood_oxy_4 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_4]*self._d_lambda_4
        self._factor_blood_oxy_5 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_5]*self._d_lambda_5
        self._factor_blood_oxy_6 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_6]*self._d_lambda_6
        self._factor_blood_oxy_7 = self._df_molar_extinction['oxyhemoglobin'][self._lambda_7]*self._d_lambda_7

        self._factor_melanin_0 = self._df_molar_extinction['melanin'][self._lambda_0]*self._d_lambda_0
        self._factor_melanin_1 = self._df_molar_extinction['melanin'][self._lambda_1]*self._d_lambda_1
        self._factor_melanin_2 = self._df_molar_extinction['melanin'][self._lambda_2]*self._d_lambda_2
        self._factor_melanin_3 = self._df_molar_extinction['melanin'][self._lambda_3]*self._d_lambda_3
        self._factor_melanin_4 = self._df_molar_extinction['melanin'][self._lambda_4]*self._d_lambda_4
        self._factor_melanin_5 = self._df_molar_extinction['melanin'][self._lambda_5]*self._d_lambda_5
        self._factor_melanin_6 = self._df_molar_extinction['melanin'][self._lambda_6]*self._d_lambda_6
        self._factor_melanin_7 = self._df_molar_extinction['melanin'][self._lambda_7]*self._d_lambda_7
        
        self._eps_bilirubin = np.array([
            self._df_molar_extinction['bilirubin'][self._lambda_0],
            self._df_molar_extinction['bilirubin'][self._lambda_1],
            self._df_molar_extinction['bilirubin'][self._lambda_2],
            self._df_molar_extinction['bilirubin'][self._lambda_3],
            self._df_molar_extinction['bilirubin'][self._lambda_4],
            self._df_molar_extinction['bilirubin'][self._lambda_5],
            self._df_molar_extinction['bilirubin'][self._lambda_6],
            self._df_molar_extinction['bilirubin'][self._lambda_7]
        ])
        self._eps_hemoglobin = np.array([
            self._df_molar_extinction['hemoglobin'][self._lambda_0],
            self._df_molar_extinction['hemoglobin'][self._lambda_1],
            self._df_molar_extinction['hemoglobin'][self._lambda_2],
            self._df_molar_extinction['hemoglobin'][self._lambda_3],
            self._df_molar_extinction['hemoglobin'][self._lambda_4],
            self._df_molar_extinction['hemoglobin'][self._lambda_5],
            self._df_molar_extinction['hemoglobin'][self._lambda_6],
            self._df_molar_extinction['hemoglobin'][self._lambda_7]
        ])
        self._eps_melanin = np.array([
            self._df_molar_extinction['melanin'][self._lambda_0],
            self._df_molar_extinction['melanin'][self._lambda_1],
            self._df_molar_extinction['melanin'][self._lambda_2],
            self._df_molar_extinction['melanin'][self._lambda_3],
            self._df_molar_extinction['melanin'][self._lambda_4],
            self._df_molar_extinction['melanin'][self._lambda_5],
            self._df_molar_extinction['melanin'][self._lambda_6],
            self._df_molar_extinction['melanin'][self._lambda_7]
        ])

        log.debug("Class AdaLovelace created!")
    
    # --------------------------------------------------------------------------

    def compounds_calculation(self, spectra):
        log.debug("Compounds calculation Callback")
        super_equation = np.array([
            # [self._factor_bili_1, self._factor_blood_desoxy_1, self._factor_blood_oxy_1, self._factor_melanin_1], 
            [self._factor_bili_2, self._factor_blood_desoxy_2, self._factor_blood_oxy_2, self._factor_melanin_2],
            [self._factor_bili_4, self._factor_blood_desoxy_4, self._factor_blood_oxy_4, self._factor_melanin_4],
            [self._factor_bili_5, self._factor_blood_desoxy_5, self._factor_blood_oxy_5, self._factor_melanin_5],
            [self._factor_bili_6, self._factor_blood_desoxy_6, self._factor_blood_oxy_6, self._factor_melanin_6]
            ])

        ch_415 = 0
        ch_445 = 1
        ch_480 = 2
        ch_515 = 3
        ch_555 = 4
        ch_590 = 5
        ch_630 = 6
        ch_680 = 7
        
        # Reference raw values from white paper reflection
        ref_415 = 21554.144
        ref_445 = 50745.678
        ref_480 = 17446.882
        ref_515 = 45874.603
        ref_555 = 65512.253
        ref_590 = 60898.035
        ref_630 = 45750.930
        ref_680 = 22980.943

        super_absorption = np.array(
            [spectra["data"][ch_445], 
             spectra["data"][ch_555],
             spectra["data"][ch_590],
             spectra["data"][ch_630]])
        
        x = np.linalg.solve(super_equation, super_absorption)
        # print(x)
        bili_conc = x[0]
        desoxy_conc = x[1]
        oxy_conc = x[2]
        melanin_conc = x[3]
        log.info(f"CONCENTRATIONS in µmol/L (µM): Bilirubin {bili_conc*100e6:3.2f}, Desoxy {desoxy_conc*100}, Oxy{oxy_conc*100}, Melanin {melanin_conc*100}")
        return x

    # --------------------------------------------------------------------------
    
    def compounds_researcher(self, spectra):
        log.debug("Compounds Researcher")
        
        ch_415 = 0
        ch_445 = 1
        ch_480 = 2
        ch_515 = 3
        ch_555 = 4
        ch_590 = 5
        ch_630 = 6
        ch_680 = 7
        
        log.debug("Melanin Evaluation")
        melanin_concentration_630 = spectra["data"][ch_630]/self._factor_melanin_6
        melanin_concentration_680 = spectra["data"][ch_680]/self._factor_melanin_7
        
        log.info(f'Melanin 630: {melanin_concentration_630} M')
        log.info(f'Melanin 680: {melanin_concentration_680} M')
        
        log.debug("Hemoglobin Evaluation")
        hemoglobin_concentration_590 = (spectra["data"][ch_590] - self._factor_melanin_5*melanin_concentration_630)/(self._factor_blood_oxy_5*0.75 + self._factor_blood_desoxy_5*0.25)
        
        log.info(f'Hemoglobin 590: {hemoglobin_concentration_590} M')
        
        log.debug("Bilirubin Evaluation")
        bilirubin_concentration_445 = (spectra["data"][ch_445] - self._factor_melanin_1*melanin_concentration_630 - (self._factor_blood_desoxy_1*0.5 + self._factor_blood_oxy_1*0.5)*hemoglobin_concentration_590)/self._factor_bili_1
        
        log.info(f'Bilirubin 445: {bilirubin_concentration_445} M')
        
    # --------------------------------------------------------------------------
       
    # Define the absorption coefficients for each chromophore (example values)
    def absorption_model(self, wavelength, C_bilirubin, C_oxyhemoglobin, C_desoxyhemoglobin, C_melanin, S, b):
        try: 
            # Sum the absorption contributions
            optical_path_factor = 15
            absorption = (self._df_molar_extinction['bilirubin'][wavelength] * C_bilirubin +
                          self._df_molar_extinction['oxyhemoglobin'][wavelength] * C_oxyhemoglobin + 
                          self._df_molar_extinction['desoxyhemoglobin'][wavelength] * C_desoxyhemoglobin + 
                          self._df_molar_extinction['melanin'][wavelength] * C_melanin) * optical_path_factor * self._df_depth['depth'].loc[wavelength]/10
                          
            # print(f'Absorption: {absorption}')
            
            # Add scattering contribution
            scattering = S * (wavelength ** -b)
            # print(f'Scattering: {scattering}')
            
            # Total attenuation
            return absorption + scattering
        except Exception as e:
            print(f'Error; {e}')
    

    def evaluation(self, measured_attenuation, guess): 
        # return self.absorption_model(415, 2e-6, 0.01, 0.1, 1, 1)
        
        # Fit the model to the data
        popt, pcov = curve_fit(self.absorption_model, self._wavelengths, measured_attenuation,
                            p0=guess, bounds=(0, [1., 1., 1., 1., 1., 1.]), method='trf')
        # Extract the bilirubin concentration and other fitted parameters
        C_bilirubin = popt[0]
        C_oxyhemoglobin = popt[1]
        C_desoxyhemoglobin = popt[2]
        C_melanin = popt[3]
        S = popt[3]
        b = popt[4]

        print(f"Bilirubin concentration: {C_bilirubin}")
        print(f"Oxyhemoglobin concentration: {C_oxyhemoglobin}")
        print(f"Desoxyhemoglobin concentration: {C_desoxyhemoglobin}")
        print(f"Melanin concentration: {C_melanin}")
        print(f"Scattering amplitude (S): {S}")
        print(f"Scattering power (b): {b}")
        
        
        
    
# ----------------------------------------------------------------------------------------------------------------------

if __name__ == "__main__":
    
    # broker_address = "raspberrypi"
    # log.info("MQTT: creating a new MQTT client instance")
    # client = mqtt.Client("Universe")

    # log.info("MQTT: connecting to broker - %s", broker_address)
    # client.connect(broker_address)

    # topic_quality = "spec/reply/quality"
    # log.info("Subscribing to topic %s" %topic_quality)
    # client.subscribe(topic_quality)

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