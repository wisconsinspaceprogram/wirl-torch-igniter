class PlotterManager:
  def __init__(self, plotters):
    self.plotters = plotters

  def new_data(self, data_array):
    for p in range(len(self.plotters)):
      self.plotters[p].new_data(data_array[0], data_array[p+3]) #Skipping index 1 and 2, not data

  def update_graphs(self):
    for p in range(len(self.plotters)):
      self.plotters[p].update()

  def clear(self):
    for p in range(len(self.plotters)):
      self.plotters[p].clear()