import pygame, sys
import pygame_chart as pyc
from BluetoothConnection import BluetoothConnection
from Plotter import Plotter
from PlotterManager import PlotterManager
import time
import os
import csv
import datetime

dirname = os.path.dirname(__file__)+"/Images/"
print(dirname)

# Screen and window settings
pygame.init()
pygame.display.set_caption("WIRL Igniter Control")
screen = pygame.display.set_mode((1950,1000))
screen.fill((200, 200, 200))
font = pygame.font.Font(None, 30) 


# Buttons
def Buttonify(Picture, coords, size, surface):
    image = pygame.image.load(Picture)
    image = pygame.transform.scale(image, size)
    imagerect = image.get_rect()
    imagerect.topleft = coords
    surface.blit(image,imagerect)
    return (image,imagerect)

#Bluetooth buttons
connectButton = Buttonify(dirname + 'Connect.png',(1200, 25), (150, 50), screen)
disconnectButton = Buttonify(dirname + 'Disconnect.png',(1375, 25), (150, 50), screen)
connectionStatus = Buttonify(dirname + 'Disconnected.png', (1550, 25), (300, 50), screen)

#Data buttons
newLogButton = Buttonify(dirname + 'NewLog.png',(100, 25), (150, 50), screen)
clearButton = Buttonify(dirname + 'Clear.png',(275, 25), (150, 50), screen)
pauseButton = Buttonify(dirname + 'Pause.png',(450, 25), (150, 50), screen)
playButton = Buttonify(dirname + 'Play.png',(625, 25), (150, 50), screen)

def drawControlButtons():
  #Vent buttons
  global ventButton
  global oxVentButton
  global methVenetButton
  global fireButton
  global dryFireButton
  global sdConnectButton

  ventButton = Buttonify(dirname + 'Vent.png',(1375, 675), (150, 50), screen)
  oxVentButton = Buttonify(dirname + 'OxVent.png',(1375, 750), (150, 50), screen)
  methVenetButton = Buttonify(dirname + 'MethVent.png',(1375, 825), (150, 50), screen)

  #Fire Buttons
  fireButton = Buttonify(dirname + 'Fire.png',(1575, 675), (150, 50), screen)
  dryFireButton = Buttonify(dirname + 'DryFire.png',(1575, 750), (150, 50), screen)
  sdConnectButton = Buttonify(dirname + 'SDLoad.png',(1575, 825), (150, 50), screen)

drawControlButtons()

#Abort button
abortButton = Buttonify(dirname + "Abort.png",(1775, 675), (150, 50), screen)

#Arm Button
armButton = Buttonify(dirname + 'Arm.png',(1175, 675), (150, 50), screen)

# Valve base image
Buttonify(dirname + 'PNID.png', (1150, 100), (800, 548), screen)

# Valve display
valves = [None] * 6
def drawValves(valveStates):
  openPath = dirname + 'OpenValve.png'
  closedPath = dirname + 'ClosedValve.png'

  valves[0] = Buttonify(openPath if valveStates[0] == '1' else closedPath,(1300, 220), (30, 30), screen)[1]
  valves[1] = Buttonify(openPath if valveStates[1] == '1' else closedPath,(1770, 220), (30, 30), screen)[1]
  valves[2] = Buttonify(openPath if valveStates[2] == '1' else closedPath,(1390, 275), (30, 30), screen)[1]
  valves[3] = Buttonify(openPath if valveStates[3] == '1' else closedPath,(1680, 275), (30, 30), screen)[1]
  valves[4] = Buttonify(openPath if valveStates[4] == '1' else closedPath,(1425, 465), (30, 30), screen)[1]
  valves[5] = Buttonify(openPath if valveStates[5] == '1' else closedPath,(1635, 465), (30, 30), screen)[1]

drawValves([0, 0, 0, 0, 0, 0])

#System state display
def updateSystemState(state):
  if(state < 11):
    path = dirname + "State" + str(state) + ".png"
    Buttonify(path, (1175, 750), (150, 50), screen)

def updateSDState(SDState):
  path = dirname + "SD" + str(SDState) + ".png"
  Buttonify(path, (1175, 825), (150, 50), screen)

updateSystemState(0)
updateSDState(0)

#Arm overlay
armedOverlay = Buttonify(dirname + 'Disarmed.png', (1375, 675), (366, 211), screen)

#Sensor calibrate buttons
ptCalRect = pygame.Rect(750, 650, 250, 32)
tcCalRect = pygame.Rect(750, 700, 250, 32)

ptCalText = "PT Calibrate: "
tcCalText = "TC Calibrate: "

ptCalActive = False
tcCalActive = False

