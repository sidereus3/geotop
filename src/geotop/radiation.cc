
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
#include "radiation.h"
#include "geotop_common.h"
#include "inputKeywords.h"
#include <stdexcept>

using namespace mio;

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
void sun(double JDfrom0, double *E0, double *Et, double *Delta){
	
	double Gamma = 2.0*GTConst::Pi*convert_JDfrom0_JD(JDfrom0)/365.25;
	
	//correction sun-earth distance
	*E0=1.00011+0.034221*cos(Gamma)+0.00128*sin(Gamma)+0.000719*cos(2*Gamma)+0.000077*sin(2*Gamma);
	
	//Correction for sideral day (rad)	
	*Et=0.000075 + 0.001868*cos(Gamma) - 0.032077*sin(Gamma) - 0.014615*cos(2*Gamma) - 0.04089*sin(2*Gamma);	
	
	//Solar Declination 
	*Delta=0.006918-0.399912*cos(Gamma)+0.070257*sin(Gamma)-0.006758*cos(2*Gamma)+0.000907*sin(2*Gamma)-0.002697*cos(3*Gamma)+0.00148*sin(3*Gamma);
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
//angles in rad
double SolarHeight(double JD, double latitude, double Delta, double dh){
	
	double sine;
	//solar hour [0.0-24.0]
	double h = (JD-floor(JD))*24.0 + dh;	
	
	if(h>=24) h-=24.0;
	if(h<0) h+=24.0;
	
	sine = sin(latitude)*sin(Delta) + cos(latitude)*cos(Delta)*cos(GTConst::omega*(12-h));
	if (sine>1) sine = 1.;
	if (sine<-1) sine = -1.;
	
	return Fmax(asin(sine), 0.0);
}

double SolarHeight_(double JD, double *others){
	return SolarHeight(JD, others[0], others[1], others[2]);
}

double SolarHeight__(double JD, void *others){ return SolarHeight_(JD, (double *)others); }


/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
//angles in rad
double SolarAzimuth(double JD, double latitude, double Delta, double dh){
	
	double alpha, direction, sine, cosine;
	//solar hour [0.0-24.0]
	double h = (JD-floor(JD))*24.0 + dh;	
	
	if(h>=24) h-=24.0;
	if(h<0) h+=24.0;
	
	//solar height
	sine = sin(latitude)*sin(Delta) + cos(latitude)*cos(Delta)*cos(GTConst::omega*(12-h));
	if (sine>1) sine = 1.;
	if (sine<-1) sine = -1.;	
	alpha=asin(sine);
	
	//solar azimuth
	if(h<=12){
		if(alpha==GTConst::Pi/2.0){	//zenith
			direction=GTConst::Pi/2.0;
		}else{
			cosine=(sin(alpha)*sin(latitude)-sin(Delta))/(cos(alpha)*cos(latitude));
			if (cosine>1) cosine = 1.;
			if (cosine<-1) cosine = -1.;	
			direction=GTConst::Pi - acos(cosine);
		}
	}else{
		if(alpha==GTConst::Pi/2.0){ //zenith
			direction=3*GTConst::Pi/2.0;
		}else{
			cosine=(sin(alpha)*sin(latitude)-sin(Delta))/(cos(alpha)*cos(latitude));
			if (cosine>1) cosine = 1.;
			if (cosine<-1) cosine = -1.;	
			direction=GTConst::Pi + acos(cosine);
		}
	}
		
	return direction;
	
}

double SolarAzimuth_(double JD, double *others){
	return SolarAzimuth(JD, others[0], others[1], others[2]);
}

double SolarAzimuth__(double JD, void *others){ return SolarAzimuth_(JD, (double *)others); }

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double TauatmCosinc(double JD, double *others){
	
	/*double latitude = others[0];
	double Delta = others[1];
	double dh = others[2];*/
	double RH = others[3];
	double T = others[4];
	double P = others[5];
	double slope = others[6];
	double aspect = others[7];
	double Lozone = others[8];
	double alpha = others[9];
	double beta = others[10];
	double albedo = others[11];
	
	double height, dir;
	
	height = SolarHeight_(JD, others);
	if (height>0) {
		dir = SolarAzimuth_(JD, others);
		return atm_transmittance(Fmax(height,asin(0.05)),P,RH,T,Lozone,alpha,beta,albedo)*Fmax(0.0,cos(slope)*sin(height)+sin(slope)*cos(height)*cos(-aspect+dir));
	}else {
		return 0.0;
	}
}

double TauatmCosinc_(double JD, void *others){ return TauatmCosinc(JD, (double *)others); }


/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double TauatmSinalpha(double JD, double *others){
	
	/*double latitude = others[0];
	double Delta = others[1];
	double dh = others[2];*/
	double RH = others[3];
	double T = others[4];
	double P = others[5];
	double Lozone = others[8];
	double alpha = others[9];
	double beta = others[10];
	double albedo = others[11];
	
	double height;
	
	height = SolarHeight_(JD, others);
	if (height>0) {
		return atm_transmittance(Fmax(height,asin(0.05)),P,RH,T,Lozone,alpha,beta,albedo) * Fmax(sin(height),0.05);// Matteo, August 2015 corrected 0.5 with 0.0 according to UZH
    
        
	}else {
		return 0.0;
	}
}

double TauatmSinalpha_(double JD, void *others){ return TauatmSinalpha(JD, (double *)others); }

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double Cosinc(double JD, double *others){
	
	/*double latitude = others[0];
	double Delta = others[1];
	double dh = others[2];*/
	double slope = others[6];
	double aspect = others[7];
	
	double alpha, direction;
	
	alpha = SolarHeight_(JD, others);
	direction = SolarAzimuth_(JD, others);
	if (alpha>0) {
		return Fmax(0.0,cos(slope)*sin(alpha)+sin(slope)*cos(alpha)*cos(-aspect+direction));
	}else {
		return 0.0;
	}	
}

double Cosinc_(double JD, void *others){ return Cosinc(JD, (double *)others); }

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double Sinalpha(double JD, double *others){
	
	/*double latitude = others[0];
	double Delta = others[1];
	double dh = others[2];*/
	
	double alpha;
	
	alpha = SolarHeight_(JD, others);
	if (alpha>0) {
		return Fmax(sin(alpha), 0.05);
	}else {
		return 0.0;
	}
}

double Sinalpha_(double JD, void *others){ return Sinalpha(JD, (double *)others); }

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double Tauatm(double JD, double *others){
	
	/*double latitude = others[0];
	double Delta = others[1];
	double dh = others[2];*/
	double RH = others[3];
	double T = others[4];
	double P = others[5];
	double Lozone = others[8];
	double alpha = others[9];
	double beta = others[10];
	double albedo = others[11];	
	double height;
	
	height = SolarHeight_(JD, others);
	if (height < asin(0.05)) height = asin(0.05);
	return atm_transmittance(height, P, RH, T, Lozone, alpha, beta, albedo);
}

double Tauatm_(double JD, void *others){ return Tauatm(JD, (double *)others); }

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void shortwave_radiation(double JDbeg, double JDend, double *others, double sin_alpha, double E0, double sky, double SWrefl_surr, 
	double tau_cloud, short shadow, double *SWb, double *SWd, double *cos_inc_bd, double *tau_atm_sin_alpha, short *SWb_yes){

	double kd, tau_atm, cos_inc, tau_atm_cos_inc;
	double tau_cloud_to_use;
	
	tau_atm = adaptiveSimpsons2(Tauatm_, others, JDbeg, JDend, 1.E-6, 20) / (JDend - JDbeg);
	//tau_atm = Tauatm( 0.5*(JDbeg+JDend), others);

	kd=diff2glob(tau_cloud*tau_atm);

	*tau_atm_sin_alpha = adaptiveSimpsons2(TauatmSinalpha_, others, JDbeg, JDend, 1.E-6, 20) / (JDend - JDbeg);
	//*tau_atm_sin_alpha = tau_atm * sin_alpha;

	*SWd = GTConst::Isc*E0*tau_cloud*(*tau_atm_sin_alpha) * sky*kd + (1.-sky)*SWrefl_surr ;

	if (shadow == 1) {
		cos_inc = 0.0;
		*SWb = 0.0;
	}else {
		cos_inc = adaptiveSimpsons2(Cosinc_, others, JDbeg, JDend, 1.E-6, 20) / (JDend - JDbeg);
		//cos_inc = Cosinc( 0.5*(JDbeg+JDend), others);
		tau_atm_cos_inc = adaptiveSimpsons2(TauatmCosinc_, others, JDbeg, JDend, 1.E-8, 20) / (JDend - JDbeg);
		//tau_atm_cos_inc = tau_atm * cos_inc;
		//cos_inc = sin_alpha;
		//tau_atm_cos_inc = *tau_atm_sin_alpha;
		*SWb = (1.-kd)*GTConst::Isc*E0*tau_cloud*tau_atm_cos_inc;
	}
		
	*cos_inc_bd = kd*sin_alpha + (1.-kd)*cos_inc;
		
	if (sin_alpha > 1.E-5) {
		if (shadow == 1) {
			*SWb_yes = 0;
		}else {
			*SWb_yes = 1;
		}
	}else {
		*SWb_yes = -1;
	}


#ifdef VERY_VERBOSE
    printf("tauatm:%f tau_cloud:%f kd:%f tausin:%f sky:%f SWrefl:%f  \n",tau_atm,tau_cloud,kd,*tau_atm_sin_alpha,sky,SWrefl_surr);
    printf("sh:%d cosinc:%f taucos:%f cosincbd:%f SWb:%f SWd:%f \n",shadow,cos_inc,tau_atm_cos_inc,*cos_inc_bd,*SWb,*SWd);
#endif    
    
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double diff2glob(double a){

//Ratio diffuse to global radiation - Erbs et al.(1982)

	double k;
	if(a<0.22){
		k=1.0-0.09*a;
	}else if(a<0.80){
		k=0.9511-0.1604*a+4.388*pow(a,2.0)-16.638*pow(a,3.0)+12.336*pow(a,4.0);
	}else{
		k=0.165;
	}
	return(k);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
// 24.11.2016: commented out the whole routine in favour of ther iqbal version: 
// to check better what to do 
 
/* Antonio + Stefano E.: The function `atm_transmittance` has been
   changed back from a previous modifcation in order to ease the
   merging between Trento and UZH branches. The old code is in
   `atm_transmittance_iqbal`.

   In the future, it would be better to add a keyword in order to
   allow the user to decide which one of the two functions must be
   used.

double atm_transmittance(double X, double P, double RH, double T, double Lozone, double a, double b, double rho_g){
		
	//X = angle of the sun above the horizon [rad]
	//P = pressure [mbar]
	//RH = relative humidity [0-1]
	//T = air temperature [C]
	//from Mayers and Dale, Predicting Daily Insolation with Hourly Cloud Height and Coverage, 1983, pg 537, Journal of Climate and Applied Meteorology

	double tau_sa;//Reyleigh scattering and gas absorption transmittance
	double tau_w;//transmittance due to water vapor
	double tau_a;//transmittance due to aerosol
	double tau_atm;//global clear sky atmospheric transmittance
	double m;//optical air mass at sea level pressure
	double w;//precipitable water [cm]
	
	m = 35. * pow( 1224.*pow(sin(X), 2.) + 1. , -0.5 );
	tau_sa = 1.021 - 0.084 * pow(m*(0.000949*P + 0.051), 0.5);
	w = 0.493*RH*(exp(26.23-5416.0/(T+GTConst::tk)))/(T+GTConst::tk); 
	tau_w = 1. - 0.077*pow(w*m, 0.3);
	tau_a = pow(0.935, m);
	tau_atm = tau_sa*tau_w*tau_a;
	return(tau_atm);
}
*/

//double atm_transmittance_iqbal(double X, double P, double RH, double T, double Lozone, double a, double b, double rho_g){
double atm_transmittance(double X, double P, double RH, double T, double Lozone, double a, double b, double rho_g){
	
	//transmissivity under cloudless sky (Iqbal par. 7.5)
	double mr, ma, w, U1, U3, tau_r, tau_o, tau_g, tau_w, tau_a, tau_aa, rho_a;
	double tau_atm_n, tau_atm_dr, tau_atm_da, tau_atm_dm, tau_atm;
	double w0 = 0.9;
	double Fc = 0.84;
	
	mr = 1./(sin(X)+0.15*(pow((3.885+X*180.0/GTConst::Pi),-1.253)));
	ma = mr*P/GTConst::Pa0;
	w = 0.493*RH*(exp(26.23-5416.0/(T+GTConst::tk)))/(T+GTConst::tk); //cm
	U1 = w*mr;
	U3 = Lozone*mr;
	tau_r = exp(-.0903*pow(ma,.84)*(1.+ma-pow(ma,1.01)));
	tau_o = 1. - (.1611*U3*pow(1.+139.48*U3, -.3035) - .002715*U3/(1.+.044*U3+.0003*U3*U3));
	tau_g = exp(-.0127*pow(ma,.26));
	tau_w = 1. - 2.4959*U1/(pow(1.+79.034*U1,.6828)+6.385*U1);
	//tau_a from 7.4.11 Iqbal
	tau_a = .12445*a - 0.0162 + (1.003 - .125*a) * exp(-b*ma*(1.089*a + .5123));
	tau_aa = 1. - (1. - w0)*(1. - ma + pow(ma, 1.06))*(1. - tau_a);
	rho_a = .0685 + (1. - Fc)*(1. - tau_a/tau_aa);
	tau_atm_n = .9751 * tau_r * tau_o * tau_g * tau_w * tau_a;
	tau_atm_dr = .79 * tau_o * tau_g * tau_w * tau_aa * .5 * (1.-tau_r) / (1. - ma + pow(ma, 1.02));
	tau_atm_da = .79 * tau_o * tau_g * tau_w * tau_aa * Fc * (1.-tau_a/tau_aa) / (1. - ma + pow(ma, 1.02));
	tau_atm_dm = (tau_atm_n + tau_atm_dr + tau_atm_da) * rho_g * rho_a / (1. - rho_g * rho_a);
	tau_atm = tau_atm_n + tau_atm_dr + tau_atm_da + tau_atm_dm;
	
	return(tau_atm);

}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void longwave_radiation(short state, double pvap, double RH, double T, double k1, double k2, double tau_cloud, double *eps, double *eps_max, double *eps_min){

	double taucloud_overcast=0.29;//after Kimball(1928)
	FILE *f;

    if(state==1){
		*eps_min = 1.24*pow((pvap/(T+GTConst::tk)),1./7.); //Brutsaert, 1975

	}else if(state==2){
		*eps_min = 1.08*(1.0-exp(-pow(pvap,(T+GTConst::tk)/2016.0)));	//Satterlund, 1979
		
	}else if(state==3){
		*eps_min = (0.7+5.95*0.00001*pvap*exp(1500/(T+GTConst::tk)));	//Idso(1981)
		
	}else if(state==4){
		*eps_min = (0.7+5.95*0.00001*pvap*exp(1500/(T+GTConst::tk)));
		*eps_min = -0.792 + 3.161*(*eps_min) - 1.573*(*eps_min)*(*eps_min);	//IDSO + HODGES		

	}else if(state==5){
		*eps_min = 0.765;	//Koenig-Langlo & Augstein, 1994

	}else if(state==6){
		*eps_min = (0.601+5.95*0.00001*pvap*exp(1500.0/(T+GTConst::tk)));//Andreas and Ackley, 1982

	}else if(state==7){
		*eps_min = (0.23+k1*pow((pvap*100.)/(T+GTConst::tk),1./k2));	//Konzelmann (1994)
		
	}else if(state==8){
		*eps_min = (1.-(1.+46.5*pvap/(T+GTConst::tk))*exp(-pow(1.2+3.*46.5*pvap/(T+GTConst::tk) , 0.5)));//Prata 1996
		
	}else if(state==9){
		*eps_min = ( 59.38 + 113.7*pow( (T+GTConst::tk)/273.16 , 6. ) + 96.96*pow((465.*pvap/(T+GTConst::tk))/25., 0.5) ) / (5.67E-8*pow(T+GTConst::tk, 4.));//Dilley 1998

	}else{

		f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
		fprintf(f,"Error:: Incorrect value for longwave radiation formula\n");
		fclose(f);
		t_error("Fatal Error! Geotop is closed. See failing report.");	

	}
	
	*eps = (*eps_min) * tau_cloud + 1.0 * (1.-tau_cloud);
	*eps_max = (*eps_min) * taucloud_overcast + 1.0 * (1.-tau_cloud);
		
	/*double fcloud;
	fcloud=pow((1.0-taucloud)/0.75,1/3.4);
	*eps=(*eps_min)*(1.0-pow(fcloud,6.0))+0.979*pow(fcloud,4.0); Pirazzini*/
  
#ifdef VERY_VERBOSE
    printf("state:%d T:%f pvap:%f eps:%f taucloud:%f epsmin:%f epsmax:%f\n",state,T,pvap,*eps,tau_cloud,*eps_min,*eps_max);
#endif

}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double SB(double T){	//Stefan-Boltzmann law
	double R;
	R=5.67E-8*pow(T+GTConst::tk,4.0);
	return(R);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double dSB_dT(double T){
	double dR_dT;
	dR_dT=4.0*5.67E-8*pow(T+GTConst::tk,3.0);
	return(dR_dT);
}


/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

  void rad_snow_absorption(long r, long c, GeoVector<double>& frac, double R, Statevar3D *snow){

	long l, m;
	double res=R, z=0.0, rho, k;

	frac.resize(frac.size(),0.);
		
	//in case of snow
	if( snow->lnum[r][c] > 1){
		
		for(l=snow->lnum[r][c];l>=1;l--){
			m=snow->lnum[r][c]-l+1;
			z+=0.001*snow->Dzl[l][r][c];
			rho=(snow->w_ice[l][r][c]+snow->w_liq[l][r][c])/(0.001*snow->Dzl[l][r][c]);
			k=rho/3.0+50.0;
            frac[m]=res-R*exp(-k*z);
			res=R*exp(-k*z);
		}
	
			frac[snow->lnum[r][c]+1]=res;
		
	}else {
           frac[0] = res;
	}
}
	
/******************************************************************************************************************************************/
double cloud_transmittance(double JDbeg, double JDend, double lat, double Delta, double dh, double RH, double T,
						   double P, double SWd, double SWb, double SW, double E0, double sky, double SWrefl_surr, double Lozone, double alpha, double beta, double albedo){
	
	double *others;
	double tau_atm;// atmosphere transmittance
	double tau_atm_sin_alpha;
	double sin_alpha;// solar elevationa angle
	double kd;
	double kd0;
	double tau = geotop::input::gDoubleNoValue;
	long j;
			
	others = (double*)malloc(12*sizeof(double));
	others[0] = lat;
	others[1] = Delta;
	others[2] = dh;
	others[3] = RH;
	others[4] = T;
	others[5] = P;
	others[8] = Lozone;
	others[9] = alpha;
	others[10] = beta;
	others[11] = albedo;
	
	tau_atm_sin_alpha = adaptiveSimpsons2(TauatmSinalpha_, others, JDbeg, JDend, 1.E-6, 20) / (JDend - JDbeg);
	//tau_atm = Tauatm( 0.5*(JDbeg+JDend), others);
	//sin_alpha = Sinalpha( 0.5*(JDbeg+JDend), others);
	//tau_atm_sin_alpha = tau_atm*sin_alpha;
	double minTauAtmSinAlpha = 0.00;
	if (tau_atm_sin_alpha > minTauAtmSinAlpha) {// matteo added sept 2015 to avoid problems in calculating Tau at sunset when solarHeight is very low
		if((long)SWd!=geotop::input::gDoubleAbsent && (long)SWd!=geotop::input::gDoubleNoValue && (long)SWb!=geotop::input::gDoubleAbsent && (long)SWb!=geotop::input::gDoubleNoValue){
			if( SWb+SWd > 0 && SWd > 0){
				kd = SWd / (Fmax(0.,SWb)+SWd);
				tau = ( SWd - (1.-sky)*SWrefl_surr ) / ( GTConst::Isc*E0*tau_atm_sin_alpha*sky*kd );
			}
			
		}else if((long)SW!=geotop::input::gDoubleAbsent && (long)SW!=geotop::input::gDoubleNoValue){
			
			kd=0.2;
			tau_atm = adaptiveSimpsons2(Tauatm_, others, JDbeg, JDend, 1.E-6, 20) / (JDend - JDbeg);
			sin_alpha = adaptiveSimpsons2(Sinalpha_, others, JDbeg, JDend, 1.E-6, 20) / (JDend - JDbeg);
			j=0;
			
			do{
				
				j++;
				kd0=kd;
				//SW = (1-kd(T))*Isc*T*sin + sky*Kd(T)*Isc*T*sin + (1-sky)*SWsurr
				//T=Ta*Tc
				//SW - (1-sky)*SWsurr = Tc * (Isc*Ta*sin) * ( (1-kd) + sky*kd )
				//Tc = ( SW - (1-sky)*SWsurr ) / ( (Isc*Ta*sin) * ( (1-kd) + sky*kd )
				tau = ( SW - (1.-sky)*SWrefl_surr ) / ( GTConst::Isc*E0*tau_atm_sin_alpha * ( (1-kd) + sky*kd ) );
				if(tau > 1) tau = 1.0; // commented after check against UZH (Matteo, August 2015)
                                                       // decommented during debug session (S.C +S.E 24/11/2016)
				if(tau < 0) tau = 0.0;
				kd = diff2glob(tau * tau_atm);
			}while(fabs(kd0-kd)>0.005 && j<1000);
			
		}
	}
		
	if( (long)tau != geotop::input::gDoubleNoValue){
		if(tau<GTConst::min_tau_cloud) tau=GTConst::min_tau_cloud;
	}
	
	free(others);
		
	return tau;
}
	
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double find_tau_cloud_station(double JDbeg, double JDend, long station_number, Meteo *met, const std::vector<mio::MeteoData>& vec_meteo,
                              double Delta, double E0, double Et, double ST, double SWrefl_surr,double Lozone, double alpha, double beta, double albedo)
{
	double P, RH, T, c;
	double tdew = geotop::input::gDoubleNoValue;
	double swDirect=geotop::input::gDoubleNoValue, swDiffuse=geotop::input::gDoubleNoValue;

	const MeteoData& current = vec_meteo.at(station_number-1); //MeteoIO starts counting at 0
	size_t sw_direct = current.getParameterIndex("SWdirect");

	if ((sw_direct != IOUtils::npos) && (current(sw_direct) != IOUtils::nodata)) { //check for SWb
		size_t sw_diffuse = current.getParameterIndex("Swdiffuse");
		if ((sw_diffuse != IOUtils::npos) && (current(sw_diffuse) != IOUtils::nodata)) { //check for SWd
			swDirect = current(sw_direct);
			swDiffuse = current(sw_diffuse);
		}
	}

	if ((current(MeteoData::TA) != IOUtils::nodata) && 
	    (current(MeteoData::RH) != IOUtils::nodata) && (current(MeteoData::P) != IOUtils::nodata)) {
		tdew = tDew(current(MeteoData::TA)-GTConst::tk, current(MeteoData::RH)*100.0, current(MeteoData::P) / 100.0);
	}

	//pressure [mbar]
	P = pressure(current.meta.position.getAltitude());

	//relative humidity 
	if (current(MeteoData::RH) != IOUtils::nodata) {
		RH = current(MeteoData::RH);
	} else {
		if ((current(MeteoData::TA) != IOUtils::nodata) && (tdew != geotop::input::gDoubleNoValue)) {
			RH = RHfromTdew(current(MeteoData::TA)-GTConst::tk, tdew, current.meta.position.getAltitude());
		} else {
			RH = 0.4;
		}
	}
	
	if (RH < 0.01) RH = 0.01;
		
	T = current(MeteoData::TA) - GTConst::tk;
	if (current(MeteoData::TA) == IOUtils::nodata) T = 0.0;

	c = cloud_transmittance(JDbeg, JDend, current.meta.position.getLat()*GTConst::Pi/180., Delta,
					    (current.meta.position.getLon() * GTConst::Pi/180. - ST * GTConst::Pi/12. + Et)/GTConst::omega, RH,
					    T, P, swDiffuse, swDirect, current(MeteoData::ISWR), E0, met->st->sky[station_number], SWrefl_surr, Lozone, alpha, beta, albedo);

	return c;
}

//double find_tau_cloud_station_meteodistr(double JDbeg, double JDend, long i, Meteo *met,
//						double Delta, double E0, double Et, double ST, double SWrefl_surr)

double find_tau_cloud_station_meteodistr(double JDbeg, double JDend, long i, Meteo *met, double Delta, double E0, double Et,
							  double ST, double SWrefl_surr, double Lozone, double alpha, double beta, double albedo)
{
	double P, RH, T, c;

	//pressure [mbar]
	P=pressure(met->st->Z[i]);

	//relative humidity
	if((long)met->var[i-1][iRh] != geotop::input::gDoubleNoValue && (long)met->var[i-1][iRh] != geotop::input::gDoubleAbsent){
		RH=met->var[i-1][iRh]/100.;
	}else {
		if ( (long)met->var[i-1][iT] != geotop::input::gDoubleAbsent && (long)met->var[i-1][iT] != geotop::input::gDoubleNoValue && (long)met->var[i-1][iTdew] != geotop::input::gDoubleAbsent && (long)met->var[i-1][iTdew] != geotop::input::gDoubleNoValue){
			RH=RHfromTdew(met->var[i-1][iT], met->var[i-1][iTdew], met->st->Z[i]);
		}else {
			RH=0.4;
		}
	}
	if(RH<0.01) RH=0.01;

	T=met->var[i-1][iT];
	if((long)T == geotop::input::gDoubleNoValue || (long)T == geotop::input::gDoubleAbsent) T=0.0;

	c = cloud_transmittance(JDbeg, JDend, met->st->lat[i]*GTConst::Pi/180., Delta, (met->st->lon[i]*GTConst::Pi/180. - ST*GTConst::Pi/12. + Et)/GTConst::omega, RH,
							T, P, met->var[i-1][iSWd], met->var[i-1][iSWb], met->var[i-1][iSW], E0, met->st->sky[i], SWrefl_surr,Lozone, alpha, beta, albedo);

	return c;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

short shadows_point(double **hor, long n, double alpha, double azimuth, double tol_mount, double tol_flat)

/** routine that tells you whether a point is in shadow or not, depending on the solar azimuth, elevation and horizon file at that point
 * @author: Matteo Dall'Amico, May 2011
 *Inputs: DOUBLEMATRIX* hor_height: matrix of horizon_height at the point
 *double alpha: solar altitude (degree)
 *double azimuth: solar azimuth (degree)
 *double tol_mount: tolerance over a mountaneaus horizon to have a reliable cloud datum (degree)
 *double tol_flat: tolerance over a mountaneaus horizon to have a reliable cloud datum (degree)
 * Output: shad: 1=the point is in shadow, 0 the point is in sun
 */
{
	double horiz_H;// horizon elevation at a defined solar azimuth
	double w;//weight
	long i, iend = -1, ibeg = -1;
	short shad; 
	
	/* compare the current solar azimuth with the horizon matrix */
	if(azimuth>=hor[n-1][0] || azimuth<hor[0][0]){
		iend=0;
		ibeg=n-1;
	}else{
		for (i=1; i<=n-1; i++){
			if(azimuth>=hor[i-1][0] && azimuth<hor[i][0]){
				iend=i;
				ibeg=i-1;
			}
		}
	}

    if (ibeg < 0)
    {
        std::out_of_range e("ibeg");
        throw e;
    }

    if (iend < 0)
    {
        std::out_of_range e("iend");
        throw e;
    }
	
	if (iend>ibeg) {
		w=(azimuth-hor[ibeg][0])/(hor[iend][0]-hor[ibeg][0]);
	}else if (azimuth>hor[ibeg][0]) {
		w=(azimuth-hor[ibeg][0])/(hor[iend][0]+360.0-hor[ibeg][0]);
	}else {
		w=(azimuth-(hor[ibeg][0]-360.0))/(hor[iend][0]-(hor[ibeg][0]-360.0));
	}

	horiz_H = w * hor[iend][1] + (1.0-w) * hor[ibeg][1]; // horizon elevation at a particular time

	if(alpha<tol_flat){
		shad=1;
	}else if(alpha<horiz_H+tol_mount){
		shad=1;
	}else{
		shad=0;
	}
		
	return (shad);
}

/******************************************************************************************************************************************/

void shadow_haiden(GeoMatrix<double>& Z, double alpha, double direction, GeoMatrix<short>& SH)

/**	@Author: Thomas Haiden, Year: 16 june 2003
 * 	@brief Function that calculates if each pixel (matrix of shadows) is in shadow or at sun given
 *  a solar elevation angle and a solar azimuth.
 *  Inputs:	top 		elevation DTM (m)
 *  		alpha:		solar elevation angle (radiants)
 *  		direction:	solar azimuth angle (from N clockwise, radiants)
 *  		point: flag indicating whether the simulation is a point (=1) or a distributed simulation
 *  Outputs:shadow: 	shadows matrix (1 shadow, 0 sun)
 *  imported by Matteo Dall'Amico on August 2009. Authorization: see email of David Whiteman on 16 june 2011
 *  revised by Stefano Endrizzi on May 2011.
 */
{
	long orix,oriy,k,l,kk,ll,r,c,rr,cc;
	double sx,sy,sz,xp,yp,x,y,zray,ztopo,q,z1,z2;
	
	long nk=geotop::common::Variables::Nr;//y
	long nl=geotop::common::Variables::Nc;//x
	double GDX=geotop::common::Variables::UV->U[2];
	double GDY=geotop::common::Variables::UV->U[1];
					
	sx=sin(direction)*cos(alpha);
	sy=cos(direction)*cos(alpha);
	sz=sin(alpha);
				
	if (fabs(sx)>fabs(sy)) { 
		
		if (sx>0) {
			orix=1;
		} else {
			orix=-1;
		}
		
		if (fabs(sy)>1.E-10) {
			if (sy>0) {
				oriy=1;
			} else {
				oriy=-1;
			}
		} else {
			oriy=0;
		}
		
		for (k=0; k<nk; k++) {
			for (l=0; l<nl; l++) {
				r=geotop::common::Variables::Nr-k;
				c=l+1;
				kk=k;
				ll=l;
				xp=GDX*ll+0.5*GDX;
				yp=GDY*kk+0.5*GDY;
				rr=geotop::common::Variables::Nr-kk;
				cc=ll+1;
				zray=Z[rr][cc];
				SH[r][c]=0;
				
				while ( (SH[r][c]==0) && (kk>0)&&(kk<nk-1)&&(ll>0)&&(ll<nl-1) ) {
					q=((ll+orix)*GDX+0.5*GDX-xp)/sx;
					y=yp+q*sy;
					if (fabs(y-(GDY*kk+0.5*GDY))<GDY) {
						ll=ll+orix;
						xp=GDX*ll+0.5*GDX;
						yp=y;
						rr=geotop::common::Variables::Nr-kk;
						cc=ll+1;
						z1=Z[rr][cc];
						rr=geotop::common::Variables::Nr-(kk+oriy);
						z2=Z[rr][cc];
						ztopo=z1+(z2-z1)*(yp-(GDY*kk+0.5*GDY))/(oriy*GDY);
					}else {
						q=((kk+oriy)*GDY+0.5*GDY-yp)/sy;
						x=xp+q*sx;
						kk=kk+oriy;
						xp=x;
						yp=GDY*kk+0.5*GDY;
						rr=geotop::common::Variables::Nr-kk;
						cc=ll+1;						
						z1=Z[rr][cc];
						cc=ll+orix+1;
						z2=Z[rr][cc];
						ztopo=z1+(z2-z1)*(xp-(GDX*ll+0.5*GDX))/(orix*GDX);
					}
					zray=zray+q*sz; 	
					if (ztopo>zray) SH[r][c]=1;
				} 
			}
		}

	} else {
		
		if (sy>0) {
			oriy=1;
		}else {
			oriy=-1;
		}

		if (fabs(sx)>1.E-10) {
			if (sx>0) {
				orix=1;
			}else {
				orix=-1;
			}
		}else {
			orix=0;
		}

		for (k=0; k<nk; k++) {
			for (l=0; l<nl; l++) {
				r=geotop::common::Variables::Nr-k;
				c=l+1;
				kk=k;
				ll=l;
				xp=GDX*ll+0.5*GDX;
				yp=GDY*kk+0.5*GDY;
				rr=geotop::common::Variables::Nr-kk;
				cc=ll+1;
				zray=Z[rr][cc];
				SH[r][c]=0;
				
				while ( (SH[r][c]==0) &&(kk>0)&&(kk<nk-1)&&(ll>0)&&(ll<nl-1)) {
					q=((kk+oriy)*GDY+0.5*GDY-yp)/sy;
					x=xp+q*sx;
					if (fabs(x-(GDX*ll+0.5*GDX))<GDX) {
						kk=kk+oriy;
						yp=GDY*kk+0.5*GDY;
						xp=x;
						rr=geotop::common::Variables::Nr-kk;
						cc=ll+1;
						z1=Z[rr][cc];
						cc=ll+orix+1;
						z2=Z[rr][cc];
						ztopo=z1+(z2-z1)*(xp-(GDX*ll+0.5*GDX))/(orix*GDX);
					} else {
						q=((ll+orix)*GDX+0.5*GDX-xp)/sx;
						y=yp+q*sy;
						ll=ll+orix;
						yp=y;
						xp=GDX*ll+0.5*GDX;
						rr=geotop::common::Variables::Nr-kk;
						cc=ll+1;
						z1=Z[rr][cc];
						rr=geotop::common::Variables::Nr-(kk+oriy);
						z2=Z[rr][cc];
						ztopo=z1+(z2-z1)*(yp-(GDY*kk+0.5*GDY))/(oriy*GDY);
					}
					zray=zray+q*sz; 	
					if (ztopo>zray) SH[r][c]=1;
				} 
			}
		}
	}
}
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double find_albedo(double dry_albedo, double sat_albedo, double wat_content, double residual_wc, double saturated_wc){

	return (dry_albedo + (sat_albedo-dry_albedo) * (wat_content - residual_wc) / (saturated_wc - residual_wc) );

}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void find_actual_cloudiness_meteoio(double *tau_cloud, double *tau_cloud_av, short *tau_cloud_yes, short *tau_cloud_av_yes, int meteo_stat_num,
					   Meteo *met, const std::vector<mio::MeteoData>& vec_meteo, double JDb, double JDe, double Delta,
					   double E0, double Et, double ST, double SWrefl_surr, double Lozone, double alpha, double beta, double albedo)
{
	short SWdata = 0; //flag indicating which type of SW data available: 0=none, 1=global, 2=beam and diff measured
	double tc;

	const MeteoData& current = vec_meteo.at(meteo_stat_num-1); //MeteoIO starts counting at 0
	size_t sw_direct = current.getParameterIndex("SWdirect");
	size_t cloud_factor = current.getParameterIndex("CloudFactor");

	if ((sw_direct != IOUtils::npos) && (current(sw_direct) != IOUtils::nodata)) { //check for SWb
		size_t sw_diffuse = current.getParameterIndex("Swdiffuse");
		if ((sw_diffuse != IOUtils::npos) && (current(sw_diffuse) != IOUtils::nodata)) { //check for SWd
			SWdata = 2;
		}
	} else if (current(MeteoData::ISWR) != IOUtils::nodata) {
		SWdata = 1;
	}
	//calculate tau_cloud instantaneous
	if (SWdata > 0) {
		tc = find_tau_cloud_station(JDb, JDe, meteo_stat_num, met, vec_meteo, Delta, E0, Et, ST, SWrefl_surr,Lozone, alpha, beta, albedo);
		if ((long)tc != geotop::input::gDoubleNoValue) {
			*tau_cloud_yes = 1;
			*tau_cloud = tc;
		} else {
			*tau_cloud_yes = 0;
		}		
	} else {
		*tau_cloud_yes = 0;
	}
	//calculate tau_cloud average
	if ((cloud_factor != IOUtils::npos) && (current(cloud_factor) != IOUtils::nodata)) {
		tc = current(cloud_factor);

		*tau_cloud_av_yes = 1;
		tc = 1. - 0.71*tc; //from fraction of sky covered by clouds to cloud transmissivity after Kimball (1928)
		if(tc > 1) tc = 1.;
		if(tc < 0) tc = 0.;
		*tau_cloud_av = tc;
	} else if (current(MeteoData::TAU_CLD) != IOUtils::nodata) {
		tc = current(MeteoData::TAU_CLD);

		*tau_cloud_av_yes = 1;
		if(tc > 1) tc = 1.;
		if(tc < 0) tc = 0.;
		*tau_cloud_av = tc;
	} else {
		*tau_cloud_av_yes = 0;
	}
}

void find_actual_cloudiness(double *tau_cloud, double *tau_cloud_av, short *tau_cloud_yes, short *tau_cloud_av_yes,int meteo_stat_num,
							Meteo *met, double JDb, double JDe, double Delta, double E0, double Et, double ST, double SWrefl_surr, double Lozone, double alpha, double beta, double albedo)
{

	//SWdata = flag -> 0= no radiation data measured, 1=beam and diff measured, 2=global measured
	short SWdata;
	double tc;

	if((long)met->var[met->nstsrad-1][iSWb]!=geotop::input::gDoubleAbsent && (long)met->var[met->nstsrad-1][iSWd]!=geotop::input::gDoubleAbsent){
		if((long)met->var[met->nstsrad-1][iSWb]!=geotop::input::gDoubleNoValue && (long)met->var[met->nstsrad-1][iSWd]!=geotop::input::gDoubleNoValue){
			SWdata=2;
		}else{
			SWdata=0;
		}
	}else if((long)met->var[met->nstsrad-1][iSW]!=geotop::input::gDoubleAbsent){
		if((long)met->var[met->nstsrad-1][iSW]!=geotop::input::gDoubleNoValue){
			SWdata=1;
		}else{
			SWdata=0;
		}
	}else{
		SWdata=0;
	}
    
	if(SWdata>0){
		tc = find_tau_cloud_station_meteodistr(JDb, JDe, met->nstsrad, met, Delta, E0, Et, ST, SWrefl_surr, Lozone, alpha, beta, albedo);
		if ( (long)tc != geotop::input::gDoubleNoValue){
			*tau_cloud_yes = 1;
			*tau_cloud = tc;
		}else {
			*tau_cloud_yes = 0;
		}
        
	}else{
		*tau_cloud_yes = 0;
	}

	if( (long)met->var[met->nstcloud-1][iC]!=geotop::input::gDoubleAbsent && (long)met->var[met->nstcloud-1][iC]!=geotop::input::gDoubleNoValue ){

		tc = met->var[met->nstcloud-1][iC];
		*tau_cloud_av_yes = 1;
		tc = 1. - 0.71*tc;//from fraction of sky covered by clouds to cloud transmissivity after Kimball (1928)
		if(tc > 1) tc = 1.;
		if(tc < 0) tc = 0.;
		*tau_cloud_av = tc;

	}else if( (long)met->var[met->nstcloud-1][itauC]!=geotop::input::gDoubleAbsent && (long)met->var[met->nstcloud-1][itauC]!=geotop::input::gDoubleNoValue ){

		tc = met->var[met->nstcloud-1][itauC];
		*tau_cloud_av_yes = 1;
		if(tc > 1) tc = 1.;
		if(tc < 0) tc = 0.;
		*tau_cloud_av = tc;

	}else{

		*tau_cloud_av_yes = 0;

	}

    
}
//*****************************************************************************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************
//*****************************************************************************************************************

// added from meteodirst.c

double find_cloudfactor(double Tair, double RH, double Z, double T_lapse_rate, double Td_lapse_rate){

	double fcloud, Z_ref, press_ratio, f_max, one_minus_RHe, f_1, Td, Td_700, Tair_700, rh_700;

#ifdef VERY_VERBOSE
    printf("Tair:%f RH:%f Z:%f LP1:%f LP2:%f\n",Tair,RH,Z,T_lapse_rate,Td_lapse_rate);
#endif
	//Assume that 700 mb is equivalent to 3000 m in a standard atmosphere.
	Z_ref = 3000.0;

	//Define the ratio of 700 mb level pressure to the surface pressure (~1000 mb).
	press_ratio = 0.7;

	//Assume dx = 80.0 km, for Walcek (1994).

	//Walcek coefficients.
	f_max = 78.0 + 80.0/15.5;
	one_minus_RHe = 0.196 + (0.76-80.0/2834.0) * (1.0 - press_ratio);
	f_1 = f_max * (press_ratio - 0.1) / 0.6 / 100.0;

	//Convert the gridded topo-surface RH to Td.
	Td = Tdew(Tair, Fmax(0.1, RH), Z);

	//Convert the topo-surface temperature values to 700 mb values.
	Td_700 = temperature(Z_ref, Z, Td, Td_lapse_rate);
	Tair_700 = temperature(Z_ref, Z, Tair, T_lapse_rate);

	//Convert each Td to a gridded relative humidity (0-1).
    rh_700 = RHfromTdew(Tair_700, Td_700, Z);

	//Use this RH at 700 mb to define the cloud fraction (0-1).
	fcloud = f_1 * exp((rh_700 - 1.0)/one_minus_RHe);
	fcloud = Fmin(1.0, fcloud);
	fcloud = Fmax(0.0, fcloud);

	return(fcloud);

}


