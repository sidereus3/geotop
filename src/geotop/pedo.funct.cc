
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 2.1 release candidate  (release date: 31 december 2016)
 
 Copyright (c), 2016 - GEOtop Foundation
 
 This file is part of GEOtop 2.1 
 
 GEOtop 2.1  is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 2.1  is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */
#include "pedo.funct.h"
#include "geotop_common.h"

/*
in all the following subroutine
w=theta
i=theta_ice
s=saturated water content
r=residual water content
a=alpha van genuchten
n=n van genuchten
m=m van genuchten
pmin=psi min
Ss=specific storativity
*/

/*--------------------------------------------*/
double psi_teta(double w, double i, double s, double r, double a, double n, double m, double pmin, double Ss )

{

 double psi,TETA,TETAsat,TETAmin;
 short sat;
 
 TETAsat=1.0-i/(s-r);
 TETAmin=1.0/pow((1.0+pow(a*(-pmin),n)),m);

 if(w>s-i){
	TETA=TETAsat; 
	sat=1;
 }else{
 	TETA=(w-r)/(s-r);
	sat=0;
 }
 
 if(TETA<TETAmin) TETA=TETAmin;
 
 if(TETA>1.0-1.E-6){
	psi=0.0;
 }else{
	psi=(pow((pow(TETA,-1.0/m)-1.0),1.0/n))*(-1.0/a);
 }
 if(sat==1) psi += (w-(s-i))/Ss; 

 return psi;
}




/*--------------------------------------------*/
double teta_psi(double psi, double i, double s, double r, double a, double n, double m, double pmin, double Ss)
{
 double teta,TETA,psisat;
 short sat=0;

 psisat=(pow((pow(1.0-i/(s-r),-1.0/m)-1.0),1.0/n))*(-1.0/a);
 if(psi>psisat) sat=1;
  
 if(psi<pmin) psi=pmin;
 
 if(sat==0){
	if(psi>-1.E-6){
		TETA=1.0;
	}else{
		TETA=1.0/pow((1.0+pow(a*(-psi),n)),m);
	}
	teta=r+TETA*(s-r);
 }else{
	teta= s-i + Ss * (psi-psisat);
 }

 return teta;
}

/*--------------------------------------------*/
double dteta_dpsi(double psi, double i, double s, double r, double a, double n, double m, double pmin, double Ss )
//it is the derivative of teta with respect to psi [mm^-1]
{
 double dteta,psisat;

 psisat=(pow((pow(1.0-i/(s-r),-1.0/m)-1.0),1.0/n))*(-1.0/a);
  
 if(psi>=psisat){
	dteta=Ss;
 }else{
	dteta=(s-r)*(a*m*n)*(pow(-a*psi,n-1.0))*pow(1.0+pow(-a*psi,n),-m-1.0);
 } 
	
 return dteta;
}

/*--------------------------------------------*/
double k_hydr_soil(double psi, double ksat, double imp, double i, double s, double r, double a, double n, double m, double v, double T, double ratio)

{

	double k,TETA,psisat;
	
	psisat = (pow((pow(1.0-i/(s-r),-1.0/m)-1.0),1.0/n))*(-1.0/a);  
	TETA = 1.0/pow((1.0+pow(a*(-Fmin(psisat,psi)),n)),m);
	
	k = ksat * pow(TETA,v)*(pow((1-pow((1-pow(TETA,(1.0/m))),m)),2.0));
	
	if (k/ksat < ratio) k = ratio * ksat;
	
	//printf("ksat:%e TETA:%e k:%e ",ksat,TETA,k);
		
	if(T>=0){
		k *= (0.000158685828*T*T+0.025263459766*T+0.731495819);
	}else{
		k *= 0.731495819;
	}
	
	//printf("k:%e ",k);
	
	k *= (pow(10.0, -imp*i/(s-r)));	
	
	//printf("imp:%e %e k:%e ",imp,i/(s-r),k);

	return k;
 
}

/*--------------------------------------------*/

double psi_saturation(double i, double s, double r, double a, double n, double m){
	
	double psisat;
		
	if(i<0) i=0.;
	if(1.0-i/(s-r)>1.E-6){
		psisat=(pow((pow(1.0-i/(s-r),-1.0/m)-1.0),1.0/n))*(-1.0/a);
	}else{
		psisat=0.0;
	}
	
	return(psisat);

}
/*------------------------------------------------------------------------------------------------------*/

