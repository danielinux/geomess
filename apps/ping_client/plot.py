import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys

maxrange = 0
minrange = 0
fig1 = plt.figure()
ax = fig1.add_subplot(111, aspect = "equal")


def addnode(x,y,rm,rg, minrange, maxrange):
    ax.plot([x],[y], 'ro')
    circle=plt.Circle((x,y),rm,color='r', fill=False)
    circleg=plt.Circle((x,y),rg,color='g', fill=False)
    fig1.gca().add_artist(circle)
    fig1.gca().add_artist(circleg)
    if maxrange < x + rm:
        maxrange = x + rm
    if maxrange < y + rm:
        maxrange = y + rm
    if minrange > x - rm:
        minrange = x - rm
    if minrange > y - rm:
        minrange = y - rm
    return minrange,maxrange


f = open("map.csv") or sys.exit()
while True:
    l = f.readline()
    if l == "":
        break
    pos = l.split(',')
    minrange,maxrange = addnode(int(pos[0]), int(pos[1]), int(pos[2]), int(pos[3]), minrange,maxrange)
f.close()

print("min max:",minrange," ",maxrange)
ax.set_xlim((minrange,maxrange))
ax.set_ylim((minrange,maxrange))


plt.show()
