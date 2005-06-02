#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>
#include <Glasses/ZImage.h>
#include <Glasses/PRSBase.h>



struct Helix {
  // constant
  Float_t fA;       // contains charge and magnetic field data
  PRSBase * fRnrMod;  // RnrMod data 

  //parameters dependend pT and pZ size, set in init function
  Float_t fLam;        // momentum ratio pT/pZ
  Float_t fR;          // a/pT
  Float_t fPhiStep;    // step size in xy projection, dependent of RnrMode and momentum
    
  Int_t   fN;           // step number in helix;
  Int_t   NMax;      
  Float_t x_off, y_off;
  Float_t sin, cos;
  Float_t stex;        // texture step
  Bool_t  crosR;
  
  Helix(Float_t a, PRSBase* rs) :
    fA(a) {fRnrMod = rs;}

  void init(Float_t pT, Float_t pZ){
    fN=0;
    crosR = false;
    x_off = 0; y_off = 0;
    fLam = pZ/pT;
    fR =  pT/fA;

    fPhiStep = fRnrMod->mMinAng *TMath::Pi()/180;
    if(fRnrMod->mDelta < TMath::Abs(fR)){
      Float_t ang  = 2*TMath::ACos(1-fRnrMod->mDelta/TMath::Abs(fR));
      if (ang < fPhiStep) fPhiStep = ang; 
    }
    if(fA<0) fPhiStep = -fPhiStep;
    sin = TMath::Sin(fPhiStep); cos = TMath::Cos(fPhiStep);
    stex = TMath::Abs(fR*fPhiStep)*TMath::Sqrt(1+fLam*fLam)/fRnrMod->mTexFactor;
  }

  void set_bounds(Float_t x, Float_t y, Float_t z){
    // check steps for max orbits
    NMax =Int_t(fRnrMod->mMaxOrbs*2*TMath::Pi()/TMath::Abs(fPhiStep));
    // check steps for Z boundaries
    Float_t nz;
    if(fLam > 0){
      nz = (fRnrMod->mMaxZ - z)/fLam*TMath::Abs(fR*fPhiStep);
    }
    else {
      nz = (-fRnrMod->mMaxZ - z)/fLam*TMath::Abs(fR*fPhiStep);
    }
    if (nz < NMax) NMax = Int_t(nz);
    

    
    // check steps if circles intesetct
    if(TMath::Sqrt(x*x+y*y) < fRnrMod->mMaxR +TMath::Abs(fR)){
      crosR = true;
    }
  }


  void step(Float_t &vx, Float_t &vy, Float_t &vz, 
	    Float_t &px, Float_t &py, Float_t &pz) 
  {
    fN++;
    vx += (px*sin - py*(1 - cos))/fA + x_off;
    vy += (py*sin + px*(1 - cos))/fA + y_off;
    vz += fLam*TMath::Abs(fR*fPhiStep);
    glTexCoord2f(fN*stex,fRnrMod->mTexVCoor);
    glVertex3f(vx, vy, vz);
    Float_t px_t = px*cos - py*sin ;
    Float_t py_t = py*cos + px*sin ;
    px = px_t;
    py = py_t;
  }