double Harmonic_Mean(double D1, double D2, double K1, double K2)
	/*  calculates the harmonic mean of a property K weighted on the layer depth D
	 * It is used to calculate the property K at the interface between two consecutive layers K1,D1 and K2,D2
	 *  mean=(D1+D2)/(D1/K1+D2/K2)
	 * Author: Matteo Dall'Amico, Sept 2011 */
{
	return((D1+D2)/(D1/K1+D2/K2));
}


double Arithmetic_Mean(double D1, double D2, double K1, double K2)
{
	return((D1*K2+D2*K1)/(D1+D2));
}

/*------------------------------------------------------------------------------------------------------*/

double Mean(short a, double D1, double D2, double K1, double K2)

{

	if(a==0){
		return(Harmonic_Mean(D1,D2,K1,K2));
	}else if(a==1){
		return(Arithmetic_Mean(D1,D2,K1,K2));
	}else{
		return(0.0);
	}
}

	
/*------------------------------------------------------------------------------------------------------*/

double Psif(double T){
	
	double psi;
	
	if(T<0){
		psi=T*(1000.0*GTConst::Lf)/(GTConst::GRAVITY*(GTConst::Tfreezing+GTConst::tk));
	}else{
		psi=0;
	}
		
	return(psi);
}

/******************************************************************************************************************************************/
//double theta_from_psi(double psi, double ice, long l, double **pa, double pmin){
double theta_from_psi(double psi, double ice, long l, const GeoTensor<double>& pa, const long& sy, double pmin)
{
	double s=pa(sy,jsat,l);
	double res=pa(sy,jres,l);
	double a=pa(sy,ja,l);
	double n=pa(sy,jns,l);
	double m=1.-1./n;
	double Ss=pa(sy,jss,l);
	
	return teta_psi(psi, ice, s, res, a, n, m, pmin, Ss);
	
}

/******************************************************************************************************************************************/
//double psi_from_theta(double th, double ice, long l, double **pa, double pmin){
double psi_from_theta(double th, double ice, long l, const GeoTensor<double>& pa, const long& sy, double pmin)
{
	double s=pa(sy,jsat,l);
	double res=pa(sy,jres,l);
	double a=pa(sy,ja,l);
	double n=pa(sy,jns,l);
	double m=1.-1./n;
	double Ss=pa(sy,jss,l);
	
	return psi_teta(th, ice, s, res, a, n, m, pmin, Ss);
	
}

/******************************************************************************************************************************************/

//double dtheta_dpsi_from_psi(double psi, double ice, long l, double **pa, double pmin){			
double dtheta_dpsi_from_psi(double psi, double ice, long l, GeoTensor<double>& pa, const long& sy, double pmin)
{
	double s=pa[sy][jsat][l];
	double res=pa[sy][jres][l];
	double a=pa[sy][ja][l];
	double n=pa[sy][jns][l];
	double m=1.-1./n;
	double Ss=pa[sy][jss][l];
	
	return dteta_dpsi(psi, ice, s, res, a, n, m, pmin, Ss);
}


/******************************************************************************************************************************************/
//double k_from_psi(long jK, double psi, double ice, double T, long l, double **pa, double imp, double ratio){
double k_from_psi(long jK, double psi, double ice, double T, long l, GeoTensor<double>& pa, const long& sy, double imp, double ratio)
{	
	double kmax=pa[sy][jK][l];
	double s=pa[sy][jsat][l];
	double res=pa[sy][jres][l];
	double a=pa[sy][ja][l];
	double n=pa[sy][jns][l];
	double m=1.-1./n;
	double v=pa[sy][jv][l];
	
	return k_hydr_soil(psi, kmax, imp, ice, s, res, a, n, m, v, T, ratio);	
}

/******************************************************************************************************************************************/

double psisat_from(double ice, long l, GeoTensor<double>& pa, const long& sy)
{		
	double s=pa[sy][jsat][l];
	double res=pa[sy][jres][l];
	double a=pa[sy][ja][l];
	double n=pa[sy][jns][l];
	double m=1.-1./n;
	
	return psi_saturation(ice, s, res, a, n, m);	
}
/******************************************************************************************************************************************/
