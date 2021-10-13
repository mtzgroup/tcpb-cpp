!****************************************************
! Program for testing TCPB working as an API library
! Implemented by: Vinicius Wilian D. Cruzeiro
!****************************************************

program test_api

implicit none

character(len=80)  :: host
character(len=256) :: tcfile
integer :: port, status
integer :: numqmatoms, nummmatoms
integer :: i
double precision :: totenergy
character(len=5), allocatable :: qmattypes(:)
double precision, allocatable :: qmcoords(:), mmcoords(:), mmcharges(:), qmgrad(:), mmgrad(:)
double precision, parameter :: BohrToAng = 0.52917724924

! Set information about the server
host = "localhost"
port = 12345

! Other input variables
tcfile = "terachem.inp"
tcfile = trim(tcfile)//CHAR(0)

! Information about initial QM and MM region
numqmatoms = 3
allocate(qmattypes(numqmatoms))
qmattypes = (/ "O","H","H" /)
do i =1, numqmatoms
  qmattypes(i)=trim(qmattypes(i))//CHAR(0)
end do

! Attempts to connect to the TeraChem server
write (*,*) "Attempting to connect to TeraChem server using host ", &
  trim(host)//CHAR(0), " and port ", port, "."
status = -1
call tc_connect(trim(host)//CHAR(0), port, status)
if (status == 0) then
  write (*,*) "Successfully connected to TeraChem server."
else if (status == 1) then
  write (*,*) "ERROR: Connection to TeraChem server failed!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Connection to TeraChem server succeed, but the &
    server is not available!"
  STOP
else
  write (*,*) "ERROR: Status on tc_connect function is not recognized!"
  STOP
end if

! Setup TeraChem
status = -1
call tc_setup(tcfile,qmattypes,numqmatoms,status)
if (status == 0) then
  write (*,*) "TeraChem setup completed with success."
else if (status == 1) then
  write (*,*) "ERROR: No options read from TeraChem input file!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Failed to setup TeraChem."
  STOP
else
  write (*,*) "ERROR: Status on tc_setup function is not recognized!"
  STOP
end if

! Set QM region coordinates
allocate(qmcoords(3*numqmatoms),qmgrad(3*numqmatoms))
qmcoords = (/ -4.4798000,         -2.8400000,          4.2456000,&
           -4.8525000,         -3.7649000,          4.3951000,&
           -3.6050000,         -2.7568000,          4.9264000 /)
do i =1, 3*numqmatoms
  qmcoords(i)=qmcoords(i)/BohrToAng
end do

! No MM region at the moment
nummmatoms = 0

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,status)
if (status == 0) then
  write (*,*) "Computed energy and gradient with success."
else if (status == 1) then
  write (*,*) "ERROR: Mismatch in the variables passed to the function to compute energy and gradient!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Problem to compute energy and gradient!"
  STOP
else
  write (*,*) "ERROR: Status on tc_compute_energy_gradient function is not recognized!"
  STOP
end if

! Print results
write (*,*) "Results from 1st calculation (only one water molecule in the QM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! We now add an MM region
nummmatoms = 15
allocate(mmcoords(3*nummmatoms),mmcharges(nummmatoms),mmgrad(3*nummmatoms))
mmcoords = (/ -2.6793000,         -2.1596000,          5.9264000,&
  -1.7944000,         -2.5941000,          6.0208000,&
  -2.4543000,         -1.2247000,          5.9247000,&
  -6.0739000,         -0.8812700,          5.2104000,&
  -5.3910000,         -1.5014000,          4.7942000,&
  -5.4189000,         -0.3240900,          5.9375000,&
  -4.0898000,         -5.6279000,          2.9956000,&
  -4.6091000,         -5.6876000,          2.2341000,&
  -4.1166000,         -6.5262000,          3.2888000,&
  -2.3448000,         -2.6425000,          1.8190000,&
  -2.7846000,         -3.1506000,          2.6164000,&
  -1.5986000,         -3.2938000,          1.7252000,&
  -4.6456000,         -4.4223000,          7.4705000,&
  -3.6650000,         -4.5356000,          7.1235000,&
  -4.9759000,         -3.5580000,          7.3041000 /)
do i =1, 3*nummmatoms
  mmcoords(i)=mmcoords(i)/BohrToAng
end do
mmcharges = (/ -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417 /)

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,status)
if (status == 0) then
  write (*,*) "Computed energy and gradient with success."
else if (status == 1) then
  write (*,*) "ERROR: Mismatch in the variables passed to the function to compute energy and gradient!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Problem to compute energy and gradient!"
  STOP
else
  write (*,*) "ERROR: Status on tc_compute_energy_gradient function is not recognized!"
  STOP
end if

