/// \file   scatterfit.cpp
/// \brief  Function to fit a scatter curve to a profile
/// \author Axel Wagner

#include <math.h>
//extern "C" {
#include "stir/lmmin.h"//}
#include "stir/scatterfit.h"


/// \brief User data structure for fitting routine
///
/// This is an extension of the data structure of the original lmmin code.
/// We added some data members. This is possible because a pointer to an
/// instance of this type is passed as a void pointer through the lmmin
/// code. We need the values of the additional members in out lm_evaluate.
/// We also left out the user_t member of the original code, because we
/// have equidistant data here.
///
struct lm_data_type{
    double* user_y;
    double (*user_func)( double user_t_point, double* par );
    int     gapStart, gapEnd;
};


/// \brief calculate a gaussian curve
/// \param t	x coordinate of the curve.
/// \param p	Parameters of Gaussian curve:
///   - p[0] = x coordinate of center
///   - p[1] = full width at half maximum
///   - p[2] = square root of amplitude (see note)
///
/// \note The amplitude a is given here as the square root of the normal
/// amplitude. This is because we want to have only positive amplitudes, but
/// the optimization sometimes would choose negative amplitudes to find a
/// better fit. Calculating the square assures that the real amplitude is
/// always positive.
///
inline
double gaussian( double t, double* p )
{
	t = (t-p[0]) / p[1];
	return p[2]*p[2] * exp(-4.0*log(2.0) * (t*t));
}


/// \brief dummy print callback for fitting routine
///
/// This is just an empty dummy callback, because we do not want to do any output
/// during the fitting process.
///
void lm_print_nothing(int, double*, int, double*, void*, int, int, int)
{
}

/// \brief evaluation callback for fitting routine
///
/// This function calculates the errors between the model, given
/// by data->user_func and the current parameters par, and the
/// real data in data->y.
///
/// This function is a special version of the original lmmin code.
/// It checks for the gap and always returns zero error for the
/// range of the gap. This means that any curve will fit to that
/// range, and the fitting algorithm will ignore that range.
///
/// \param[in] par	  Parameter array. At the end of the minimization,
///                   it contains the approximate solution vector.
/// \param[in] m_dat  number of data points
/// \param[out] fvec  array containing the error values
/// \param[in] data   read-only pointer to a lm_data_type
/// \param[out] info  If set to a negative value, the minimization
///                   procedure will stop.
///
void lm_evaluate( double* par, int m_dat, double* fvec, 
                  void* data, int* info )
/* 
 *	par is an input array. At the end of the minimization, it contains
 *        the approximate solution vector.
 *
 *	m_dat is a positive integer input variable set to the number
 *	  of functions.
 *
 *	fvec is an output array of length m_dat which contains the function
 *        values the square sum of which ought to be minimized.
 *
 *	data is a read-only pointer to lm_data_type, as specified by lmuse.h.
 *
 *      info is an integer output variable. If set to a negative value, the
 *        minimization procedure will stop.
 */
{
    int i;
    lm_data_type *mydata = (lm_data_type*)data;

	for (i = 0; i < mydata->gapStart; ++i)
		fvec[i] = mydata->user_y[i] - mydata->user_func( i, par);
	for (i = mydata->gapStart; i <= mydata->gapEnd; ++i)
		fvec[i] = 0.0;
	for (i = mydata->gapEnd+1; i < m_dat; ++i)
		fvec[i] = mydata->user_y[i] - mydata->user_func( i, par);
		
    *info = *info; // to prevent a 'unused variable' warning
}


/// \brief Fit a Gaussian function to tails of a profile
///
/// This function tries to estimate a gaussian curve from the tails
/// at both ends of a profile, by using a least square fitting routine.
/// The center part (given by gapStart and gapEnd) of the profile is
/// ignored.
///
/// \param[in] n	Total number of points in the profile
/// \param[in] y	The data points of the profile
/// \param[in] gapStart	Index of the first data value to be ignored
/// \param[in] gapEnd	Index of the last data value to be ignored
/// \param[out] result	Index of the last data value to be ignored
///
/// \returns	true on success, false on error. The fitInfo member
/// of the result parameter gives more detailled informations:
///  - 0 = "invalid input"
///  - 1 = "success (f)",
///  - 2 = "success (p)",
///  - 3 = "success (f,p)",
///  - 4 = "degenerate",
///  - 5 = "call limit",
///  - 6 = "failed (f)",
///  - 7 = "failed (p)",
///  - 8 = "failed (o)",
///  - 9 = "no memory",
///  - 10 = "user break"
///
bool FitGaussianTails(int n, double* y, int gapStart, int gapEnd, FitResult& result)
{
    double p[3];
    
  	// guess initial parameters (maybe this needs some tuning)
///\todo Maybe this guess of initial parameters needs some tuning.
 	p[0] = n/2.0;
 	p[1] = (gapEnd - gapStart)/2;
 	p[2] = 2 * y[gapStart];

	// initialize data structures for fitting routine:
    lm_control_type control;
    lm_data_type data;

    lm_initialize_control( &control );
    control.ftol = 1.e-6;
    control.xtol = 1.e-6;

    data.user_func = gaussian;
    data.user_y = y;
    data.gapStart = gapStart;
    data.gapEnd = gapEnd;

	// call the fitting routine:
    lm_minimize( n, 3, p, lm_evaluate, lm_print_nothing, &data, &control );

	// extract results:
	result.x0 = p[0];
	result.fwhm = p[1];
	result.a = p[2]*p[2];
	result.fitInfo = control.info;
	result.iterations = control.nfev;

 	return control.info >= 1 && control.info < 4;
}

