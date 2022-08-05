import sys
# Load ctypes
try:
    import ctypes
except:
    print("ERROR: Failed to import ctypes in pytcpb")
    sys.exit(1)

# Load libtcpb
try:
    libtcpb = ctypes.CDLL('libtcpb.so')
except:
    print("ERROR: Failed to load libtcpb.so in pytcpb.")
    print("       Make sure the path to this library is in your LD_LIBRARY_PATH")
    sys.exit(1)

# Novel variables types
CharArr5 = ctypes.c_char * 5
CharArr80 = ctypes.c_char * 80
CharArr256 = ctypes.c_char * 256

# Function tc_connect
libtcpb.tc_connect_.argtypes = (CharArr80, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int))
libtcpb.tc_connect_.restype = None
def connect(host,port):
    """
    Python version of function tc_connect from libtcpb.so
    """
    global libtcpb
    bhost = CharArr80()
    bhost.value = str.encode(host)
    status = ctypes.c_int()
    libtcpb.tc_connect_(bhost,ctypes.c_int(port),status)
    return status.value

# Function tc_setup
libtcpb.tc_setup_.argtypes = (CharArr256, ctypes.POINTER(CharArr5), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int))
libtcpb.tc_setup_.restype = None
def setup(tcfile,qmattypes):
    """
    Python version of function tc_setup from libtcpb.so
    """
    global libtcpb
    numqmatoms = len(qmattypes)
    btcfile = CharArr256()
    btcfile.value = str.encode(tcfile)
    NewCharType = CharArr5 * numqmatoms
    bqmattypes = NewCharType()
    for i in range(numqmatoms):
        bqmattypes[i].value = str.encode(qmattypes[i])
    status = ctypes.c_int()
    libtcpb.tc_setup_(btcfile,bqmattypes,ctypes.c_int(numqmatoms),status)
    return status.value

# Function tc_compute_energy_gradient
libtcpb.tc_compute_energy_gradient_.argtypes = (ctypes.POINTER(CharArr5), ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int),
                                                ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
                                                ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_double),
                                                ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int))
libtcpb.tc_compute_energy_gradient_.restype = None
def compute_energy_gradient(qmattypes,qmcoords,mmcoords=[],mmcharges=[],globaltreatment=0):
    """
    Python version of function tc_compute_energy_gradient from libtcpb.so
    """
    global libtcpb
    numqmatoms = len(qmattypes)
    nummmatoms = int(len(mmcoords)/3)
    if (len(qmcoords)!=3*numqmatoms or (mmcharges and len(mmcharges)!=nummmatoms) or globaltreatment<0 or globaltreatment>2):
        return 1
    NewCharType = CharArr5 * numqmatoms
    bqmattypes = NewCharType()
    for i in range(numqmatoms):
        bqmattypes[i].value = str.encode(qmattypes[i])
    QMDoubleType = ctypes.c_double * (3*numqmatoms)
    bqmcoords = QMDoubleType()
    for i in range(3*numqmatoms):
        bqmcoords[i] = ctypes.c_double(qmcoords[i])
    bqmgrad = QMDoubleType()
    MMDoubleType = ctypes.c_double * (3*nummmatoms)
    bmmcoords = MMDoubleType()
    for i in range(3*nummmatoms):
        bmmcoords[i] = ctypes.c_double(mmcoords[i])
    bmmgrad = MMDoubleType()
    MMChargeDoubleType = ctypes.c_double * nummmatoms
    bmmcharges = MMChargeDoubleType()
    if (mmcharges):
        for i in range(nummmatoms):
            bmmcharges[i] = ctypes.c_double(mmcharges[i])
    btotenergy = ctypes.c_double()
    status = ctypes.c_int()
    libtcpb.tc_compute_energy_gradient_(bqmattypes,bqmcoords,ctypes.c_int(numqmatoms),btotenergy,bqmgrad,bmmcoords,bmmcharges,ctypes.c_int(nummmatoms),
                                        bmmgrad,ctypes.c_int(globaltreatment),status)
    return btotenergy.value, bqmgrad, bmmgrad, status.value

# Function tc_get_qm_charges
libtcpb.tc_get_qm_charges_.argtypes = (ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int))
libtcpb.tc_get_qm_charges_.restype = None
def get_qm_charges(numqmatoms):
    """
    Python version of function tc_get_qm_charges from libtcpb.so
    """
    global libtcpb
    DoubleType = ctypes.c_double * (numqmatoms)
    bqmcharges = DoubleType()
    status = ctypes.c_int()
    libtcpb.tc_get_qm_charges_(bqmcharges,status)
    return bqmcharges, status.value

# Function tc_finalize
libtcpb.tc_finalize_.argtypes = ()
libtcpb.tc_finalize_.restype = None
def finalize():
    """
    Python version of function tc_finalize from libtcpb.so
    """
    global libtcpb
    libtcpb.tc_finalize_()