! Print results
write (*,*) "Results from 2nd calculation (one water molecule in the QM region and five in the MM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do
do i =1, nummmatoms
  write (*,'(a,i3,a,3f16.10,a)') "MM Grad(",i,",:) = ",mmgrad(3*(i-1)+1), mmgrad(3*(i-1)+2), mmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,status)
if (status == 0) then
  write (*,*) "Computed energy and gradient with success."
else if (status == 1) then
  write (*,*) "ERROR: Mismatch in the variables passed to the function to compute energy and gradient!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Problem to compute energy and gradient!"
  STOP
else
  write (*,*) "ERROR: Status on tc_compute_energy_gradient function is not recognized!"
  STOP
end if

! Print results
write (*,*) "Results from 3rd calculation (just repeating the 2nd calculation)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do
do i =1, nummmatoms
  write (*,'(a,i3,a,3f16.10,a)') "MM Grad(",i,",:) = ",mmgrad(3*(i-1)+1), mmgrad(3*(i-1)+2), mmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! Change coordinates of the QM region
qmcoords = (/ -4.4748000,         -2.8700000,          4.5456000,&
           -4.8525000,         -3.7649000,          4.3951000,&
           -3.6050000,         -2.7568000,          4.9264000 /)
do i =1, 3*numqmatoms
  qmcoords(i)=qmcoords(i)/BohrToAng
end do

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,status)
if (status == 0) then
  write (*,*) "Computed energy and gradient with success."
else if (status == 1) then
  write (*,*) "ERROR: Mismatch in the variables passed to the function to compute energy and gradient!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Problem to compute energy and gradient!"
  STOP
else
  write (*,*) "ERROR: Status on tc_compute_energy_gradient function is not recognized!"
  STOP
end if

! Print results
write (*,*) "Results from 4th calculation (changed coordinates of the QM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do
do i =1, nummmatoms
  write (*,'(a,i3,a,3f16.10,a)') "MM Grad(",i,",:) = ",mmgrad(3*(i-1)+1), mmgrad(3*(i-1)+2), mmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! Move one water molecule from the MM region to the QM region
deallocate(qmattypes,qmcoords,qmgrad,mmcoords,mmcharges,mmgrad)
numqmatoms = 6
allocate(qmattypes(numqmatoms),qmcoords(3*numqmatoms),qmgrad(3*numqmatoms))
qmattypes = (/ "O","H","H","O","H","H" /)
do i =1, numqmatoms
  qmattypes(i)=trim(qmattypes(i))//CHAR(0)
end do
qmcoords = (/ -4.4798000,         -2.8400000,          4.2456000,&
           -4.8525000,         -3.7649000,          4.3951000,&
           -3.6050000,         -2.7568000,          4.9264000,&
           -2.6793000,         -2.1596000,          5.9264000,&
           -1.7944000,         -2.5941000,          6.0208000,&
           -2.4543000,         -1.2247000,          5.9247000 /)
do i =1, 3*numqmatoms
  qmcoords(i)=qmcoords(i)/BohrToAng
end do
nummmatoms = 12
allocate(mmcoords(3*nummmatoms),mmcharges(nummmatoms),mmgrad(3*nummmatoms))
mmcoords = (/ -6.0739000,         -0.8812700,          5.2104000,&
  -5.3910000,         -1.5014000,          4.7942000,&
  -5.4189000,         -0.3240900,          5.9375000,&
  -4.0898000,         -5.6279000,          2.9956000,&
  -4.6091000,         -5.6876000,          2.2341000,&
  -4.1166000,         -6.5262000,          3.2888000,&
  -2.3448000,         -2.6425000,          1.8190000,&
  -2.7846000,         -3.1506000,          2.6164000,&
  -1.5986000,         -3.2938000,          1.7252000,&
  -4.6456000,         -4.4223000,          7.4705000,&
  -3.6650000,         -4.5356000,          7.1235000,&
  -4.9759000,         -3.5580000,          7.3041000 /)
do i =1, 3*nummmatoms
  mmcoords(i)=mmcoords(i)/BohrToAng
end do
mmcharges = (/ -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417,&
  -0.834,&
  0.417,&
  0.417 /)

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,status)
if (status == 0) then
  write (*,*) "Computed energy and gradient with success."
else if (status == 1) then
  write (*,*) "ERROR: Mismatch in the variables passed to the function to compute energy and gradient!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Problem to compute energy and gradient!"
  STOP
else
  write (*,*) "ERROR: Status on tc_compute_energy_gradient function is not recognized!"
  STOP
end if

! Print results
write (*,*) "Results from 5th calculation (moved one molecule from the MM to the QM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do
do i =1, nummmatoms
  write (*,'(a,i3,a,3f16.10,a)') "MM Grad(",i,",:) = ",mmgrad(3*(i-1)+1), mmgrad(3*(i-1)+2), mmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! Finalizes variables on the TeraChem side
call tc_finalize()

end program test_api