def drawCalibrationBoxes():
  global ptCalText
  global tcCalText
  global screen

  ptCalTextSurface = font.render(ptCalText, True, (255, 255, 255)) 
  tcCalTextSurface = font.render(tcCalText, True, (255, 255, 255)) 

  pygame.draw.rect(screen, (150, 150, 150) if ptCalActive else (100, 100, 100), ptCalRect) 
  pygame.draw.rect(screen, (150, 150, 150) if tcCalActive else (100, 100, 100), tcCalRect) 

  screen.blit(ptCalTextSurface, (ptCalRect.x + 5, ptCalRect.y + 5))
  screen.blit(tcCalTextSurface, (tcCalRect.x + 5, tcCalRect.y + 5))

  if not ptCalActive:
    ptCalText = "PT Calibrate: "
  if not tcCalActive:
    tcCalText = "TC Calibrate: "
  

drawCalibrationBoxes()

#Fire duration update buttons
fdUpdateRect = pygame.Rect(750, 750, 250, 32)
fdUpdateText = "Fire Duration: "
fdUpdateActive = False

def drawFDUpdateBox():
  global fdUpdateText
  global screen

  fdUpdateTextSurface = font.render(fdUpdateText, True, (255,255,255))

  pygame.draw.rect(screen, (150, 150, 150) if fdUpdateActive else (100, 100, 100), fdUpdateRect)
  screen.blit(fdUpdateTextSurface, (fdUpdateRect.x + 5, fdUpdateRect.y + 5))

  if not fdUpdateActive:
    fdUpdateText = "Fire Duration: "

drawFDUpdateBox()

# Computer end state variables
connected = False
collectData = True
armed = False

bt = BluetoothConnection(0, False)

# Graphs
plotters = PlotterManager([Plotter(screen, "Methane Pre-Valve", "Temperature [C]", 50, 100, 300, 200, 1),
                           Plotter(screen, "Methane Post-Valve", "Temperature [C]", 50, 300, 300, 200, 1),
                           Plotter(screen, "Oxygen Pre-Valve", "Temperature [C]", 50, 500, 300, 200, 1),
                           Plotter(screen, "Oxygen Post-Valve", "Temperature [C]", 50, 700, 300, 200, 1),
                           Plotter(screen, "Methane Pre-Valve", "Pressure [psia]", 375, 100, 300, 200, 1),
                           Plotter(screen, "Methane Post-Valve", "Pressure [psia]", 375, 300, 300, 200, 1),
                           Plotter(screen, "Oxygen Pre-Valve", "Pressure [psia]", 375, 500, 300, 200, 1),
                           Plotter(screen, "Oxygen Post-Valve", "Pressure [psia]", 375, 700, 300, 200, 1),
                           Plotter(screen, "Methane Mass Flow", "Mass Flow [kg/s]", 700, 100, 300, 200, 0.0001),
                           Plotter(screen, "Oxygen Mass Flow", "Mass Flow [kg/s]", 700, 300, 300, 200, 0.0001)])

