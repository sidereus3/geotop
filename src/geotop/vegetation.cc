
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

#include "vegetation.h"
#include "geotop_common.h"

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void Tcanopy(long r, long c, double Tv0, double Tg, double Qg, double dQgdT, double Tg0, double Qg0, double Ta, double Qa, 
		double zmu, double zmT, double z0, double z0s, double d0, double z0r, double hveg, double v, double LR, double P, 
		double SW, double SWv, double LW, double e, double LSAI, double decaycoeff0, const GeoMatrix<double>& land, long lu, double Wcrn0, double Wcrnmax,
		double Wcsn0, double Wcsnmax, double *dWcrn, double *dWcsn, double *LWv, double *LWg, double *Hv, double *Hg, 
		double *dHgdT, double *LEv, double *Eg, double *dEgdT, double *Ts, double *Qs, const GeoMatrix<double>& froot, double *theta,
		GeoVector<double>& soil_transp_layer, double *Lobukhov, Par *par, long n, double *rm, double *rh, double *rv, double *rc,
		double *rb, double *ruc, double *u_top, double *Etrans, double *Tv, double *Qv, double *decay, double *Locc,
		   double *LWup_above_v, double psi, GeoTensor<double>& soil, long sy, double *T, GeoVector<double>& soil_evap_layer){


	double C, C0;
	double A=1.0;
	double T00=Tv0, T10, T11=Tv0, T11p=Tv0, DT, Wcrn=Wcrn0, Wcsn=Wcsn0;
	double err0, err1=1.E+99, nw;
	double Lobukhov0, h0=0.0, h1, dhdT;
	double subl_can, melt_can, fwliq, fwice;
	double alpha, beta;
	long cont, cont2, chgsgn=0;
	short a=0;
	FILE *f;

	//vegetation thermal capacity
	C0=land(lu,jcd)*LSAI*GTConst::c_can + GTConst::c_ice*Wcsn + GTConst::c_liq*Wcrn;
	C=C0; 
	
	fwliq=pow(Wcrn/Wcrnmax,2./3.);
	fwice=pow(Wcsn/Wcsnmax,2./3.);	
		
	//calculates values at the instant t0 -> h0
	if(A<1){		
		*Ts=0.5*Tg0+0.5*Ta;
		*Qs=0.5*Qg0+0.5*Qa;		
		canopy_fluxes(r, c, T00, Tg0, Ta, Qg0, Qa, zmu, zmT, z0, z0s, d0, z0r, hveg, v, LR, P, SW, LW, e, LSAI, decaycoeff0, land, lu,
				    Wcrn0, Wcrnmax, Wcsn0, Wcsnmax, &subl_can, Etrans, LWv, LWg, Hv, LEv, &h0, &dhdT, Ts, Qs, Qv, ruc,  
				    froot, theta, soil_transp_layer, chgsgn, Lobukhov, par, n, rm, rh, rv, rc, rb, u_top, decay, Locc, LWup_above_v,
				    psi, soil, sy, &alpha, &beta, T, soil_evap_layer); 
	}

	//calculates values at the instant t1 -> h1, through iterations
	cont=0;
	chgsgn=0;	
	*Ts=0.5*Tg+0.5*Ta;
	*Qs=0.5*Qg+0.5*Qa;				
	canopy_fluxes(r, c, T11, Tg, Ta, Qg, Qa, zmu, zmT, z0, z0s, d0, z0r, hveg, v, LR, P, SW, LW, e, LSAI, decaycoeff0, land, lu, Wcrn0,
			    Wcrnmax, Wcsn0, Wcsnmax, &subl_can, Etrans, LWv, LWg, Hv, LEv, &h1, &dhdT, Ts, Qs, Qv, ruc, froot, theta, soil_transp_layer, chgsgn, 
			    Lobukhov, par, n, rm, rh, rv, rc, rb, u_top, decay, Locc, LWup_above_v, psi, soil, sy, &alpha, &beta, T, soil_evap_layer); 
	
	do{

		T10=T11;
		
		//Generalized Newton-Raphson
		cont2=0;
		nw=1.0;
		err0=fabs( C*(T11-T00)/par->Dt - SWv - A*h1 - (1.-A)*h0 );
		
		//eq. C*(T11-T00)/Dt = cost + h(T10) + dh(T10)/dT * (T11-T10)
		//eq. C*(T11-T10)/Dt + C*(T10-T00)/Dt = cost + h(T10) + dh(T10)/dT * (T11-T10)
		//eq. T11-T10 = ( cost + h(T10) - C*(T10-T00)/Dt ) / ( C/Dt - dh/dT )
		
		DT=( -C*(T10-T00)/par->Dt + SWv + A*h1 + (1.0-A)*h0 ) / ( C/par->Dt - A*dhdT );
		
		if(DT!=DT){
			f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
			fprintf(f,"Error:: NwRph Tcanopy T00:%f T10:%f SWv:%f h0:%f h1:%f dhdT:%f C:%f Wcsn:%f Wcrn:%f %ld %ld\n",T00,T10,SWv,h0,h1,dhdT,C,Wcsn,Wcrn,r,c); 
			fclose(f);
			t_error("Fatal Error! Geotop is closed. See failing report.");	
		}
			
		Lobukhov0=(*Lobukhov);
		
		do{
			
			T11 = T10 + nw*DT;	
			
			if(subl_can<0 && T11<0){ //condensation as frost
				Wcsn=Wcsn0-subl_can*par->Dt;
				Wcrn=Wcrn0;
				
			}else if(subl_can<0 && T11>=0){ //condensation as dew
				Wcrn=Wcrn0-subl_can*par->Dt;
				Wcsn=Wcsn0;
				
			}else{	//partly evaporation, partly sublimation
				if(fwliq+fwice>0){
					Wcsn=Wcsn0-(fwice/(fwliq+fwice))*subl_can*par->Dt;
					Wcrn=Wcrn0-(fwliq/(fwliq+fwice))*subl_can*par->Dt;				
				}else{
					Wcsn=Wcsn0;
					Wcrn=Wcrn0;
				}
			}
						
			if(Wcrn>Wcrnmax) Wcrn=Wcrnmax;
			if(Wcrn<0) Wcrn=0.0;
			if(Wcsn>Wcsnmax) Wcsn=Wcsnmax;
			if(Wcsn<0) Wcsn=0.0;			
						
			if(T11>0 && Wcsn>0){	//melting
				melt_can=Fmin(Wcsn, GTConst::c_ice*Wcsn*(T11-0.0)/GTConst::Lf);
				T11p=T11 - GTConst::Lf*melt_can/C;
				Wcsn-=melt_can;
				Wcrn+=melt_can;
				
			}else if(T11<0 && Wcrn>0){  //freezing
				melt_can=-Fmin(Wcrn, GTConst::c_liq*Wcrn*(0.0-T11)/GTConst::Lf);
				T11p=T11 - GTConst::Lf*melt_can/C;
				Wcsn-=melt_can;
				Wcrn+=melt_can;	
				
			}else{
				T11p=T11;
			}
									
			C=land(lu,jcd)*LSAI*GTConst::c_can + GTConst::c_ice*Wcsn + GTConst::c_liq*Wcrn;
			C=(C+C0)/2.;
			
			canopy_fluxes(r, c, T11p, Tg, Ta, Qg, Qa, zmu, zmT, z0, z0s, d0, z0r, hveg, v, LR, P, SW, LW, e, LSAI, decaycoeff0, 
					    land, lu, Wcrn, Wcrnmax, Wcsn, Wcsnmax, &subl_can, Etrans, LWv, LWg, Hv, LEv, &h1, &dhdT, Ts, Qs, Qv, ruc,
					    froot, theta, soil_transp_layer, chgsgn, Lobukhov, par, n, rm, rh, rv, rc, rb, u_top, decay, Locc,
					    LWup_above_v, psi, soil, sy, &alpha, &beta, T, soil_evap_layer); 
			
			err1=fabs(C*(T11-T00)/par->Dt - SWv - A*h1 - (1.0-A)*h0 );
			
			nw/=3.0;
			cont2++;
																	
		} while(err1>err0 && cont2<5);	
		
		if(Lobukhov0*(*Lobukhov)<0) chgsgn++;		
				
		cont++;		
		
		if(fabs(T11-T10)<0.01 && err1<0.1) a=1;
								
	}while(a==0 && cont<par->maxiter_canopy);
	
	*Tv=T11p;
	*dWcrn=Wcrn-Wcrn0;
	*dWcsn=Wcsn-Wcsn0;
						
	Turbulence::turbulent_fluxes(*ruc, *ruc/beta, P, *Ts , Tg, *Qs, alpha*Qg, alpha*dQgdT, *Hg, *dHgdT, *Eg, *dEgdT);
		
	if(*Tv!=(*Tv)){
		f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
		fprintf(f,"Error:: Tv no value %ld %ld\n",r,c);
		fclose(f);
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}
}		
			
		
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
		
