# WIRL Torch Igniter Control Program

## How to Use

### 1. Configure the Device

To configure the device, you'll need your computer to recognize it as a valid Bluetooth device. The Bluetooth module the igniter uses (HC-06) may not show up in your Bluetooth device connect menu. Before trying to connect to the device, ensure everything is connected, and the red light is blinking on the Bluetooth module (indicating it's in pairing mode).

Two (of many) ways to configure this device if it's not showing up in your menu are:

1. Go into your file explorer, right-click on any file, and find the "send to" option. Then, in the Bluetooth section, see if the HC-06 device is there. If so, click on that. The file sending won't go through, but that's okay; it seems more devices (with maybe less formal or strong signal) show up here.
   
2. Download the BT-Serial Terminal app from the Windows Store and use that to scan for devices. Then, again try to connect to HC-06.

The device password is **1234**; you should only need to input this once.

### 2. Configure Your Environment

Firstly, make sure you have the following files downloaded in the same folder:

- Images (ensure you have all the images in here)
- Main.py
- Plotter.py
- PlotterManager.py
- BluetoothConnection.py

The script will create a Logs folder at this directory which logs every line of data sent from the igniter. Please don't push these back to GitHub; the gitignore should take care of this, but keep an eye on it.

You'll need to install the following Python libraries:

- pygame
- pygame_chart
- csv
- bluetooth (pybluez, use "pip install git+https://github.com/pybluez/pybluez.git#egg=pybluez")
- func_timeout

That should be all, but if an error comes up saying you need another package, install it and add it to this readme.

Finally, you should be able to run Main.py and see some charts populated with just a line which is the default state, Bluetooth connection buttons saying disconnected, and a schematic of the igniter. 

### 3. Running the Igniter

In order to have any control over the igniter, you'll need to connect to it via Bluetooth. Use the "Connect" button in the top right; it may take an attempt or two to successfully connect. Restarting this app can also help with any issues connecting. Once the status changes to green, you're ready to control the system.

The main interface is the PNID diagram; each red/green dot represents a valve in the system. When clicked, these will toggle. Note that the displayed state (red for closed, green for open) is transmitted from the Arduino to this script, so this should accurately reflect what the igniter is setting the valve to.

You can also use the buttons below the PNID to execute pre-programmed sequences. Note these sequences are saved on the Arduino, so if the connection is lost during a test fire, for example, it will complete its cycle and set all valves to closed at the end. Note that the fire button will fire the igniter; be careful where you click!

Before running a true test, click the new log button to start a new log file and make it easier to access the test's telemetry.

Anyways, if you have any other questions, ask John.

Also please add any info / descriptions you have to this readme about the project, this is by far a full outline of the project as it stands :) 