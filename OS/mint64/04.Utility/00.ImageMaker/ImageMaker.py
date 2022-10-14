import sys
import struct

if len(sys.argv) < 3 :
    print("[Usage]: ImageMaker BootLoader.bin Kernel32.bin")
    sys.exit(1)

print("")

with open("Disk.img", "wb") as DiskImg:
    
    print("[INFO] Read BootLoader.bin")
    with open(sys.argv[1], "rb") as f:
        BootLoader = f.read()
    
    print("[INFO] Read Kernel32.bin")
    with open(sys.argv[2], "rb") as f:
        Kernel32 = f.read()
    
    print("[INFO] Adjust BootLoader.bin and Kernel32.bin")
    if len(BootLoader) % 512 != 0:
        BootLoader += b'\x00' * ( 512 - (len(BootLoader) % 512) )
    
    if len(Kernel32) % 512 != 0:
        Kernel32 += b'\x00' * ( 512 - (len(Kernel32) % 512) )
    
    print("[INFO] Start to write kernel information to BootLoader.bin")
    DiskImgData = BootLoader + Kernel32
    SectorCount = int(len(Kernel32) / 512)  # Read Kernel total sector count
    SectorCountRaw = struct.pack("<H", SectorCount)

    # Update TOTALSECTORCOUNT
    DiskImgData = DiskImgData[:5] + SectorCountRaw + DiskImgData[7:]

    print("[INFO] Copy BootLoader.bin and Kernel32.bin to Disk.img")
    DiskImg.write(DiskImgData)