#Data logging stuff
filename = os.path.dirname(__file__)+'/Logs/' + str(datetime.datetime.now()).replace(" ", "_").replace(".", "_").replace(":", "_") + '.csv'

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

          #Arm Button
          if armButton[1].collidepoint(mouse) and connected:
            drawControlButtons()
            
            if armed:
              armed = False
              armedOverlay = Buttonify(dirname + 'Disarmed.png', (1225, 675), (366, 211), screen)
            else:
              armed = True
              armedOverlay = Buttonify(dirname + 'Armed.png', (1225, 675), (366, 211), screen)

            
            print(armed)
          #Connect button
          if connectButton[1].collidepoint(mouse) and not connected:
            print("Connect")

            try:
              connectionStatus = Buttonify(dirname + 'Connecting.png', (1550, 25), (300, 50), screen)
              pygame.display.update()

              bt = BluetoothConnection('98:D3:51:FE:70:D0', True)
              connected = True
              connectionStatus = Buttonify(dirname + 'Connected.png', (1550, 25), (300, 50), screen)
              plotters.clear()
            except:
              connectionStatus = Buttonify(dirname + 'Disconnected.png', (1550, 25), (300, 50), screen)
              print("Failed to connect")

          #Disconnect button
          if disconnectButton[1].collidepoint(mouse) and connected:
            try:
              bt.disconnect()
              connected = False
              connectionStatus = Buttonify(dirname + 'Disconnected.png', (1550, 25), (300, 50), screen)
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

          #New Log File
          if newLogButton[1].collidepoint(mouse):
            filename = os.path.dirname(__file__)+'/Logs/' + str(datetime.datetime.now()).replace(" ", "_").replace(".", "_").replace(":", "_") + '.csv'

          #Valve buttons
          for v in range(len(valves)):
            if valves[v].collidepoint(mouse) and connected and armed:
              bt.send_line("s" + str(v+5) + "e")  #s and e used to note start and end of message, so we can only get full messages in arduino

          #vent buttons
          if ventButton[1].collidepoint(mouse) and connected and armed:
            bt.send_line("s4e")  #s and e used to note start and end of message, so we can only get full messages in arduino, state 4

          if oxVentButton[1].collidepoint(mouse) and connected and armed:
            bt.send_line("s2e")  #s and e used to note start and end of message, so we can only get full messages in arduino

          if methVenetButton[1].collidepoint(mouse) and connected and armed:
            bt.send_line("s3e")  #s and e used to note start and end of message, so we can only get full messages in arduino   

          #FIre buttons 
          if fireButton[1].collidepoint(mouse) and connected and armed:
            bt.send_line("s1e")
          
          if dryFireButton[1].collidepoint(mouse) and connected and armed:
            bt.send_line("s11e")

          #Abort Button
          if abortButton[1].collidepoint(mouse) and connected:
            bt.send_line("sAe");

          #SD Reload
          if sdConnectButton[1].collidepoint(mouse) and connected:
            bt.send_line("s12e")

          #Calibrate buttons
          ptCalActive = ptCalRect.collidepoint(mouse)
          tcCalActive = tcCalRect.collidepoint(mouse)
          fdUpdateActive = fdUpdateRect.collidepoint(mouse)
          
        #Watching the user's input to add it to the calibration text window
        if event.type == pygame.KEYDOWN:
          if ptCalActive:
            if event.key == pygame.K_BACKSPACE and ptCalText != "PT Calibrate: ": 
              ptCalText = ptCalText[:-1]
            elif ((event.key == pygame.K_KP_ENTER or event.key == pygame.K_RETURN) and ptCalText != "PT Calibrate: "):
              bt.send_line("s_P" + str(ptCalText[14:]) + "e")
              #print("s_P" + str(ptCalText[14:]) + "e")
              ptCalText = "PT Calibrate: "
            elif not (event.key == pygame.K_KP_ENTER or event.key == pygame.K_RETURN):
              ptCalText += event.unicode

          if tcCalActive:
            if event.key == pygame.K_BACKSPACE and ptCalText != "TC Calibrate: ": 
              tcCalText = tcCalText[:-1]
            elif ((event.key == pygame.K_KP_ENTER or event.key == pygame.K_RETURN) and tcCalText != "TC Calibrate: "):
              bt.send_line("s_T" + str(tcCalText[14:]) + "e")
              #print("s_T" + str(tcCalText[14:]) + "e")
              tcCalText = "TC Calibrate: "
            elif not (event.key == pygame.K_KP_ENTER or event.key == pygame.K_RETURN):
              tcCalText += event.unicode
          
          if fdUpdateActive:
            if event.key == pygame.K_BACKSPACE and fdUpdateText != "Fire Duration: ": 
              fdUpdateText = fdUpdateText[:-1]
            elif ((event.key == pygame.K_KP_ENTER or event.key == pygame.K_RETURN) and fdUpdateText != "Fire Duration: "):
              bt.send_line("s_F" + str(fdUpdateText[15:]) + "e")

              fdUpdateText = "Fire Duration: "
            elif not (event.key == pygame.K_KP_ENTER or event.key == pygame.K_RETURN):
              fdUpdateText += event.unicode
              

    # Reading data
    if connected:
      #Read new data
      new_lines = bt.read_lines()

      #Disconnected detection
      if new_lines == ['']:
        bt.disconnect()
        connected = False
        connectionStatus = Buttonify(dirname + 'Disconnected.png', (1550, 25), (300, 50), screen)

      #Stripping data for each line
      for l in range(len(new_lines)):
        line = new_lines[l]
        data_array = line.split(",")
        print(line)

        #Logging non-empty lines ("" is an empty line so > 2)
        if len(data_array) > 2:
          with open(filename, 'a', newline='', errors='ignore') as csvfile:
            csv_writer = csv.writer(csvfile)
            csv_writer.writerow(data_array, )

        #Checking to make sure we have no null values
        fullData = True
        for value in data_array:
          if len(value) == 0:
            fullData = False

        # If right number of datapoints
        if len(data_array) == 22 and fullData:
          #Plotting on graphs
          if collectData:
            plotters.new_data(data_array) #first index is time, 2nd is state (not used), 3rd is valve state (not used), rest are for plotting, last one is SD data
        
          #Updating valve symbols
          vavleStates = data_array[2].split(":")
          if len(vavleStates) == 6:
            drawValves(vavleStates)
            
          #System state update
          systemState = int(data_array[1])
          if(systemState >= 0 and systemState <= 12):
            updateSystemState(systemState)

          #SD update
          sdLoaded = int(data_array[13])
          if(sdLoaded >= 0 and sdLoaded <= 1):
            updateSDState(sdLoaded)

    # Updating visual display
    drawCalibrationBoxes()
    drawFDUpdateBox()
    plotters.update_graphs()
    pygame.display.update()

    
