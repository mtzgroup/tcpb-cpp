import sys
# Load tcpb_wrapper.py
try:
    import pytcpb as tc
except:
    print("ERROR: Failed to import pytcpb in test_api.py")
    sys.exit(1)

BohrToAng = 0.52917724924

# Set information about the server
host = "localhost"
port = 12345

# Other input variables
tcfile = "terachem.inp"

# Set global treatment
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
    print(" ERROR: No options read from TeraChem input file!")
    sys.exit(1)
elif (status == 2):
    printf(" ERROR: Failed to setup TeraChem.")
    sys.exit(1)
else:
    printf(" ERROR: Status on tc_setup function is not recognized!")
    sys.exit(1)

# Set QM region coordinates
qmcoords = [-4.4798000,  -2.8400000,   4.2456000,
            -4.8525000,  -3.7649000,   4.3951000,
            -3.6050000,  -2.7568000,   4.9264000]
for i in range(len(qmcoords)):
    qmcoords[i] /= BohrToAng

# No MM region at the moment
mmmcharges = []

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,globaltreatment=0)
if (status == 0):
    print(" Computed energy and gradient with success.")
elif (status == 1):
    print(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!")
    sys.exit(1)
elif (status == 2):
    print(" ERROR: Problem to compute energy and gradient!")
    sys.exit(1)
else:
    printf(" ERROR: Status on tc_compute_energy_gradient function is not recognized!")
    sys.exit(1)

# Print results
print(" Results from 1st calculation (only one water molecule in the QM region)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))

# We now add an MM region
mmcoords = [-2.6793000,  -2.1596000,   5.9264000,
            -1.7944000,  -2.5941000,   6.0208000,
            -2.4543000,  -1.2247000,   5.9247000,
            -6.0739000,  -0.8812700,   5.2104000,
            -5.3910000,  -1.5014000,   4.7942000,
            -5.4189000,  -0.3240900,   5.9375000,
            -4.0898000,  -5.6279000,   2.9956000,
            -4.6091000,  -5.6876000,   2.2341000,
            -4.1166000,  -6.5262000,   3.2888000,
            -2.3448000,  -2.6425000,   1.8190000,
            -2.7846000,  -3.1506000,   2.6164000,
            -1.5986000,  -3.2938000,   1.7252000,
            -4.6456000,  -4.4223000,   7.4705000,
            -3.6650000,  -4.5356000,   7.1235000,
            -4.9759000,  -3.5580000,   7.3041000 ]
for i in range(len(mmcoords)):
    mmcoords[i] /= BohrToAng
mmcharges = [  -0.834,
                0.417,
                0.417,
                -0.834,
                0.417,
                0.417,
                -0.834,
                0.417,
                0.417,
                -0.834,
                0.417,
                0.417,
                -0.834,
                0.417,
                0.417 ]

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,mmcoords,mmcharges,globaltreatment=0)
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
print(" Results from 2nd calculation (one water molecule in the QM region and five in the MM region)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))
for i in range(len(mmcharges)):
    print("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]))

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,mmcoords,mmcharges,globaltreatment=0)
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
print(" Results from 3rd calculation (just repeating the 2nd calculation)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))
for i in range(len(mmcharges)):
    print("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]))

# Change coordinates of the QM region
qmcoords = [-4.4748000,  -2.8700000,   4.5456000,
            -4.8525000,  -3.7649000,   4.3951000,
            -3.6050000,  -2.7568000,   4.9264000 ]
for i in range(len(qmcoords)):
    qmcoords[i] /= BohrToAng

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,mmcoords,mmcharges,globaltreatment=0)
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
print(" Results from 4th calculation (changed coordinates of the QM region)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))
for i in range(len(mmcharges)):
    print("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]))

# Move one water molecule from the MM region to the QM region
qmattypes = ["O","H","H","O","H","H"]
qmcoords = [-4.4798000,  -2.8400000,   4.2456000,
            -4.8525000,  -3.7649000,   4.3951000,
            -3.6050000,  -2.7568000,   4.9264000,
            -2.6793000,  -2.1596000,   5.9264000,
            -1.7944000,  -2.5941000,   6.0208000,
            -2.4543000,  -1.2247000,   5.9247000 ]
for i in range(len(qmcoords)):
    qmcoords[i] /= BohrToAng
mmcoords = [-6.0739000,  -0.8812700,   5.2104000,
            -5.3910000,  -1.5014000,   4.7942000,
            -5.4189000,  -0.3240900,   5.9375000,
            -4.0898000,  -5.6279000,   2.9956000,
            -4.6091000,  -5.6876000,   2.2341000,
            -4.1166000,  -6.5262000,   3.2888000,
            -2.3448000,  -2.6425000,   1.8190000,
            -2.7846000,  -3.1506000,   2.6164000,
            -1.5986000,  -3.2938000,   1.7252000,
            -4.6456000,  -4.4223000,   7.4705000,
            -3.6650000,  -4.5356000,   7.1235000,
            -4.9759000,  -3.5580000,   7.3041000 ]
for i in range(len(mmcoords)):
    mmcoords[i] /= BohrToAng
mmcharges =  [ -0.834,
                0.417,
                0.417,
               -0.834,
                0.417,
                0.417,
               -0.834,
                0.417,
                0.417,
               -0.834,
                0.417,
                0.417 ]

# Compute energy and gradient
print("")
totenergy, qmgrad, mmgrad, status = tc.compute_energy_gradient(qmattypes,qmcoords,mmcoords,mmcharges,globaltreatment=0)
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
print(" Results from 5th calculation (moved one molecule from the MM to the QM region)")
print("E = %16.10f Hartrees"%(totenergy))
for i in range(len(qmattypes)):
    print("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]))
for i in range(len(mmcharges)):
    print("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr"%(i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]))

# Finalizes variables on the TeraChem side
tc.finalize()

# Delete variables that have been allocated
del qmattypes, qmcoords, qmgrad, mmcoords, mmcharges, mmgrad