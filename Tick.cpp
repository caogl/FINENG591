#include "tick.h"

#include <string>
#include <fstream>
#include <iostream>
#include "boost/date_time/gregorian/gregorian.hpp" 
#include <boost/math/tools/roots.hpp>
#include <boost/math/special_functions/erf.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <cmath>
using namespace boost::numeric::ublas;
using namespace boost::math::tools;
using namespace boost::math;
using std::ofstream;
using std::endl;
using std::string;
using std::cout;
using namespace boost::gregorian;

//EFFECTS: return option price by Crank Necolson Method
void crank(double& price, double& delta, double K, double stock, double r, double T, double vol);

//EFFECTS: return implied volatility by Crank Necolson Method
double impsolve(Tick& tick) ;

//EFFECTS: calculates black scholes option price
//MODIFIES: blsPrice
void Tick::cn_price() {
	crank(cn_Price, delta, strike,stockLast,interest, tau, sigma);
}

//EFFECTS: calculates implied volitility
//MODIFIES: impVol
void Tick::cn_impv() {
	impVol = impsolve(*this);
}

//EFFECTS: calculate difference bettween bsPrice and bid/ask.
//If ask < blsPrice, absError = ask - blsPrice; relError = absError/bsPrice;
//If bid > blsPrice, absError = bid - blsPrice; relError = absError/bsPrice;
//MODIFIES: absError, relError
void Tick::compute_error() {

	if (ask < cn_Price)
		absError = ask - cn_Price;
	if (bid > cn_Price)
		absError = bid - cn_Price; 
	else
		absError = 0;

	relError = absError/cn_Price;

}

