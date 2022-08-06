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
double precision, allocatable :: qmcoords(:), mmcoords(:), mmcharges(:), qmcharges(:), qmgrad(:), mmgrad(:)
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

! No MM region at the moment
nummmatoms = 0

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
write (*,*) "Results from 1st calculation (only one water molecule in the QM region)"
write (*,'(a,f16.10,a)') "E = ", totenergy, " Hartrees"
do i =1, numqmatoms
  write (*,'(a,i3,a,3f16.10,a)') "QM Grad(",i,",:) = ",qmgrad(3*(i-1)+1), qmgrad(3*(i-1)+2), qmgrad(3*(i-1)+3), " Hartree/Bohr"
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

! We now add an MM region, defined in Angstroms and then converted to Bohrs. Charges in atomic units
nummmatoms = 15
allocate(mmcoords(3*nummmatoms),mmcharges(nummmatoms),mmgrad(3*nummmatoms))
mmcoords = (/ -2.6793000d0,  -2.1596000d0,   5.9264000d0,&
              -1.7944000d0,  -2.5941000d0,   6.0208000d0,&
              -2.4543000d0,  -1.2247000d0,   5.9247000d0,&
              -6.0739000d0,  -0.8812700d0,   5.2104000d0,&
              -5.3910000d0,  -1.5014000d0,   4.7942000d0,&
              -5.4189000d0,  -0.3240900d0,   5.9375000d0,&
              -4.0898000d0,  -5.6279000d0,   2.9956000d0,&
              -4.6091000d0,  -5.6876000d0,   2.2341000d0,&
              -4.1166000d0,  -6.5262000d0,   3.2888000d0,&
              -2.3448000d0,  -2.6425000d0,   1.8190000d0,&
              -2.7846000d0,  -3.1506000d0,   2.6164000d0,&
              -1.5986000d0,  -3.2938000d0,   1.7252000d0,&
              -4.6456000d0,  -4.4223000d0,   7.4705000d0,&
              -3.6650000d0,  -4.5356000d0,   7.1235000d0,&
              -4.9759000d0,  -3.5580000d0,   7.3041000d0 /)
do i =1, 3*nummmatoms
  mmcoords(i)=mmcoords(i)/BohrToAng
end do
mmcharges = (/ -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0 /)

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
write (*,*) "Results from 2nd calculation (one water molecule in the QM region and five in the MM region)"
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
write (*,*) "Results from 3rd calculation (just repeating the 2nd calculation)"
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
write (*,*) "Charges from 3rd calculation"
do i =1, numqmatoms
  write (*,'(a,i3,a,f16.10,a)') "QM Charge(",i,") = ",qmcharges(i)
end do

! Change coordinates of the QM region, defined in Angstroms and then converted to Bohrs
qmcoords = (/ -4.4748000d0,  -2.8700000d0,   4.5456000d0,&
              -4.8525000d0,  -3.7649000d0,   4.3951000d0,&
              -3.6050000d0,  -2.7568000d0,   4.9264000d0 /)
do i =1, 3*numqmatoms
  qmcoords(i)=qmcoords(i)/BohrToAng
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
write (*,*) "Results from 4th calculation (changed coordinates of the QM region)"
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
write (*,*) "Charges from 4th calculation"
do i =1, numqmatoms
  write (*,'(a,i3,a,f16.10,a)') "QM Charge(",i,") = ",qmcharges(i)
end do

! Move one water molecule from the MM region to the QM region, defined in Angstroms and then converted to Bohrs. Charges in atomic units.
deallocate(qmattypes,qmcoords,qmgrad,qmcharges,mmcoords,mmcharges,mmgrad)
numqmatoms = 6
allocate(qmattypes(numqmatoms),qmcoords(3*numqmatoms),qmgrad(3*numqmatoms),qmcharges(numqmatoms))
qmattypes = (/ "O","H","H","O","H","H" /)
do i =1, numqmatoms
  qmattypes(i)=trim(qmattypes(i))//CHAR(0)
end do
qmcoords = (/ -4.4798000d0,  -2.8400000d0,   4.2456000d0,&
              -4.8525000d0,  -3.7649000d0,   4.3951000d0,&
              -3.6050000d0,  -2.7568000d0,   4.9264000d0,&
              -2.6793000d0,  -2.1596000d0,   5.9264000d0,&
              -1.7944000d0,  -2.5941000d0,   6.0208000d0,&
              -2.4543000d0,  -1.2247000d0,   5.9247000d0 /)
do i =1, 3*numqmatoms
  qmcoords(i)=qmcoords(i)/BohrToAng
end do
nummmatoms = 12
allocate(mmcoords(3*nummmatoms),mmcharges(nummmatoms),mmgrad(3*nummmatoms))
mmcoords = (/ -6.0739000d0,  -0.8812700d0,   5.2104000d0,&
              -5.3910000d0,  -1.5014000d0,   4.7942000d0,&
              -5.4189000d0,  -0.3240900d0,   5.9375000d0,&
              -4.0898000d0,  -5.6279000d0,   2.9956000d0,&
              -4.6091000d0,  -5.6876000d0,   2.2341000d0,&
              -4.1166000d0,  -6.5262000d0,   3.2888000d0,&
              -2.3448000d0,  -2.6425000d0,   1.8190000d0,&
              -2.7846000d0,  -3.1506000d0,   2.6164000d0,&
              -1.5986000d0,  -3.2938000d0,   1.7252000d0,&
              -4.6456000d0,  -4.4223000d0,   7.4705000d0,&
              -3.6650000d0,  -4.5356000d0,   7.1235000d0,&
              -4.9759000d0,  -3.5580000d0,   7.3041000d0 /)
do i =1, 3*nummmatoms
  mmcoords(i)=mmcoords(i)/BohrToAng
end do
mmcharges = (/ -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0,&
               -0.834d0,&
                0.417d0,&
                0.417d0 /)

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
write (*,*) "Results from 5th calculation (moved one molecule from the MM to the QM region)"
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
write (*,*) "Charges from 5th calculation"
do i =1, numqmatoms
  write (*,'(a,i3,a,f16.10,a)') "QM Charge(",i,") = ",qmcharges(i)
end do

! Finalizes variables on the TeraChem side
call tc_finalize()

! Deallocate variables that have been allocated
deallocate(qmattypes,qmcoords,qmgrad,mmcoords,mmcharges,mmgrad)

end program test_api
