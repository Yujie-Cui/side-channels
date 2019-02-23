import sys # command line argument
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

MISS = 0
HIT = 1

csv_file = "histogram.csv"
if len(sys.argv) == 2:
	csv_file = sys.argv[1]

print('Reading %s' % csv_file)
df = pd.read_csv(csv_file);
print(df)
max_cycles = len(df.index)
print('max cycles', max_cycles)

bar_width = 0.05

bars = [None] * 2
bars[MISS] = {}
bars[MISS]["values"] = []

bars[HIT] = {}
bars[HIT]["values"] = []

for index, row in df.iterrows():
	bars[MISS]["values"].append(row["misses"])
	bars[HIT]["values"].append(row["hits"])

bars[MISS]["ticks"] = np.arange(max_cycles)
bars[HIT]["ticks"] = np.arange(max_cycles)

ticks = bars[MISS]['ticks']
values = bars[MISS]['values']
plt.bar(ticks, values, color="red", edgecolor='white', width=bar_width, label='miss')

#ticks = bars[HIT]["ticks"]
#values = bars[HIT]["values"]
#plt.bar(ticks, values, color="blue", edgecolor='white', width=bar_width, label="hit")

#plt.savefig("graph.png", bbox_inches="tight")
plt.show()
