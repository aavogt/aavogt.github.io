! Jfun.f90
! aavogt@uwaterloo.ca April 2013.
! refer to Jversion.html or Jversion.Rmd
!
! A pure R version was also done, but this is 50 times or so faster.

! result after calling intU contains an integral of
! (u-u0) from yMin to yMax, all other parameters constant:
!
!             yMax
!            /
!            [
! result =   I     (u(y; x, r, u0) - u0) dy
!            ]
!            /
!             yMin
!
! this is a multiple of the outlet concentration. This
! can then be multiplied by some parameters to give
! the total mass of solute that was lost.
!
! this approach works much better than integrating over
! the bed volume, particularly after the column is fully
! loaded.
subroutine intU(x, yMin, yMax, r, u0, result)
    implicit none
    real (kind=8), intent(in) :: x,yMin, yMax, r, u0
    real (kind=8), intent(out) :: result

    integer, parameter :: limit = 100, lenw = limit*4

    real (kind=8) :: epsabs, epsrel, abserr, work(lenw)
    integer neval, ier, last, iwork(limit)

    epsabs = 1e-7
    epsrel = 1e-6

    call dqags ( subfun, yMin, yMax, epsabs, epsrel, result, abserr, &
        & neval, ier, limit, lenw, last, iwork, work)

    contains
    function subfun(y)

    real *8 :: y, subfun
    real *8, external :: FU

    subfun = FU( x, y, r, u0 )

    end function
end subroutine intU

! FF FU FV are based on equation 55.
! The M and N disapear because they are substituted into equation 16
function FF (x,y,r,u0)
    real *8 :: FF,x,y,r,u0
    real *8, external :: J

    ! b is beta defined right above eq. 31
    b = (1d0-r)*u0 + r
    FF = J((r*x)/b,b*y)*exp((b-1d0)*y+(1d0/b-1d0)*r*x) & 
        & +(1d0-J(x,r*y))*exp((1d0-r)*(x-y))
end function

function FU (x,y,r,u0)
    real *8 ::  FU, x,y,r,u0, FF
    real *8, external :: J
    b = (1d0-r)*u0 + r
    FU = u0*J((r*x)/b,b*y)* & 
            & exp((b-1d0)*y-(1d0/b-1d0)*r*x) / FF(x,y,r,u0)
    if (FU - 1d0 .GE. 1d-3) print *, "FU=", FU
end function

function FV (x,y,r,u0)
    real *8 :: FV, x,y,r,u0, FF
    real *8, external :: J
    b = (1d0-r)*u0 + r
    FV = (u0*exp((b-1d0)*y+(1d0/b-1d0)*r*x) & 
            & *(1d0-J(b*y,(r*x)/b)))/b / FF(x,y,r,u0)
    if (FV - 1d0 .GE. 1d-3) print *, "FV=", FV
end function

! same as FV but a subroutine for R
subroutine FVsub (x,y,r,u0, out)
    implicit real *8 (a-z)
    out = FV(x,y,r,u0)
end subroutine

! same as FU but a subroutine for R
subroutine FUsub (x,y,r,u0, out)
    implicit real *8 (a-z)
    out = FU(x,y,r,u0)
end subroutine

! same as Jsub, but as a function
function J (x,y)
    real (kind=8) :: J, x, y
    call Jsub (x,y,J)
end function

! equation 39, slight manipulation should avoid 0*inf
subroutine Jsub (x,y,j)
    real (kind=8), intent(in) :: x,y
    real (kind=8), intent(out) :: j

    integer, parameter :: limit = 100, lenw = limit*4

    real (kind=8) :: epsabs, epsrel, abserr, work(lenw)
    integer neval, ier, last, iwork(limit)

    epsabs = 1e-7
    epsrel = 1e-6

    call dqags ( subfun, 0d0, x, epsabs, epsrel, j, abserr, neval, ier, &
        & limit, lenw, last, iwork, work)

    j = 1d0 - j

    if (ier .ne. 0) print *, ier, abserr, abserr/j

    contains
    function subfun(tau)
        implicit none
        real (kind=8) :: tau, subfun
        real (kind=8), external :: dbsi0e
        subfun = dbsi0e(2d0 * sqrt(tau*y)) * exp(2d0*sqrt(tau*y) - (y+tau))
    end function
end subroutine 

