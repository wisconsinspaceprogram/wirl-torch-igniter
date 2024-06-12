import bluetooth
import func_timeout

class BluetoothConnection:

  def __init__(self, address, connect):
    self.address = address
    self.socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)

    if connect:
      #Connecting to socket
      print(f"Connecting to device {address}...")

      self.socket.connect((self.address, 1))  # 1 is the port number for Serial Port Profile (SPP)
      print("Connected successfully.")


  #reading the next line output by bluetooth device, note that this will wait until the full line is read
  def read_lines_raw(self):
    line = ""
    while True:
      line += self.socket.recv(1024).decode(errors='ignore')
      if line.endswith("|"):
          return line.split("|")
  
  def read_lines(self):
    try:  
      return func_timeout.func_timeout(4, self.read_lines_raw, [])
    except:
      return ['']
        
  def send_line(self, message):
    try:
       func_timeout.func_timeout(4, lambda: self.socket.send(message), [])
       return True
    except Exception as msg:
       #print(msg)
       return False

  def disconnect(self):
     self.socket.close()

  @staticmethod
  def discover_devices():
    addresses = []

    print("Searching for Bluetooth devices...")
    nearby_devices = bluetooth.discover_devices(duration=4, lookup_names=True, flush_cache=True)
    if not nearby_devices:
        print("No Bluetooth devices found.")
    else:
        print("Found Bluetooth devices:")
        i = 0
        
        for addr, name in nearby_devices:
            #print(f"#{i} - {addr} - {name}")
            i += 1
            addresses.append([i, name, addr])
    return addresses
  