  Bool_t loop_from_to(Float_t &vx, Float_t &vy, Float_t &vz, 
		      Float_t &px, Float_t &py, Float_t &pz, 
		      Float_t ex, Float_t ey, Float_t ez)
  {
        
    if ( TMath::Abs(ez) > fRnrMod->mMaxZ){
      loop_to_bounds(vx, vy, vz, px, py, pz);
      return false;
    }


    Float_t p0x = px, p0y = py;
    Float_t zs = fLam*TMath::Abs(fR*fPhiStep);
    Float_t fnsteps = (ez - vz)/zs;
    Int_t   nsteps  = Int_t((ez - vz)/zs);
    Float_t sinf = TMath::Sin(fnsteps*fPhiStep);
    Float_t cosf = TMath::Cos(fnsteps*fPhiStep);
    Int_t os_max =Int_t(fRnrMod->mMaxOrbs*2*TMath::Pi()/TMath::Abs(fPhiStep));

    {
      glBegin(GL_LINE_STRIP);
      glTexCoord2f(0.0, fRnrMod->mTexVCoor);
      glVertex3f(vx,vy,vz);

      if(nsteps > 0){
	Float_t xf  = vx + (px*sinf - py*(1 - cosf))/fA;  
	Float_t yf =  vy + (py*sinf + px*(1 - cosf))/fA;
	x_off =  (ex - xf)/fnsteps;
	y_off =  (ey - yf)/fnsteps;
	for (Int_t l=0; l<nsteps; l++) {
	  if ( vx*vx+vy*vy > fRnrMod->mMaxR*fRnrMod->mMaxR){
	    glEnd(); return false;
	  }
          if(fN > os_max ){
	    glEnd();
	    return false;
	  }
	  step(vx,vy,vz,px,py,pz); 
	}
	 
      }
      Float_t fu = TMath::Sqrt((vx-ex)*(vx-ex)+(vy-ey)*(vy-ey) +(vz-ez)*(vz-ez));
      glTexCoord2f(fN*stex + fu/fRnrMod->mTexFactor, fRnrMod->mTexVCoor);
      vx = ex; vy = ey; vz = ez; 
      glVertex3f(vx,vy,vz);
      glEnd();
      
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
    
  Bool_t loop_to_bounds(Float_t &vx, Float_t &vy, Float_t &vz, 
			Float_t &px, Float_t &py, Float_t &pz)
  {
    set_bounds(vx,vy,vz);
    if(NMax > 0){
      glBegin(GL_LINE_STRIP);
      glTexCoord2f(0.0, fRnrMod->mTexVCoor);
      glVertex3f(vx, vy, vz);
      Float_t tx,ty;
      while(fN < NMax){
	tx = vx + (px*sin - py*(1 - cos))/fA + x_off;
	ty = vy + (py*sin + px*(1 - cos))/fA + y_off;
	if ( tx*tx+ty*ty > fRnrMod->mMaxR*fRnrMod->mMaxR){
	  glEnd(); return false;
	}
	if(TMath::Abs(vz + fLam*TMath::Abs(fR*fPhiStep)) >  fRnrMod->mMaxZ){
	  glEnd(); return false;
	}
	step(vx,vy,vz,px,py,pz);
      }
      glEnd();
      return true;
    }
    return false;
  }

};

/**************************************************************************/
//  LINE
/**************************************************************************/




struct Line {
  PRSBase * fRnrMod;  // RnrMod data 
  void draw_to(Float_t &x0, Float_t &y0, Float_t &z0, Float_t x1, Float_t y1, Float_t z1)
  {
    glBegin(GL_LINES);
    glTexCoord2f(0.,fRnrMod->mTexVCoor);
    glVertex3f(x0, y0, z0);
    Float_t u = TMath::Sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1))/fRnrMod->mTexFactor;

    x0=x1; y0=y1; z0=z1;

    glTexCoord2f(u,fRnrMod->mTexVCoor);
    glVertex3f(x0, y0, z0);
    glEnd();	
  }
  
  Bool_t in_bounds(Float_t ex, Float_t ey, Float_t ez){
    if ((TMath::Abs(ez) > fRnrMod->mMaxZ) || (ex*ex+ey*ey > fRnrMod->mMaxR*fRnrMod->mMaxR))
      return false;
    else
      return true;
  }

  Float_t get_bounds_time(Float_t vx, Float_t vy, Float_t vz, Float_t px, Float_t py, Float_t pz)
  {
    Float_t tZ,Tb = 0;
    // time where particle intersect +/- mMaxZ
    if (pz > 0) {
      tZ = (fRnrMod->mMaxZ - vz)/pz;
    }
    else  if (pz < 0 ) {
      tZ = (-1)*(fRnrMod->mMaxZ + vz)/pz;
    }
    // time where particle intersects cylinder
    Float_t tR=0;
    Double_t a = px*px + py*py;
    Double_t b = 2*(vx*px + vy*py);
    Double_t c = vx*vx + vy*vy - fRnrMod->mMaxR*fRnrMod->mMaxR;
    Double_t D = b*b - 4*a*c;
    if(D >= 0) {
      Double_t D_sqrt=TMath::Sqrt(D);
      tR = ( -b - D_sqrt )/(2*a);
      if( tR < 0) {
	tR = ( -b + D_sqrt )/(2*a);
      }

      // compare the two times
      Tb = tR < tZ ? tR : tZ;
      return Tb;
    }
    return tZ;
  }
}; // struct Line

