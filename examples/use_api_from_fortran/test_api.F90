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
character(len=5), allocatable :: qmattypes(:)

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
nummmatoms = 0

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

! Finalizes variables on the TeraChem side
call tc_finalize()

end program test_api
