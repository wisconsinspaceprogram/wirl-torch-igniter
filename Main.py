import pygame, sys
import pygame_chart as pyc
from BluetoothConnection import BluetoothConnection
from Plotter import Plotter, PlotterManager
import time
import os

dirname = os.path.dirname(__file__)+"/Images/"
print(dirname)

# Screen and window settings
pygame.init()
pygame.display.set_caption("WIRL Igniter Control")
screen = pygame.display.set_mode((1800,900))
screen.fill((200, 200, 200))


# Buttons
def Buttonify(Picture, coords, size, surface):
    image = pygame.image.load(Picture)
    image = pygame.transform.scale(image, size)
    imagerect = image.get_rect()
    imagerect.topleft = coords
    surface.blit(image,imagerect)
    return (image,imagerect)

#Bluetooth buttons
connectButton = Buttonify(dirname + 'Connect.png',(1100, 25), (150, 50), screen)
disconnectButton = Buttonify(dirname + 'Disconnect.png',(1275, 25), (150, 50), screen)
connectionStatus = Buttonify(dirname + 'Disconnected.png', (1450, 25), (300, 50), screen)

#Data buttons
clearButton = Buttonify(dirname + 'Clear.png',(275, 25), (150, 50), screen)
pauseButton = Buttonify(dirname + 'Pause.png',(450, 25), (150, 50), screen)
playButton = Buttonify(dirname + 'Play.png',(625, 25), (150, 50), screen)

#Vent buttons
ventButton = Buttonify(dirname + 'Vent.png',(1100, 650), (150, 50), screen)
oxVentButton = Buttonify(dirname + 'OxVent.png',(1100, 725), (150, 50), screen)
methVenetButton = Buttonify(dirname + 'MethVent.png',(1100, 800), (150, 50), screen)

# Valve base image
Buttonify(dirname + 'PNID.png', (1000, 100), (800, 548), screen)

# Valve display
valves = [None] * 6
def drawValves(valveStates):
  openPath = dirname + 'OpenValve.png'
  closedPath = dirname + 'ClosedValve.png'

  valves[0] = Buttonify(openPath if valveStates[0] == '1' else closedPath,(1150, 220), (30, 30), screen)[1]
  valves[1] = Buttonify(openPath if valveStates[1] == '1' else closedPath,(1620, 220), (30, 30), screen)[1]
  valves[2] = Buttonify(openPath if valveStates[2] == '1' else closedPath,(1240, 275), (30, 30), screen)[1]
  valves[3] = Buttonify(openPath if valveStates[3] == '1' else closedPath,(1530, 275), (30, 30), screen)[1]
  valves[4] = Buttonify(openPath if valveStates[4] == '1' else closedPath,(1275, 465), (30, 30), screen)[1]
  valves[5] = Buttonify(openPath if valveStates[5] == '1' else closedPath,(1485, 465), (30, 30), screen)[1]

drawValves([0, 0, 0, 0, 0, 0])

#System state display
def updateSystemState(state):
  path = dirname + "State" + str(state) + ".png"
  Buttonify(path, (1150, 550), (150, 50), screen)

updateSystemState(0)

# Computer end state variables
connected = False
collectData = True

bt = BluetoothConnection(0, False)

# Graphs
plotters = PlotterManager([Plotter(screen, "Methane Pre-Valve", "Temperature [C]", 0, 100, 500, 200),
                           Plotter(screen, "Methane Post-Valve", "Temperature [C]", 0, 300, 500, 200),
                           Plotter(screen, "Oxygen Pre-Valve", "Temperature [C]", 0, 500, 500, 200),
                           Plotter(screen, "Oxygen Post-Valve", "Temperature [C]", 0, 700, 500, 200),
                           Plotter(screen, "Methane Pre-Valve", "Pressure [psig]", 500, 100, 500, 200),
                           Plotter(screen, "Methane Post-Valve", "Pressure [psig]", 500, 300, 500, 200),
                           Plotter(screen, "Oxygen Pre-Valve", "Pressure [psig]", 500, 500, 500, 200),
                           Plotter(screen, "Oxygen Post-Valve", "Pressure [psig]", 500, 700, 500, 200)])


