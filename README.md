
# ORBIT

ORBIT is an ESP32 embedded platform and dashboard system built using ESP-IDF.  
It is designed as a lightweight embedded UI/portal framework featuring:

###### THIS WAS A SEMI COPY OF 'ZAIRE SYSTEM' ANOTHER PROJECT I WORKED ON(MUCH MORE ADVANCED AND BETTER IN MY OPINION). 
###### I COPIED THIS SEVERAL FILES AS A STARTING POINT FOR THE STRUCTURE AND FUNCTIONS OF THIS PROJECT, 
###### AND THEN MODIFIED IT FOR THE ORBIT SYSTEM. 
###### THERE MAY BE SOME UNUSED OR OUTDATED CODE IN THESE FILES, 
###### BUT I LEFT IT IN FOR NOW AS A REFERENCE FOR ANY FUTURE SETTINGS I MAY WANT TO ADD.

- Captive portal setup
- Embedded dashboard UI
- Settings management
- NVS persistent storage
- SPI display support
- Touchscreen support
- Modular widget system
- Device onboarding flow
- Multi-language support
- Custom UI rendering engine

---

# REQUIREMENTS

- ESP-IDF v5.3.x recommended
- ESP32
- SPI LCD Display (ILI9341 currently supported)
- Touch controller support

---

# DISPLAY SUPPORT

ORBIT is currently designed around the popular:

- Cheap Yellow Display (CYD)
- Cheap Yellow Chinese Display variants
- ESP32 + ILI9341 touchscreen boards
<img src="images/YELLO_LCD.png" width="400">

These displays are widely available online and commonly include:
- ILI9341 LCD
- XPT2046 touch controller
- ESP32
- SPI interface

---

# IMPORTANT ESP-IDF CONFIGURATION

Before building ORBIT, open:

```bash
idf.py menuconfig
```

Then change the following settings:

---

## FLASH SIZE

Change:

```txt
2MB -> 4MB
```

Location:

```txt
Serial Flasher Config -> Flash size
```

---

## PARTITION TABLE (VERY IMPORTANT)

Change:

```txt
Single Factory App -> Custom Partition Table (VERY IMPORTANT)
```

Location:

```txt
Partition Table -> Partition Table
```

ORBIT requires a custom partition layout for:
- SPIFFS storage
- UI assets
- future OTA support
- persistent system storage

Failure to change this may cause:
- boot issues
- SPIFFS mount failures
- asset loading failures

---

# BUILD

```bash
idf.py build
```

---

# FLASH

```bash
idf.py flash monitor
```

---

# FEATURES

## Embedded Dashboard
- Real-time UI rendering
- Modular widget architecture
- Dynamic screen updates
- Optimized partial redraw system

## Settings System
- Persistent NVS storage
- User/device configuration
- Theme + language management

## Captive Portal
- Wi-Fi onboarding
- Device configuration
- Embedded web dashboard

## UI Framework
- Custom rendering pipeline
- Widget-based architecture
- Touch interaction support

---

# STATUS

ORBIT is currently in active development.

Some systems are experimental and subject to change.

---

# AUTHOR

Yann Kabambi  
