# WIP! [todo: setup repository :)]

# LoRa Turf Roof Sensor

0. order new antenna for base station
1. test for range
1.5 adjust SF in code

2. if soil moisture is no data use -2 so we can know if its really 0 or not | -1 out of range, -2 no data (*), 0 is 0
2. log soil moisture actual analog data

3. calibrate actual soil moisture
4. compare data with MET

### Open Questions
- Evaluate waterproofing solutions for the sensor hardware.
- Assess whether using raw soil moisture data provides better results.

## Project To-Do
### High Priority
1. **LoRa Settings Optimization:**  
    - Current configuration prioritizes speed over range, resulting in limited communication distance (line of sight or a few meters).
    - Action: Adjust LoRa parameters to increase range, understanding this may reduce data transmission speed.

### Medium/Low Priority
- Implement a special LoRa command to prevent the device from entering sleep mode, and add a command to return it to sleep mode.
- Monitor the number of message retransmissions ("bounces") to detect potential issues.
- Graph backoff and timeout events for inclusion in the thesis.

### Procurement
- Purchase additional 18650 batteries (Hafnarfjörður).
- Acquire coin cell batteries.

### Refunds
- Initiate a refund for the SD card reader purchased from AliExpress.

## 📄 License
MIT License.

## 👤 Author
Jezreel Tan
📧 jvt6@hi.is

Testing Data | around 86m wow so bad

#define FREQUENCY     433E6
#define TX_POWER      20
#define BANDWIDTH     62.5E3
#define SPREAD_FACTOR 10 // maybe 9 is ok?
#define CODING_RATE   5
#define PREAMBLE      8
#define SYNC_WORD     0x12
crc on! send asyc false!