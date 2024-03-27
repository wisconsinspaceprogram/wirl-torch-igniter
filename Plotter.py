import pygame, sys
import pygame_chart as pyc

class Plotter:
  
  def __init__(self, screen, name, ylab, x, y, width, height):
    self.name = name
    self.datax = []
    self.datay = []
    self.x = x
    self.y = y
    self.width = width
    self.height = height
    self.ylab = ylab

    self.fig = pyc.Figure(screen, x, y, width, height, bg_color=(200, 200, 200))
    self.fig.add_gridlines()
    self.fig.add_title(self.name)
    self.fig.add_yaxis_label(self.ylab)

  def new_data(self, x, y):
    self.datax.append(float(x))
    self.datay.append(float(y))

    if len(self.datax) > 100:
      self.datax.pop(0)
      self.datay.pop(0)

  def update(self):
    if len(self.datax) > 4 and len(self.datay) > 4:
      self.fig.line(self.name, self.datax, self.datay)
      self.fig.set_ylim((min(self.datay)-1, max(self.datay)+1))
      self.fig.draw() 
    
    else:
      self.fig.line(self.name, [0, 1], [0, 1])
      self.fig.draw() 

  def clear(self):
    self.datax = []
    self.datay = []

