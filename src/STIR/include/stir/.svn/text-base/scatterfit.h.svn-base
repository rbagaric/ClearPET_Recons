/// \file   scatterfit.h
/// \brief  Function to fit a scatter curve to a profile
/// \author Axel Wagner

#ifndef scatterfit_h_included
#define scatterfit_h_included


/// \brief Result of Fitting function
struct FitResult
{
	double	x0;
	double	fwhm;
	double	a;
	int		fitInfo;
	int		iterations;
};

bool FitGaussianTails(int n, double* y, int gapStart, int gapEnd, FitResult& result);

#endif // scatterfit_h_included