void crank(double& price, double& delta, double K, 
				   double stock, double r, double T, double vol){
	double Smax = (K * 1.3 > stock? K * 1.3: stock * 1.3);
	double Smin = (stock > K * 0.7? K * 0.7: stock* 0.7);
	double ns=20;
	double nt=20;
	double ds,dt;
	vector<double> S(ns);
	vector<double> t(nt);
	matrix<double> A(ns-2,ns-2),V(nt,ns);
	permutation_matrix<int> pm(ns-2);
	vector<double> b(ns-2);


	ds=(Smax-Smin)/(ns-1.);
	for(int ii=1;ii<=ns;ii++){
		S(ii-1)=Smin+(ii-1)*ds;
	}
	dt=T/(nt-1.);
	for(int ii=1;ii<=nt;ii++){
		t(ii-1)=(ii-1)*dt;
	}


	for(int ii=1;ii<=ns;ii++){
		if(S(ii-1)-K>=0)
			V(0,ii-1)=S(ii-1)-K;
		else
			V(0,ii-1)=0;
	}


	for(int tt=2;tt<=nt;tt++)
	{

		for(int jj=1;jj<=ns-2;jj++){
			if(jj==1)
				A(0,jj-1)=1+0.5*vol*vol*S(jj)*S(jj)/ds/ds*dt+0.5*r*dt;
			else if(jj==2)
				A(0,jj-1)=-0.25*vol*vol*S(jj-1)*S(jj-1)/ds/ds*dt-0.25*r*S(jj-1)/ds*dt;
			else
				A(0,jj-1)=0;
		}


		for(int ii=2;ii<=ns-3;ii++){
			for(int jj=1;jj<=ns-2;jj++){
				if(jj==ii-1)
					A(ii-1,jj-1)=-0.25*vol*vol*S(jj+1)*S(jj+1)/ds/ds*dt+0.25*r*S(jj+1)/ds*dt;
				else if(jj==ii)
					A(ii-1,jj-1)=1+0.5*vol*vol*S(jj)*S(jj)/ds/ds*dt+0.5*r*dt;
				else if(jj==ii+1)
					A(ii-1,jj-1)=-0.25*vol*vol*S(jj-1)*S(jj-1)/ds/ds*dt-0.25*r*S(jj-1)/ds*dt;
				else
					A(ii-1,jj-1)=0;
			}
		}

		for(int jj=1;jj<=ns-2;jj++){
			if(jj==ns-3)
				A(ns-3,jj-1)=-0.25*vol*vol*S(jj+1)*S(jj+1)/ds/ds*dt+0.25*r*S(jj+1)/ds*dt;
			else if(jj==ns-2)
				A(ns-3,jj-1)=1+0.5*vol*vol*S(jj)*S(jj)/ds/ds*dt+0.5*r*dt;
			else
				A(ns-3,jj-1)=0;
		}

		b(0)=(0.25*vol*vol*S(1)*S(1)/ds/ds*dt+0.25*r*S(1)/ds*dt)*V(tt-2,2)+
			(1-0.5*vol*vol*S(1)*S(1)/ds/ds*dt-0.5*r*dt)*V(tt-2,1)+
			(0.25*vol*vol*S(1)*S(1)/ds/ds*dt-0.25*r*S(1)/ds*dt)*V(tt-2,0)-
			(-0.25*vol*vol*S(1)*S(1)/ds/ds*dt+0.25*r*S(1)/ds*dt)*0;

		for(int ii=2;ii<=ns-3;ii++){
			b(ii-1)=(0.25*vol*vol*S(ii)*S(ii)/ds/ds*dt+0.25*r*S(ii)/ds*dt)*V(tt-2,ii+1)+
				(1-0.5*vol*vol*S(ii)*S(ii)/ds/ds*dt-0.5*r*dt)*V(tt-2,ii)+
				(0.25*vol*vol*S(ii)*S(ii)/ds/ds*dt-0.25*r*S(ii)/ds*dt)*V(tt-2,ii-1);
		}

		b(ns-3)=(0.25*vol*vol*S(ns-2)*S(ns-2)/ds/ds*dt+0.25*r*S(ns-2)/ds*dt)*V(tt-2,ns-1)+
				(1-0.5*vol*vol*S(ns-2)*S(ns-2)/ds/ds*dt-0.5*r*dt)*V(tt-2,ns-2)+
				(0.25*vol*vol*S(ns-2)*S(ns-2)/ds/ds*dt-0.25*r*S(ns-2)/ds*dt)*V(tt-2,ns-3)-
				(-0.25*vol*vol*S(ns-2)*S(ns-2)/ds/ds*dt-0.25*r*S(ns-2)/ds*dt)*(S(ns-1)-K*exp(-r*t(tt-1)));
		
		lu_factorize(A,pm);
		lu_substitute(A,pm,b);

		V(tt-1,0)=0;
		for(int ii=2;ii<=ns-1;ii++){
			V(tt-1,ii-1)=b(ii-2);
		}
		V(tt-1,ns-1)=S(ns-1)-K*exp(-r*t(tt-1));
	}

	int index = 0;
	for(int i = 0; i < ns ; ++i)
		if (S(i) < stock && S(i+1) > stock)
		{	
			index = i;
			break;
		}
	delta = ( V(nt-1,index+1) - V(nt-1,index) )/( S(index+1) - S(index) );
	price = V(nt-1,index)+delta *(stock - S(index) );
}

double BlackScholes(Tick& tick,double tempSigma)
{
	double S = 0.5*(tick.stockAsk + tick.stockBid);
	double X = tick.strike; 
	double t = tick.tau; 
	double v = tempSigma; 
	double r = tick.interest;

	double d1 = (log(S/X) + (r +.5*v*v)*t)/(v*sqrt(t)); 
	double d2 = d1 - v * sqrt(t); 
	double N_d1 = .5*(1 + erf(d1/sqrt(2.))); 
	double N_d2 = .5*(1 + erf(d2/sqrt(2.))); 
	double C = 0; 
 
	C = S* N_d1 - X* pow(2.71828,-r*t) * N_d2; 
	return C;
} 

double impliedVol(Tick& tick,double tempSigma) 
{ 
	double a = BlackScholes(tick,tempSigma);
	return a - tick.last; 
	cout << endl;
} 

double impsolve(Tick& tick) 
{ 
	double lower = 0; 
	double upper = 10; 
	double mid; 
	double midValue; 
	double tol = 0.001; 
 
	double lowerValue = impliedVol(tick,lower); 
	double upperValue = impliedVol(tick,upper); 
 
	while(fabs( lowerValue - upperValue) > tol && (lowerValue*upperValue < 0 )) 
	{ 
		mid = (lower + upper)/2 ; 
 		midValue = impliedVol(tick,mid); 
 		if(midValue*upperValue > 0) 
		{ 
			upper = mid; 
			upperValue = midValue; 
		} 
		else{ 
			lower = mid; 
			lowerValue = midValue; 
		} 
	} 
	return mid; 
} 