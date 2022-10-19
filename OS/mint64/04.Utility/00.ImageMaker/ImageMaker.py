import sys
import struct

if len(sys.argv) < 4 :
    print("[Usage]: python3 ImageMaker.py BootLoader.bin Kernel32.bin Kernel64.bin")
    sys.exit(1)

print("")

with open("Disk.img", "wb") as DiskImg:
    
    print("[INFO] Read BootLoader.bin")
    with open(sys.argv[1], "rb") as f:
        BootLoader = f.read()
    
    print("[INFO] Read Kernel32.bin")
    with open(sys.argv[2], "rb") as f:
        Kernel32 = f.read()

    print("[INFO] Read Kernel64.bin")
    with open(sys.argv[3], "rb") as f:
        Kernel64 = f.read()
    
    print("[INFO] Adjust BootLoader.bin and Kernel32.bin and Kernel64.bin")
    if len(BootLoader) % 512 != 0:
        BootLoader += b'\x00' * ( 512 - (len(BootLoader) % 512) )
    
    if len(Kernel32) % 512 != 0:
        Kernel32 += b'\x00' * ( 512 - (len(Kernel32) % 512) )
    
    if len(Kernel64) % 512 != 0:
        Kernel64 += b'\x00' * ( 512 - (len(Kernel64) % 512) )
    
    print("[INFO] Start to write kernel information to BootLoader.bin")
    DiskImgData = BootLoader + Kernel32 + Kernel64
    # Read Kernel total sector count and Protect Mode kernel sector count
    TotalSectorCount = int(len(Kernel32) / 512)
    ProtectModeSectorCount = int(len(Kernel64) / 512) 
    
    TotalSectorCountRaw = struct.pack("<H", TotalSectorCount)
    ProtectModeSectorCountRaw = struct.pack("<H", ProtectModeSectorCount)

    # Update TOTALSECTORCOUNT, ProtectModeSectorCount
    DiskImgData = DiskImgData[:5] + TotalSectorCountRaw + ProtectModeSectorCountRaw + DiskImgData[9:]

    print("[INFO] Copy BootLoader.bin and Kernel32.bin, Kernel64.bin to Disk.img")
    DiskImg.write(DiskImgData)