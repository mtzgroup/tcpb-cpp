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
write (*,*) "Results from first calculation (only one water molecule in the QM region)"
write (*,*) "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,*) "Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), "Hartree/Bohr"
end do

! Finalizes variables on the TeraChem side
call tc_finalize()

end program test_api
