#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>
#include <Glasses/ZImage.h>
#include <Glasses/MCTrackRnrStyle.h>


struct MCVertex {
  Float_t x,y,z,t;
};


struct MCStruct
{
  typedef void (MCStruct::*RegVertex)(); 

  MCTrackRnrStyle*  fRnrMod;  
  vector<MCVertex>* track_points;
  MCVertex          v;
  Float_t           fVelocity; // size of particle velocity
  
  MCStruct(MCTrackRnrStyle* rs, MCVertex* v0 , Float_t vel, vector<MCVertex>* tpv) 
  { 
    fRnrMod = rs;
    v = *v0;
    fVelocity = vel;
    track_points = tpv;
    track_points->push_back(v);
  }
};

struct MCHelix : public MCStruct {
  // constant
  Float_t fA;       // contains charge and magnetic field data

  //parameters dependend pT and pZ size, set in init function
  Float_t fLam;        // momentum ratio pT/pZ
  Float_t fR;          // a/pT
  Float_t fPhiStep;    // step size in xy projection, dependent of RnrMode and momentum
  Float_t fTimeStep;   
 
  Int_t   fN;           // step number in helix;
  Int_t   NMax;         // max number of points in helix
  Float_t x_off, y_off; // offset for fitting daughters
  Float_t sin, cos;
  Bool_t  crosR;

  MCHelix(MCTrackRnrStyle* rs, MCVertex* v0, Float_t vel, vector<MCVertex>* tpv, Float_t a):
    MCStruct(rs, v0 , vel, tpv){
      fA = a;
    };

  void init(Float_t pT, Float_t pZ)
  {
    fN=0;
    crosR = false;
    x_off = 0; y_off = 0;
    fLam = pZ/pT;
    fR =  pT/fA;

    fPhiStep = fRnrMod->mMinAng *TMath::Pi()/180;
    if(fRnrMod->mDelta < TMath::Abs(fR)){
      Float_t ang  = 2*TMath::ACos(1 - fRnrMod->mDelta/TMath::Abs(fR));
      if (ang < fPhiStep) fPhiStep = ang; 
    }
    if(fA<0) fPhiStep = -fPhiStep;
    fTimeStep = TMath::Abs(fR*fPhiStep)*TMath::Sqrt(1+fLam*fLam)/fVelocity;
    fTimeStep *= 0.01; //cm->m

    sin = TMath::Sin(fPhiStep); 
    cos = TMath::Cos(fPhiStep);
    // printf("MCHelix::init time step %e velocty %f \n", fTimeStep,fVelocity);
  }

  void set_bounds()
  {
    // check steps for max orbits
    NMax = Int_t(fRnrMod->mMaxOrbs*TMath::TwoPi()/TMath::Abs(fPhiStep));
    // check steps for Z boundaries
    Float_t nz;
    if(fLam > 0) {
      nz = (fRnrMod->mMaxZ - v.z)/(fLam*TMath::Abs(fR*fPhiStep));
    } else {
      nz = (-fRnrMod->mMaxZ - v.z)/(fLam*TMath::Abs(fR*fPhiStep));
    }
    if (nz < NMax) NMax = Int_t(nz);
    
    // check steps if circles intersect
    if(TMath::Sqrt(v.x*v.x+v.y*v.y) < fRnrMod->mMaxR + TMath::Abs(fR)) {
      crosR = true;
    }
  }


  void step(Float_t &px, Float_t &py, Float_t &pz) 
  {
    v.t += fTimeStep;
    v.x += (px*sin - py*(1 - cos))/fA + x_off;
    v.y += (py*sin + px*(1 - cos))/fA + y_off;
    v.z += fLam*TMath::Abs(fR*fPhiStep);
    track_points->push_back(v);
    Float_t px_t = px*cos - py*sin ;
    Float_t py_t = py*cos + px*sin ;
    px = px_t;
    py = py_t;
    fN++;
  }


