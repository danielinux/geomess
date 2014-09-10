import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys

maxrange = 0
minrange = 0
fig1 = plt.figure()
ax = fig1.add_subplot(111, aspect = "equal")


def addnode(x,y,r, minrange, maxrange):
    ax.plot([x],[y], 'ro')
    circle=plt.Circle((x,y),r,color='g', fill=False)
    fig1.gca().add_artist(circle)
    if maxrange < x + r:
        maxrange = x + r
    if maxrange < y + r:
        maxrange = y + r
    if minrange > x - r:
        minrange = x - r
    if minrange > y - r:
        minrange = y - r
    return minrange,maxrange


f = open("map.csv") or sys.exit()
while True:
    l = f.readline()
    if l == "":
        break
    pos = l.split(',')
    minrange,maxrange = addnode(int(pos[0]), int(pos[1]), int(pos[2]), minrange,maxrange)
f.close()

print "min max:",
print minrange,
print " ",
print maxrange
ax.set_xlim((minrange,maxrange))
ax.set_ylim((minrange,maxrange))


plt.show()
