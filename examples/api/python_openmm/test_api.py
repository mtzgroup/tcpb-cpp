import sys
# Load TCPB wrapper
try:
    import pytcpb as tc
except:
    print("ERROR: Failed to import pytcpb in test_api.py")
    sys.exit(1)

# Conversion parameter: Bohr to Angstrom
BohrToAng = 0.52917724924

# Set information about the server
host = "localhost"
port = 12345

# Other input variables
tcfile = "terachem.inp"

# Set global treatment (for how TeraChem will handle wavefunction initial guess)
globaltreatment = 0

# Information about initial QM region
qmattypes = ["O","H","H"]

# Attempts to connect to the TeraChem server
print(" Attempting to connect to TeraChem server using host %s and port %d."%(host, port))
status = tc.connect(host, port)
if (status == 0):
    print(" Successfully connected to TeraChem server.")
elif (status == 1):
    print(" ERROR: Connection to TeraChem server failed!")
    sys.exit(1)
elif (status == 2):
    print(" ERROR: Connection to TeraChem server succeed, but the \n",
          "        server is not available!")
    sys.exit(1)
else:
    print(" ERROR: Status on tc.connect function is not recognized!")
    sys.exit(1)

# Setup TeraChem
status =  tc.setup(tcfile,qmattypes)
if (status == 0):
    print(" TeraChem setup completed with success.")
elif (status == 1):
    print(" ERROR: No options read from TeraChem input file or mismatch in the input options!")
    sys.exit(1)
elif (status == 2):
    printf(" ERROR: Failed to setup TeraChem.")
    sys.exit(1)
else:
    printf(" ERROR: Status on tc_setup function is not recognized!")
    sys.exit(1)

# Set QM region coordinates, defined in Angstroms and then converted to Bohrs
qmcoords = [-4.4798000,  -2.8400000,   4.2456000,
            -4.8525000,  -3.7649000,   4.3951000,
            -3.6050000,  -2.7568000,   4.9264000]
for i in range(len(qmcoords)):
    qmcoords[i] /= BohrToAng

# MM region, defined in Angstroms and then converted to Bohrs. Charges in atomic units
mmcoords = [-2.6793000,  -2.1596000,   5.9264000,
            -1.7944000,  -2.5941000,   6.0208000,
            -2.4543000,  -1.2247000,   5.9247000 ]
for i in range(len(mmcoords)):
    mmcoords[i] /= BohrToAng
mmcharges = []

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,mmcoords,mmcharges,globaltreatment)
if (status == 0):
    print(" Computed energy and gradient with success.")
elif (status == 1):
    print(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!")
    sys.exit(1)
elif (status == 2):
    print(" ERROR: Problem to compute energy and gradient!")
    sys.exit(1)
else:
    print(" ERROR: Status on tc_compute_energy_gradient function is not recognized!")
    sys.exit(1)

# Print results
print(" Results from 1st calculation (one water molecule in the QM region and one in the MM region)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))
for i in range(int(len(mmcoords)/3)):
    print("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]))

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,mmcoords,mmcharges,globaltreatment)
if (status == 0):
    print(" Computed energy and gradient with success.")
elif (status == 1):
    print(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!")
    sys.exit(1)
elif (status == 2):
    print(" ERROR: Problem to compute energy and gradient!")
    sys.exit(1)
else:
    print(" ERROR: Status on tc_compute_energy_gradient function is not recognized!")
    sys.exit(1)

# Print results
print(" Results from 2nd calculation (one water molecule in the QM region and one in the MM region)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))
for i in range(int(len(mmcoords)/3)):
    print("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]))

# Finalizes variables on the TeraChem side
tc.finalize()

# Delete variables that have been allocated
del qmattypes, qmcoords, qmgrad, mmcoords, mmcharges, mmgrad