  Bool_t loop_to_vertex(Float_t &px, Float_t &py, Float_t &pz, 
			Float_t ex, Float_t ey, Float_t ez)
  {
        
    if ((TMath::Abs(ez) > fRnrMod->mMaxZ) || (ex*ex+ey*ey > fRnrMod->mMaxR*fRnrMod->mMaxR)) {
      loop_to_bounds(px, py, pz);
      return false;
    }


    Float_t p0x = px, p0y = py;
    Float_t zs = fLam*TMath::Abs(fR*fPhiStep);
    Float_t fnsteps = (ez - v.z)/zs;
    Int_t   nsteps  = Int_t((ez - v.z)/zs);
    Float_t sinf = TMath::Sin(fnsteps*fPhiStep);
    Float_t cosf = TMath::Cos(fnsteps*fPhiStep);

    { 

      track_points->push_back(v);

     
      if(nsteps > 0){
	Float_t xf  = v.x + (px*sinf - py*(1 - cosf))/fA;  
	Float_t yf =  v.y + (py*sinf + px*(1 - cosf))/fA;
	x_off =  (ex - xf)/fnsteps;
	y_off =  (ey - yf)/fnsteps;
        Float_t xforw, yforw, zforw;
	for (Int_t l=0; l<nsteps; l++) {
          xforw  = v.x + (px*sin - py*(1 - cos))/fA + x_off;
	  yforw =  v.y + (py*sin + px*(1 - cos))/fA + y_off;
          zforw =  v.z + fLam*TMath::Abs(fR*fPhiStep);
	  if ((xforw*xforw+yforw*yforw > fRnrMod->mMaxR*fRnrMod->mMaxR) ||(TMath::Abs(zforw) > fRnrMod->mMaxZ) ) {
	    return false;
	  }
	  step(px,py,pz); 
	}
	 
      }
      // set time to the end point
      v.t += TMath::Sqrt((v.x-ex)*(v.x-ex)+(v.y-ey)*(v.y-ey) +(v.z-ez)*(v.z-ez))/fVelocity;
      v.x = ex; v.y = ey; v.z = ez;      
      track_points->push_back(v);
    }
      
    { // fix momentum in the remaining part
      Float_t cosr =  TMath::Cos((fnsteps-nsteps)*fPhiStep); 
      Float_t sinr =  TMath::Sin((fnsteps-nsteps)*fPhiStep); 
      Float_t px_t = px*cosr - py*sinr ;
      Float_t py_t = py*cosr + px*sinr ;
      px = px_t;
      py = py_t;
    }
    { // calculate direction of faked px,py
      Float_t pxf = (p0x*cosf - p0y*sinf)/TMath::Abs(fA) + x_off/fPhiStep;
      Float_t pyf = (p0y*cosf + p0x*sinf)/TMath::Abs(fA) + y_off/fPhiStep;
      Float_t fac = TMath::Sqrt(p0x*p0x + p0y*p0y)/TMath::Sqrt(pxf*pxf + pyf*pyf);
      px = fac*pxf;
      py = fac*pyf;
    }
    return true;
  }
    
  Bool_t loop_to_bounds(Float_t &px, Float_t &py, Float_t &pz)
  {
    set_bounds();
    if(NMax > 0){
      track_points->push_back(v);
      Float_t xforw,yforw,zforw;
      while(fN < NMax){
	xforw = v.x + (px*sin - py*(1 - cos))/fA + x_off;
	yforw = v.y + (py*sin + px*(1 - cos))/fA + y_off;
	zforw =  v.z + fLam*TMath::Abs(fR*fPhiStep);
	
	if ((crosR && (xforw*xforw+yforw*yforw > fRnrMod->mMaxR*fRnrMod->mMaxR)) ||(TMath::Abs(zforw) > fRnrMod->mMaxZ)) {
	  return false;
	}

	step(px,py,pz);
      }
      return true;
    }
    return false;
  }
};

/**************************************************************************/
//  LINE
/**************************************************************************/

struct MCLine : public MCStruct
{
  MCLine(MCTrackRnrStyle* rs, MCVertex* v0 ,Float_t vel, vector<MCVertex>* tpv):
    MCStruct(rs, v0 , vel, tpv){};

  Bool_t in_bounds(Float_t ex, Float_t ey, Float_t ez)
  {
    if(TMath::Abs(ez) > fRnrMod->mMaxZ ||
       ex*ex + ey*ey  > fRnrMod->mMaxR*fRnrMod->mMaxR)
      return false;
    else
      return true;
  }

  void goto_vertex(Float_t  x1, Float_t  y1, Float_t  z1)
  {
    track_points->push_back(v);
    v.t += TMath::Sqrt((v.x-x1)*(v.x-x1)+(v.y-y1)*(v.y-y1)+(v.z-z1)*(v.z-z1))/fVelocity;
    v.x=x1; v.y=y1; v.z=z1;
    track_points->push_back(v);
  }
  

  void goto_bounds( Float_t px, Float_t py, Float_t pz)
  {
    Float_t tZ,Tb = 0;
    // time where particle intersect +/- mMaxZ
    if (pz > 0) {
      tZ = (fRnrMod->mMaxZ - v.z)/pz;
    }
    else  if (pz < 0 ) {
      tZ = (-1)*(fRnrMod->mMaxZ + v.z)/pz;
    }
    // time where particle intersects cylinder
    Float_t tR=0;
    Double_t a = px*px + py*py;
    Double_t b = 2*(v.x*px + v.y*py);
    Double_t c = v.x*v.x + v.y*v.y - fRnrMod->mMaxR*fRnrMod->mMaxR;
    Double_t D = b*b - 4*a*c;
    if(D >= 0) {
      Double_t D_sqrt=TMath::Sqrt(D);
      tR = ( -b - D_sqrt )/(2*a);
      if( tR < 0) {
	tR = ( -b + D_sqrt )/(2*a);
      }

      // compare the two times
      Tb = tR < tZ ? tR : tZ;
    } else {
      Tb = tZ;
    }

    goto_vertex(v.x+px*Tb, v.y+py*Tb, v.z+ pz*Tb);
  }
}; // struct Line