void canopy_fluxes(long r, long c, double Tv, double Tg, double Ta, double Qgsat, double Qa, double zmu, double zmT, double z0, 
				   double z0s, double d0, double z0r, double hveg, double v, double LR, double P, double SW, double LW, double e, 
				   double LSAI, double decaycoeff0, const GeoMatrix<double>& land, long lu, double Wcrn, double Wcrnmax, double Wcsn, double Wcsnmax,
				   double *Esubl, double *Etrans, double *LWv, double *LWg, double *H, double *LE, double *h, double *dhdT, 
				   double *Ts, double *Qs, double *Qv, double *ruc, const GeoMatrix<double>& froot, double *theta, GeoVector<double>& soil_transp_layer,
				   long chgsgn, double *Lobukhov, Par *par, long n, double *rm, double *rh, double *rv, double *rc, double *rb,
			    double *u_top, double *decay, double *Locc, double *LWup_above_v, double psi, GeoTensor<double>& soil, long sy, double *alpha, 
				   double *beta, double *T, GeoVector<double>& soil_evap_layer){


	double u_star, ft=0.0, fw, fwliq, fwice;
	double dQvdT, Hg, Lt, Lv, R, dLWvdT, dHdT, dEdT, dEsubldT, E;
	double Loc=1.E50, Loc0, Ts0;
	long cont, cont2, max_chgsgn=10;
    size_t l;
	short MO=par->monin_obukhov;
	FILE *f;
	
	//CANOPY FRACTION SET AT THE MAX OF SNOW AND LIQUID WATER FRACTION ON CANOPY
	fwliq=pow(Wcrn/Wcrnmax,2./3.);
	fwice=pow(Wcsn/Wcsnmax,2./3.);
	fw=Fmax(fwliq, fwice);
	if(fw<0) fw=0.0;
	if(fw>1) fw=1.0;
	
	//LONGWAVE
	longwave_vegetation(LW, e, Tg, Tv, LSAI, LWv, LWg, &dLWvdT, LWup_above_v);
			
	//FIND SPECIFIC HUMIDITY IN THE VEGETATION LAYER
	SpecHumidity_2(Qv, &dQvdT, 1.0, Tv, P);
	
	//UNDERCANOPY TURBULENT FLUXES
	
	//iteration for Ts
	cont2=0;
	do{
		
		cont2++;

		Ts0=(*Ts);
		
		//neglects stability corrections, if the number of iterations in Tcanopy is larger than a threshold
		if(chgsgn>max_chgsgn) MO=4;
		
		//apply Monin-Obukhov from canopy air to atmosphere
		Turbulence::aero_resistance(zmu, zmT, z0, d0, z0r, v, Ta, Ts0, Qa, *Qs, P, LR, Lobukhov, rm, rh, rv, par->state_turb, MO, par->maxiter_Businger);
		
		//friction speed
		u_star=sqrt(v/(*rm));
		
		//wind speed at the top of the canopy
		*u_top=(u_star/GTConst::ka) * Turbulence::CZ(MO, hveg, z0, d0, *Lobukhov, (&Turbulence::Psim));
						
		//iteration for Loc (within canopy Obukhov length)
		cont=0;
		do{
			
			cont++;
		
			Loc0=Loc;
			
			veg_transmittance(par->stabcorr_incanopy, v, u_star, *u_top, hveg, z0s, z0, d0, LSAI, decaycoeff0, *Lobukhov, Loc, rb, ruc, decay);		
			
			Turbulence::find_actual_evaporation_parameters(r, c, alpha, beta, soil_evap_layer, theta, soil, sy, T, psi, P, *ruc, Ta, Qa, Qgsat, n);
			
			if((*Qv)<(*Qs)){	//condensation	
				R=1.0;	
			}else{
				canopy_evapotranspiration(*rb, Tv, Qa, P, SW, theta, land, lu, soil, sy, froot, &ft, soil_transp_layer);
				R=fw+(1.0-fw)*ft;
			}			
			*rc = (*rb) / R;
	
			*Ts = (Ta/(*rh) + Tg/(*ruc) + Tv/(*rb)) / (1./(*rh) + 1./(*ruc) + 1./(*rb));
			*Qs = (Qa/(*rv) + (*alpha)*Qgsat*(*beta)/(*ruc) + (*Qv)/(*rc)) / (1./(*rv) + (*beta)/(*ruc) + 1./(*rc));
		
			Hg=air_cp((*Ts+Tg)/2.) * air_density((*Ts+Tg)/2., (*Qs+(*alpha)*Qgsat)/2., P) * (Tg-(*Ts))/(*ruc);	
		
			//			-u*^3
			// Loc = ------------------    Below Canopy Monin-Obukhov length (Niu&Yang)
			//       k(g/T)(Hg/(rho*C))
			
			Loc=-pow(u_star,3.0)/( GTConst::ka*(GTConst::GRAVITY/(*Ts+GTConst::tk))*(Hg/(air_density(*Ts,*Qs,P)*air_cp(*Ts))) );
			if(Hg==0.0 || Hg!=Hg) Loc=1.E+50;
			
		}while(fabs(Loc0-Loc)>0.01 && cont<=par->maxiter_Loc);
								
	}while(cont2<par->maxiter_Ts && fabs((*Ts)-Ts0)>0.01);
	
	//CANOPY FLUXES								
	Turbulence::turbulent_fluxes(*rb, *rc, P, *Ts, Tv, *Qs, *Qv, dQvdT, *H, dHdT, E, dEdT);	
	
	//Et from transpiration, E-Et condensation or evaporation/sublimation from water on the canopy
	Lt = Turbulence::Levap(Tv);
	if(E>0){	//evaporation or sublimation
		*Esubl=E*fw/R;
		dEsubldT=dEdT*fw/R;
		if(fwliq+fwice>0){
			Lv=Lt + GTConst::Lf*fwice/(fwliq+fwice);	//linear interpolation to decide if sublimation or condensation occurs
		}else{
			Lv=Lt;
		}
		
	}else{	//condensation
		*Esubl=E;
		dEsubldT=dEdT;
		if(Tv>=0){
			Lv=Lt;
		}else{
			Lv=Lt + GTConst::Lf;
		}
	}
	
	*Etrans=E-(*Esubl);
	for(l=1;l<soil_transp_layer.size();l++){
		soil_transp_layer[l] = soil_transp_layer[l] * (*Etrans);
	}
	
	*LE=Lt*(*Etrans) + Lv*(*Esubl);
	*h=(*LWv) - (*H) - (*LE);
	*dhdT=dLWvdT - dHdT - Lt*(dEdT-dEsubldT) - Lv*dEsubldT;
	*Locc=Loc;

	if(*h!=(*h)){
		f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
		fprintf(f,"Error:: No value in canopy fluxes Loc:%e v:%f rm:%e Ts:%f Tv:%f Ta:%f Tg:%f Hg:%f %ld %ld\n",Loc,v,*rm,*Ts,Tv,Ta,Tg,Hg,r,c);
		fclose(f);
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}
			
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void shortwave_vegetation(double Sd, double Sb, double x, double fwsn, double wsn, double Bsnd, double Bsnb, double Agd, 
						  double Agb, double C, double R, double T, double L, double *Sv, double *Sg, double *Sup_above){
	
	double phi1,phi2,G,K,w,wBd,wBb,xm;
	double b,c,d,f,h,s,u1,u2,u3,s1,s2,p1,p2,p3,p4,d1,d2,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,Iupb,Iupd,Idwb,Idwd,Ib,Id;
		
	phi1=0.5-0.633*C-0.33*C*C;
	phi2=0.877*(1.0-2.0*phi1);
	G=phi1+phi2*x;
	
	if(C==0){
		xm=1.0;
	}else{
		xm=(1.0-(phi1/phi2)*log((phi1+phi2)/phi1))/phi2;
	}
	
	K=G/x;
	
	w=(R+T)*(1.0-fwsn) + wsn*fwsn;
	
	wBd=(0.5*(R+T+pow(0.5*(1.0+C),2.0)*(R-T)))*(1.0-fwsn) + wsn*Bsnd*fwsn;
	wBb=( ((1.0+xm*K)/(xm*K))*0.5*w*( (G/(x*phi2+G))*(1.0-(x*phi1/(x*phi2+G))*log((x*phi1+x*phi2+G)/(x*phi1))) ) )*(1.0-fwsn) + 
			wsn*Bsnb*fwsn;
	
	b=1.0-w+wBd;
	c=wBd;
	d=xm*K*wBb;
	f=w*xm*K*(1.0-wBb/w);
	h=pow(b*b-c*c,0.5)/xm;
	s=pow(xm*K,2.0)+c*c-b*b;
	u1=b-c/Agd;
	u2=b-c*Agd;
	u3=f+c*Agd;
	s1=exp(-h*L);
	s2=exp(-K*L);
	p1=b+xm*h;
	p2=b-xm*h;
	p3=b+xm*K;
	p4=b-xm*K;
	d1=p1*(u1-xm*h)/s1 - p2*(u1+xm*h)*s1;
	d2=(u2+xm*h)/s1 - (u2-xm*h)*s1;
	h1=-d*p4-c*f;
	h2=((d-h1*p3/s)*(u1-xm*h)/s1 - p2*(d-c-h1*(u1+xm*K)/s)*s2)/d1;
	h3=-((d-h1*p3/s)*(u1+xm*h)*s1 - p1*(d-c-h1*(u1+xm*K)/s)*s2)/d1;
	h4=-f*p3-c*d;
	h5=-(h4*(u2+xm*h)/(s*s1) + (u3-h4*(u2-xm*K)/s)*s2)/d2;
	h6=(h4*(u2-xm*h)*s1/s + (u3-h4*(u2-xm*K)/s)*s2)/d2;
	h7=c*(u1-xm*h)/(d1*s1);
	h8=-c*(u1+xm*h)*s1/d1;
	h9=(u2+xm*h)/(d2*s1);
	h10=-s1*(u2-xm*h)/d2;
	
	Iupb=h1/s+h2+h3;
	Iupd=h7+h8;
	Idwb=h4*exp(-K*L)/s+h5*s1+h6/s1;
	Idwd=h9*s1+h10/s1;
	
	Ib=(1.0-Iupb) - (1.0-Agd)*Idwb - (1.0-Agb)*exp(-K*L);
	Id=(1.0-Iupd) - (1.0-Agd)*Idwd;

	if(x>0){
		*Sv=Sb*Ib + Sd*Id;
		*Sg=Sb*(1.0-Agb)*exp(-K*L) + (Sb*Idwb+Sd*Idwd)*(1.0-Agd);
		*Sup_above=Sb*Iupb + Sd*Iupd;
	}else{
		*Sv=Sd*Id;
		*Sg=Sd*Idwd*(1.0-Agd);	
		*Sup_above=Sd*Iupd;
	}
		
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void longwave_vegetation(double Lin, double eg, double Tg, double Tv, double L, double *Lv, double *Lg, double *dLv_dTv, double *Lup_above){

	double ev, Lvdw;
	
	ev=1.0-exp(-L);
	
	Lvdw=(1.0-ev)*Lin + ev*SB(Tv);
	
	*Lv=ev*(1.0+(1.0-eg)*(1.0-ev))*Lin + ev*eg*SB(Tg) - (2.0-ev*(1.0-eg))*ev*SB(Tv);
	*Lg=eg*Lvdw - eg*SB(Tg);
	*dLv_dTv=-(2.0-ev*(1.0-eg))*ev*dSB_dT(Tv);
	
	*Lup_above=(1.0-ev)*(1.0-eg)*(1.0-ev)*Lin + (1.0-ev)*eg*SB(Tg) + (1.0+(1.0-ev)*(1.0-eg))*ev*SB(Tv);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void canopy_rain_interception(double rain_max_loading, double LSAI, double Prain, double *max_storage, double *storage, double *drip){
		
	double load;
	double coeff=0.25; /*from The Partitioning of Evapotranspiration into Transpiration, Soil Evaporation, and Canopy Evaporation in a GCM:
						Impacts on Land–Atmosphere Interaction,DAVID M. LAWRENCE, PETER E. THORNTON, KEITH W. OLESON, AND GORDON B. BONAN,
						JOURNAL OF HYDROMETEOROLOGY, DOI: 10.1175/JHM596.1*/
	
	*max_storage = rain_max_loading*LSAI; //mm
	
	load = coeff*Prain*(1.0-exp(-0.5*LSAI));
	*storage = (*storage) + load;
	*drip = Prain - load;
	
	if( (*storage) > (*max_storage) ){
		*drip = *drip + (*storage) - (*max_storage);
		*storage = (*max_storage);
	}
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void canopy_snow_interception(double snow_max_loading, double LSAI, double Psnow, double Tc, double v, double Dt, 
							  double *max_storage, double *storage, double *drip){
	
	double load, unload;
	double CT=1.87E5, CV=1.56E5;
	
	*max_storage = snow_max_loading*LSAI;
	
	load = ( (*max_storage) - (*storage) ) * (1. - exp(-Psnow/(*max_storage)));	//Niu & Yang, 2004
	*storage = (*storage) + load;
	*drip = Psnow - load;
	
	if((*storage)>(*max_storage)){
		*drip = *drip - ( (*max_storage) - (*storage) );
		*storage = (*max_storage);
	}
	
	unload = Fmin(*storage, (*storage)*(Fmax(0.0, Tc+3.0)/CT + v/CV)*Dt);
	if(unload<0.1) unload=0.0;	//prevents very low snowfalls
	
	*drip = *drip + unload;
	
	*storage = (*storage) - unload;	

}
	

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void update_roughness_veg(double hc, double snowD, double zmu, double zmt, double *z0_ris, double *d0_ris, double *hc_ris){
	
	FILE *f;
	
	*hc_ris=hc-snowD;//[mm]
	*d0_ris=0.667*(*hc_ris);//[mm]
	*z0_ris=0.1*(*hc_ris);//[mm]
	
	*d0_ris=(*d0_ris)*1.E-3;//[m]
	*z0_ris=(*z0_ris)*1.E-3;//[m]
	*hc_ris=(*hc_ris)*1.E-3;//[m]
	
	if(zmu<(*hc_ris)){
		f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
		fprintf(f,"Wind speed measurement height:%f m\n",zmu);
		fprintf(f,"Effective vegetation height:%f m\n",*hc_ris);
		fprintf(f,"Wind Speed must be measured above vegetation");
		fclose(f);
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}

	if(zmt<(*hc_ris)){
		f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
		fprintf(f,"Temperature measurement height:%f m\n",zmt);
		fprintf(f,"Effective vegetation height:%f m\n",*hc_ris);
		fprintf(f,"Temperatute must be measured above vegetation");
		fclose(f);
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}
	
	if(*hc_ris-(*d0_ris)<(*z0_ris)){
		f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
		fprintf(f,"Effective vegetation height:%f m\n",*hc_ris);
		fprintf(f,"Effective 0 displacement height:%f m\n",*d0_ris);
		fprintf(f,"Effective roughness length:%f m\n",*z0_ris);
		fprintf(f,"It must be always Hcanopy-d0 >= z0");
		fclose(f);
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}

}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void root(long n, double d, double slope, GeoTensor<double>& D, GeoMatrix<double>& root_fraction, long row)
{
	//n = number of soil layers (from the surface) affected by root absorption
	//d = root depth (vertical) [mm]
	//slope = max lateral slope [rad]
	//D[] = soil layer thickness [mm]
	//root_fraction[] = weighting factor assigned to each layer for transpiration, sum of all root_fraction components gives 1
	
	long l;
	double z=0.0;
	double d_corr=d*cos(slope); //slope depth taken as ortogonal to layer boundary
		
	for(l=1;l<n;l++){
		z += D(1,jdz,l);
		if( d_corr > z ){
			root_fraction(row,l) = D(1,jdz,l)/d_corr;
		}else{
			if( d_corr > z-D(1,jdz,l)){
				root_fraction(row,l) = ( d_corr - (z-D(1,jdz,l)) ) / d_corr;
			}else{
				root_fraction(row, l) = 0.0;
			}
		}
	}
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void canopy_evapotranspiration(double rbv, double Tv, double Qa, double Pa, double SWin, double *theta, const GeoMatrix<double>& land, long lu,
						 GeoTensor<double>& soil, long sy, const GeoMatrix<double>& root, double *f, GeoVector<double>& fl){

	double fS, fe, fTemp, Rsmin, ea, ev;
	size_t l;
	
	//CANOPY TRANSPIRATION (parameters from Best (1998))
	//solar radiation [Best, (1998); Dolman et al., 1991]
	
	fS=SWin/(SWin+250.0)*1.25;
	
	//pressure deficit [Best, (1998); Dickinson et al., 1991]
	ea = VapPressurefromSpecHumidity(Qa, Pa);
	ev = SatVapPressure(Tv, Pa);
	fe=1.0-(ev-ea)/40.0;

	//temperature [Best, (1998); Dickinson et al., 1991]
	if(Tv<=0){
		fTemp=1E-12;
	}else if(Tv>=50.0){
		fTemp=1E-12;
	}else{
		fTemp=(Tv-0.0)*(50.0-Tv)/625.0;
	}
	
	*f=0.0;
	for(l=1;l<fl.size();l++){
		//water content [Wigmosta et al., (1994); Feddes et al.(1978)]
		if (theta[l] >= soil[sy][jfc][l]){
			fl[l] = 1.0;
		}else if(theta[l] > soil[sy][jwp][l]){
			fl[l] = (theta[l]-soil[sy][jwp][l])/(soil[sy][jfc][l]-soil[sy][jwp][l]);
		}else{
			fl[l] = 0.0;
		}
						
		//stomata resistance for each layer
		if(fS*fe*fTemp*fl[l]<6.0E-11){
			fl[l]=1.0E12;
		}else{
			Rsmin=land(lu,jrs);
			fl[l]=Rsmin/(fS*fe*fTemp*fl[l]);
		}
			
		//transpiration fraction for each layer
		fl[l]=root(lu,l)*(rbv/(rbv+fl[l]));
		//transpiration for all the column (as a fraction of Epc)
		*f+=fl[l];
	}
	
		for(l=1;l<fl.size();l++){
		   if(*f!=0) fl[l]/=(*f);
	}

}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void veg_transmittance(short stabcorr_incanopy, double v, double u_star, double u_top, double Hveg, double z0soil, double z0veg, double d0veg, 
					   double LSAI, double decaycoeff0, double Lo, double Loc, double *rb, double *rh, double *decay){

	double Lc = 0.4;			//characteristic dimension of vegetation [m]
	double zm = d0veg + z0veg;	//height above surface at which the canopy energy/momentum sink/source are supposed to occur
	double u_veg;				//wind speed at zm
	double phi_above, phi_below;//correction to neutral stability above and below the canopy
	double r;					//component of undercanopy resistance
	double Ktop;				//eddy diffusivity at the top of the canopy
	//double Cs, Cs_bare, Cs_dense, W;

	//stability
	//over canopy
	if(Lo<0){
		phi_above = pow(1.0-16.0*Hveg/Lo,-0.5);
	}else{
		phi_above = 1.0+5.0*Hveg/Lo;
	}
	//under canopy
	if(Loc<0){
		phi_below = pow(1.0-15.0*zm/Loc,-0.25);
	}else{
		phi_below = 1.0+4.7*zm/Loc;
	}
	
	//adjust decay coefficient in accordance with stability under canopy
	if(stabcorr_incanopy == 1){
		*decay = Fmin(1.E5, decaycoeff0*sqrt(phi_below));
	}else{
		*decay = decaycoeff0;
	}
	
	//wind speed at the canopy characteristic height (zm)
	//after Zeng 
	//u_veg = u_star;
	//after Huntingford
	u_veg = Fmax(0.001, u_top*exp((*decay)*(zm/Hveg-1.0)));
	
	//canopy resistance (see Huntingsford)
	*rb = Fmin(1.E20, 70.0*sqrt(Lc/u_veg))/LSAI;
	
	//ground resistance
	//Zeng
	//Cs_dense = 0.004;
	//Cs_bare = (pow(z0soil*u_star/1.5E-5,-0.45)*ka/0.13);
	//W = 1.0-exp(-LSAI);
	//Cs = W*Cs_dense + (1.0-W)*Cs_bare;
	//*rh = 1.0/( Cs * u_star);
	
	//Huntingford (with a term from Zeng)
	r = (Hveg*exp(*decay)/(d0veg*(*decay))) * (exp(-(*decay)*z0soil/Hveg)-exp(-(*decay)*zm/Hveg));
	Ktop = GTConst::ka*u_star*(Hveg-d0veg)/phi_above;
	*rh = Fmin(1.E20, r*d0veg/Ktop + 2.0*pow(r, 0.45)/(GTConst::ka*u_star));	//2.0 comes from ln(z0/z0h)
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