# Main loop
while True:
    # # Testing the connection
    # if connected:
    #   if bt.send_line("-"): #Nonesense line to test connection
    #     connected = True #Data sent
    #     connectionStatus = Buttonify('C:/Users/John/OneDrive/Documents/College/AIAA/WIRL/Igniter Control/Images/Connected.png', (1450, 25), (300, 50), screen)
    #   else:
    #     bt.disconnect()
    #     connected = False
    #     connectionStatus = Buttonify('C:/Users/John/OneDrive/Documents/College/AIAA/WIRL/Igniter Control/Images/Disconnected.png', (1450, 25), (300, 50), screen)

    events = pygame.event.get()

    for event in events:
        # Window exit
        if event.type == pygame.QUIT:
          pygame.quit()
          sys.exit()

        # Button clicks
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
          mouse = pygame.mouse.get_pos()

          #Connect button
          if connectButton[1].collidepoint(mouse) and not connected:
            print("Connect")

            try:
              connectionStatus = Buttonify(dirname + 'Connecting.png', (1450, 25), (300, 50), screen)
              pygame.display.update()

              bt = BluetoothConnection('98:D3:51:FE:70:D0', True)
              connected = True
              connectionStatus = Buttonify(dirname + 'Connected.png', (1450, 25), (300, 50), screen)
              plotters.clear()
            except:
              connectionStatus = Buttonify(dirname + 'Disconnected.png', (1450, 25), (300, 50), screen)
              print("Failed to connect")

          #Disconnect button
          if disconnectButton[1].collidepoint(mouse) and connected:
            try:
              bt.disconnect()
              connected = False
              connectionStatus = Buttonify(dirname + 'Disconnected.png', (1450, 25), (300, 50), screen)
            except:
              print("Failed to disconnect")

          #Data clear
          if clearButton[1].collidepoint(mouse):
            plotters.clear()
          
          #Data pause
          if pauseButton[1].collidepoint(mouse):
            collectData = False

          #Data play
          if playButton[1].collidepoint(mouse):
            collectData = True

          #Valve buttons
          for v in range(len(valves)):
            if valves[v].collidepoint(mouse) and connected:
              bt.send_line("s" + str(v+5) + "e")  #s and e used to note start and end of message, so we can only get full messages in arduino

          #vent buttons
          if ventButton[1].collidepoint(mouse) and connected:
              bt.send_line("s4e")  #s and e used to note start and end of message, so we can only get full messages in arduino, state 4

          if oxVentButton[1].collidepoint(mouse) and connected:
              bt.send_line("s2e")  #s and e used to note start and end of message, so we can only get full messages in arduino

          if methVenetButton[1].collidepoint(mouse) and connected:
              bt.send_line("s3e")  #s and e used to note start and end of message, so we can only get full messages in arduino    

    # Reading data
    if connected:
      #Read new data
      new_lines = bt.read_lines()

      #Disconnected detection
      if new_lines == ['']:
        bt.disconnect()
        connected = False
        connectionStatus = Buttonify(dirname + 'Disconnected.png', (1450, 25), (300, 50), screen)

      #Stripping data for each line
      for l in range(len(new_lines)):
        line = new_lines[l]
        data_array = line.split(",")
        print(line)

        #Checking to make sure we have no null values
        fullData = True
        for value in data_array:
          if len(value) == 0:
            fullData = False

        # If right number of datapoints
        if len(data_array) == 11 and fullData:
          #Plotting on graphs
          if collectData:
            plotters.new_data(data_array) #first index is time, 2nd is state (not used), 3rd is valve state (not used), rest are for plotting
        
          #Updating valve symbols
          vavleStates = data_array[2].split(":")
          if len(vavleStates) == 6:
            drawValves(vavleStates)
            
          #System state update
          systemState = int(data_array[1])
          if(systemState >= 0 and systemState <= 10):
            updateSystemState(systemState)

    # Updating visual display
    plotters.update_graphs()
    pygame.display.update()

    
