# Air Sensor V1.0
A small solar powered ZigBee compatible sensor with energy harvesting for temperature, humidity and VOCs.  
Features nRF52840, AEM10941, SHT40 and SGP40.

![Rendering](Hardware/rendering.png)

## Known Flaws
* Due to the extremely high resistor values of the voltage dividers and their corresponding errors, the set voltage limits are very inaccurate. 
* The overall performance is not completely satisfying, especially indoors pretty much no energy is harvested. (Maybe a different chip like TI's [BQ25570](https://www.ti.com/lit/ds/symlink/bq25570.pdf) could perform better.) The Sensirion sensors perform excellently.

## Software
The proper software (that uses ZigBee) is to be developed. The existing software is a very rudimentary Arduino example to test the sensors. It draws way too much power to be used.

## BOM
The interactive BOM can be found [here](https://franz.science/airsensor-v1.0-ibom/).  

## Reference
e-peas Configuration tool (for the resistors) in the resources section: [Product page](https://e-peas.com/product/aem10941/).