#!/usr/bin/python

from PIL import Image
from sys import argv

IMAGE_BASE_WIDTH = 75
SCALE_IMAGE_BY = 6

PAGE_SIZE = 4096
PAGE_SHIFT = 12

class AllocationVisualizer:
    def __init__(self):
        self.map = {}
        self.lowaddr = 999999999
        self.highaddr = 0

    def mark(self, addr, size, add):
        pageNum = self.pageForAddr(addr)
        pageAddr = self.pageStartForAddr(addr)

        # store off low and high addr's we've seen
        if pageAddr < self.lowaddr:
            self.lowaddr = pageAddr
        if pageAddr > self.highaddr:
            self.highaddr = pageAddr

        firstPageSize = min(size, PAGE_SIZE - (addr - pageAddr))

        if not self.map.has_key(pageNum):
            self.map[pageNum] = 0

        self.map[pageNum] += add * self.factor(firstPageSize)

        if self.map[pageNum] > 1.0:
            print self.map[pageNum], pageNum, add * self.factor(firstPageSize), firstPageSize

        size -= firstPageSize
        addr += firstPageSize

        if self.map[pageNum] > 1.0 or self.map[pageNum] < -1.0:
            raise Exception

        while size > 0:
            pageNum = self.pageForAddr(addr)
            pageAddr = self.pageStartForAddr(addr)

            if pageAddr < self.lowaddr:
                self.lowaddr = addr
            if pageAddr > self.highaddr:
                self.highaddr = addr

            try:
                self.map[pageNum] += add * self.factor(min(PAGE_SIZE, size))
            except KeyError:
                self.map[pageNum] = add * self.factor(min(PAGE_SIZE, size))

            if self.map[pageNum] > 1.0 or self.map[pageNum] < -1.0:
                raise Exception

            #print addr, size, add, add * (min(size, PAGE_SIZE) / float(PAGE_SIZE))
            size -= PAGE_SIZE
            addr += PAGE_SIZE

    def factor(self, size):
        return size / float(PAGE_SIZE)

    def alloc(self, addr, len):
        self.mark(addr, len, 1)

    def free(self, addr, len):
        self.mark(addr, len, -1)

    def pageForAddr(self, addr):
        return addr >> PAGE_SHIFT

    def pageStartForAddr(self, addr):
        return self.pageForAddr(addr) * PAGE_SIZE

    def makeImage(self, imSide = 256):
        keys = self.map.keys()
        keys.sort()

        height = (((self.highaddr - self.lowaddr) / PAGE_SIZE) / imSide) + 1
        im = Image.new("RGBA", (imSide, height), (0,255,255,255))
        pix = im.load()
        print imSide, height

        for k in keys:
            c = int(k - self.pageForAddr(self.lowaddr))
            val = self.map[k]
            x = c % imSide
            y = c / imSide

            if val == 1.0:
                colour = (0, 0, 0, 255)
            elif val == -1.0:
                colour = (119, 255, 187, 255)
            else:
                val += 1
                val /= 2
                z = int(255 - (255 * val))
                colour = (z, z, z, 255)

            pix[x, y] = colour

        return im

def main():
    map = AllocationVisualizer()

    imageIndex = 1
    opcount = 0
    counter = 0
    if len(argv) == 1:
        nr_of_img = 1
    elif len(argv) == 2:
        nr_of_img = int (argv[1])
    else:
        print "You must pass only one argument: the number of images you want to generate"
        raise Exception

    f = open("trace-out.txt", "r")

    for line in f:
        data = line.strip().split('\t')
        if data[0] != "USED" and data[0] != "FREE":
            continue
        opcount = opcount + 1
    step = opcount / nr_of_img
    
    f.seek (0,0)
    for line in f:
        data = line.strip().split('\t')
        if data[0] != "USED" and data[0] != "FREE":
            continue

        counter = counter + 1
        state = (data[0] == "USED")
        addr = int(data[1], 16)
        op = int(data[2])
        if state > 0:
            map.alloc(addr, op)
        else:
            map.free(addr, op)
        if counter == imageIndex * step and imageIndex != nr_of_img:
            im = map.makeImage(IMAGE_BASE_WIDTH)
            newsize = im.size
            w = newsize[0] * SCALE_IMAGE_BY
            h = newsize[1] * SCALE_IMAGE_BY
            im = im.resize((w, h), Image.NEAREST)
            im.save("mem%03d.png" % (imageIndex,), "PNG")
            imageIndex = imageIndex + 1
            
    f.close()

    im = map.makeImage(IMAGE_BASE_WIDTH)
    newsize = im.size
    w = newsize[0] * SCALE_IMAGE_BY
    h = newsize[1] * SCALE_IMAGE_BY
    im = im.resize((w, h), Image.NEAREST)
    im.save("mem%03d.png" % (imageIndex,), "PNG")

main()
