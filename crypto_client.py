import sys
import struct
import socket
import binascii

def do_crypto(data, keyslot, algo, iv, keyY):
    meta = struct.pack('<IIIIsss', 0xCAFEBABE, len(data), keyslot, algo, keyY, iv, '0x00'*0x3D0)
    sock = socket.create_connection(('192.168.1.137', 8081))
    
    sock.send(meta)
    
    bufsize = struct.unpack('<I', sock.recv(4))[0]
    print 'Data packet size is %X bytes.' % bufsize
    
    ofs = 0
    outdata = ''
    while ofs < len(data):
        if ofs + bufsize < len(data):
            sock.send(data[ofs:ofs+bufsize])
            outdata += sock.recv(bufsize)
        else:
            sock.send(data[ofs:])
            outdata += sock.recv(len(data) - ofs)
        ofs += bufsize
    return outdata
    

if __name__ == '__main__':
    argc = len(sys.argv)
    argv = sys.argv
    if argc < 5 or argc > 7:
        print 'Usage: %s: in_file out_file keyslot algo [iv] [keyY]' % argv[0]
        sys.exit(-1)
    
    try:
        with open(argv[1], 'rb') as f:
            in_file = f.read()
    except IOError:
        print 'Failed to read %s!' % argv[1]
        sys.exit(-1)
    
    try:
        keyslot = int(argv[3])
        algo = int(argv[4])
    except ValueError:
        print 'Invalid keyslot/algo!'
        sys.exit(-1)
        
    if keyslot >= 0x40 or algo > 6 or algo < 0 or keyslot < 0:
        print 'Invalid keyslot/algo!'
        sys.exit(-1)
        
    keyslot |= 0x80
        
    iv = '\x00' * 0x10
    keyY = '\x00' * 0x10
    
    try:
        if (argc > 5):
            iv = binascii.unhexlify(argv[5])
        if (argc > 6):
            keyY = binascii.unhexlify(argv[6])
            keyslot |= 0x40
    except TypeError:
        print 'Invalid iv/keyY!'
        sys.exit(-1)
    
    if len(iv) != 0x10 or len(keyY) != 0x10:
        print 'Invalid iv/keyY!'
        sys.exit(-1)
     
    print 'Crypto: %s, Slot 0x%X, 0x%X bytes' % (["CBC Enc", "CBC Dec", "CTR Enc", "CTR Dec", "CCM Enc", "CCM Dec"][algo], keyslot & 0x3F, len(in_file))
    
    data = do_crypto(in_file, keyslot, algo, iv, keyY)
    
    try:
        with open(argv[2], 'wb') as f:
            f.write(data)
    except IOError:
        print 'Failed to write data to %s!' % argv[2]
        sys.exit(-1)
        
    print 'All done!'