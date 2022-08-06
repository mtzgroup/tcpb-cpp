!****************************************************
! Program for testing TCPB working as an API library
! Implemented by: Vinicius Wilian D. Cruzeiro
!****************************************************

program test_api

implicit none

character(len=80)  :: host
character(len=256) :: tcfile
integer :: port, status, globaltreatment
integer :: numqmatoms, nummmatoms
integer :: i
double precision :: totenergy
character(len=5), allocatable :: qmattypes(:)
double precision, allocatable :: qmcoords(:), mmcoords(:), mmcharges(:), qmgrad(:), mmgrad(:), qmcharges(:)
! Conversion parameter: Bohr to Angstrom
double precision, parameter :: BohrToAng = 0.52917724924d0

! Set information about the server
host = "localhost"
port = 12345

! Other input variables
tcfile = "terachem.inp"
tcfile = trim(tcfile)//CHAR(0)

! Set global treatment
globaltreatment = 0

! Information about initial QM region
numqmatoms = 3
allocate(qmattypes(numqmatoms))
qmattypes = (/ "O","H","H" /)
do i =1, numqmatoms
  qmattypes(i)=trim(qmattypes(i))//CHAR(0)
end do

! Attempts to connect to the TeraChem server
write (*,*) "Attempting to connect to TeraChem server using host ", &
  trim(host), " and port ", port, "."
status = -1
call tc_connect(trim(host)//CHAR(0), port, status)
if (status == 0) then
  write (*,*) "Successfully connected to TeraChem server."
else if (status == 1) then
  write (*,*) "ERROR: Connection to TeraChem server failed!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Connection to TeraChem server succeed, but the ", &
       "       server is not available!"
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
  write (*,*) "ERROR: No options read from TeraChem input file or mismatch in the input options!"
  STOP
else if (status == 2) then
  write (*,*) "ERROR: Failed to setup TeraChem."
  STOP
else
  write (*,*) "ERROR: Status on tc_setup function is not recognized!"
  STOP
end if

! Set QM region coordinates, defined in Angstroms and then converted to Bohrs
allocate(qmcoords(3*numqmatoms),qmgrad(3*numqmatoms),qmcharges(numqmatoms))
qmcoords = (/ -4.4798000d0,  -2.8400000d0,   4.2456000d0,&
              -4.8525000d0,  -3.7649000d0,   4.3951000d0,&
              -3.6050000d0,  -2.7568000d0,   4.9264000d0 /)
do i =1, 3*numqmatoms
  qmcoords(i)=qmcoords(i)/BohrToAng
end do

! MM region, defined in Angstroms and then converted to Bohrs. Charges in atomic units
nummmatoms = 3
allocate(mmcoords(3*nummmatoms),mmgrad(3*nummmatoms))
mmcoords = (/ -2.6793000d0,  -2.1596000d0,   5.9264000d0,&
              -1.7944000d0,  -2.5941000d0,   6.0208000d0,&
              -2.4543000d0,  -1.2247000d0,   5.9247000d0 /)
do i =1, 3*nummmatoms
  mmcoords(i)=mmcoords(i)/BohrToAng
end do

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,globaltreatment,status)
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
write (*,*) "Results from 1st calculation (one water molecule in the QM region and one in the MM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do
do i =1, nummmatoms
  write (*,'(a,i3,a,3f16.10,a)') "MM Grad(",i,",:) = ",mmgrad(3*(i-1)+1), mmgrad(3*(i-1)+2), mmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! Get QM charges
write (*,*) ""
status = -1
call tc_get_qm_charges(qmcharges,status)
if (status == 0) then
  write (*,*) "Got QM charges with success."
else if (status == 1) then
  write (*,*) "ERROR: Problem to get QM charges!"
  STOP
else
  write (*,*) "ERROR: Status on tc_get_qm_charges function is not recognized!"
  STOP
end if

! Print charges
write (*,*) "Charges from 1st calculation"
do i =1, numqmatoms
  write (*,'(a,i3,a,f16.10,a)') "QM Charge(",i,") = ",qmcharges(i)
end do

! Compute energy and gradient
write (*,*) ""
status = -1
call tc_compute_energy_gradient(qmattypes,qmcoords,numqmatoms,totenergy,qmgrad,mmcoords,mmcharges,nummmatoms,mmgrad,globaltreatment,status)
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
write (*,*) "Results from 2nd calculation (one water molecule in the QM region and one in the MM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
end do
do i =1, nummmatoms
  write (*,'(a,i3,a,3f16.10,a)') "MM Grad(",i,",:) = ",mmgrad(3*(i-1)+1), mmgrad(3*(i-1)+2), mmgrad(3*(i-1)+3), " Hartree/Bohr"
end do

! Get QM charges
write (*,*) ""
status = -1
call tc_get_qm_charges(qmcharges,status)
if (status == 0) then
  write (*,*) "Got QM charges with success."
else if (status == 1) then
  write (*,*) "ERROR: Problem to get QM charges!"
  STOP
else
  write (*,*) "ERROR: Status on tc_get_qm_charges function is not recognized!"
  STOP
end if

! Print charges
write (*,*) "Charges from 2nd calculation"
do i =1, numqmatoms
  write (*,'(a,i3,a,f16.10,a)') "QM Charge(",i,") = ",qmcharges(i)
end do

! Finalizes variables on the TeraChem side
call tc_finalize()

! Deallocate variables that have been allocated
deallocate(qmattypes,qmcoords,qmgrad,mmcoords,mmgrad)

end program test_api
