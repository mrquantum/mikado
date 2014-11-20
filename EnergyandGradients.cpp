#include "random.h"
#include <algorithm>
#include "makemikadonetwork.h"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/LU>
#include <vector>
#include "EnergyandGradients.h"
#include<iostream>
using namespace Eigen;
using namespace std;
const double pi=4.0*atan(1.0);
   
int SIGN(double a,double b)
{
    double c=a*b;
    if(c>=0) {
        return 1;
    } else{
        return -1;    
    }
}

int sgn(double x)
{
    if(x>=0) {
        return 1;
    } else{
        return -1;}  
}

double ROSENBROCK(const Eigen::VectorXd &XY)
{
 double f=pow((1-XY(0)),2)+pow((XY(1)-XY(0)*XY(0)),2);
 return f;
}

VectorXd GRAD_rosen(const Eigen::VectorXd &XY)
{
 VectorXd GRAD(2);
 GRAD<<-2*(1-XY(0))-4*XY(0)*(XY(1)-XY(0)*XY(0)),2*(XY(1)-XY(0)*XY(0));
 return GRAD;
}

double dROSENdA(const Eigen::VectorXd &XY,const Eigen::VectorXd &s)
{
 double dda=s.dot(GRAD_rosen(XY));
 return dda;
    
}


double distance(const Eigen::VectorXd &XY,int one, int two)
{
  double l;
  int num=XY.size()/2;
  l=sqrt(pow(XY(two)-XY(one),2)+pow(XY(two+num)-XY(one+num),2));
  return l;
}
  
double dldxi(const Eigen::VectorXd &XY,int one, int two)
{
   double deri;
   deri=-1*(1/distance(XY,one,two))*(XY(two)-XY(one));
return deri;
}  
  
double dldyi(const Eigen::VectorXd &XY,int one,int two)
{
  double deri;
  int num=XY.size()/2;
  deri=-1*(1/distance(XY,one,two))*(XY(two+num)-XY(one+num));
return deri;
}

VectorXd gradL(const double x1,const double y1,const double x2, const double y2, 
               const int springnr, const vector<spring> &springlist,const int num)
{
 VectorXd grad(2*num);
 for(int i=0;i<grad.size();i++){
     grad(i)=0;
 }
 
 int coordNRone=springlist[springnr].one;
 int coordNRtwo=springlist[springnr].two;
 
    grad(coordNRone)=grad(coordNRone)+(1/distance1(x1,y1,x2,y2))*(-(x2-x1));
    grad(coordNRtwo)=grad(coordNRtwo)+(1/distance1(x1,y1,x2,y2))*(x2-x1);
    grad(coordNRone+num)=grad(coordNRone+num)+(1/distance1(x1,y1,x2,y2))*(-(y2-y1));
    grad(coordNRtwo+num)=grad(coordNRtwo+num)+(1/distance1(x1,y1,x2,y2))*(y2-y1);
return grad;
}  
    
double Energynetwork(const vector<spring> &springlist, const VectorXd &XY)
{
  VectorXd X(XY.size()/2);
  VectorXd Y(XY.size()/2);
  X=XY.head(XY.size()/2);
  Y=XY.tail(XY.size()/2);
  double Energy=0;
    for(int i=0;i<springlist.size();i++){
      Energy=Energy+
      0.5*springlist[i].k*pow(sqrt(pow(X(springlist[i].one)-(X(springlist[i].two)+springlist[i].wlr),2)
      +pow(Y(springlist[i].one)-(Y(springlist[i].two)+springlist[i].wud),2))-springlist[i].rlen,2);
    }

  return Energy;  
}

double distance1(const double x1, const double y1, const double x2,const double y2)
{
 double dist=sqrt(pow((x2-x1),2)+pow((y2-y1),2));
 return dist;
}


double Ebend(const vector<vector<int>> &springpairs,
             const vector<spring> &springlist,
             const VectorXd &XY,
            const double kappa)  
{
 double Energy=0;
 int num=XY.size()/2;
 
 for(int i=0;i<springpairs.size();i++){
 int springone=springpairs[i][0];
    int springtwo=springpairs[i][1];
 
    int coordNRone=springlist[springone].one;
    int coordNRtwo=springlist[springone].two;
    int coordNRthree=springlist[springtwo].two;
 
    double x1=XY(coordNRone);
    double y1=XY(coordNRone+num);
 
    double x21=XY(coordNRtwo)+springlist[springone].wlr; //version of (x2,y2) that lies in on spring 1, so possibly outside of the box
    double y21=XY(coordNRtwo+num)+springlist[springone].wud;
 
    double x23=XY(coordNRtwo);                 //version of (x2,y2) that is on spring 2, so MUST be inside the box
    double y23=XY(coordNRtwo+num);
 
    double x3=XY(coordNRthree)+springlist[springtwo].wlr;
    double y3=XY(coordNRthree+num)+springlist[springtwo].wud;

    Vector2d v1,v2;
    v1<<(x21-x1),(y21-y1);
    v2<<(x3-x23),(y3-y23);
    double dotv1v2=v1.dot(v2);
    double lenv1v2=sqrt(v1.dot(v1))*sqrt(v2.dot(v2));
    double c=dotv1v2/lenv1v2;
    
    if(c<-1) c=-1;
    if(c>1) c=1;
    
    double dE=(kappa/(distance1(x1,y1,x21,y21)+distance1(x23,y23,x3,y3)))*pow(acos(c),2);
    Energy=Energy+dE;    
}

return Energy; 
}


VectorXd Gradient(const vector<spring> &springlist,const VectorXd &XY)
{
  VectorXd gradE(XY.size());
  for(int i=0;i<gradE.size();i++){
    gradE(i)=0;
  }
  VectorXd X(XY.size()/2);
  VectorXd Y(XY.size()/2);
  X=XY.head(XY.size()/2);
  Y=XY.tail(XY.size()/2);
  
for(int i=0;i<springlist.size();i++){
    
    int one=springlist[i].one;
    int two=springlist[i].two;
    int num=XY.size()/2;
 
    gradE(one)=gradE(one)+
        springlist[i].k*(sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2))-springlist[i].rlen)*
        (X(one)-(X(two)+springlist[i].wlr))/
        sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2));
 
    gradE(two)=gradE(two)-
        springlist[i].k*(sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2))-springlist[i].rlen)*
        (X(one)-(X(two)+springlist[i].wlr))/
        sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2));

    gradE(one+num)=gradE(one+num)+
        springlist[i].k*(sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2))-springlist[i].rlen)*
        (Y(one)-(Y(two)+springlist[i].wud))/
        sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2));
 
    gradE(two+num)=gradE(two+num)-
        springlist[i].k*(sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2))-springlist[i].rlen)*
        (Y(one)-(Y(two)+springlist[i].wud))/
        sqrt(pow(X(one)-(X(two)+springlist[i].wlr),2)
        +pow(Y(one)-(Y(two)+springlist[i].wud),2));
}
return gradE;  
}  


VectorXd gradEbend(const vector<vector<int>> &springpairs,const vector<spring> &springlist,const VectorXd &XY,double kappa)
{
  VectorXd grad(XY.size()); //Total gradient
  VectorXd firstpart(grad.size());  //(pi-acos(th))^2 *grad(1/(L1+L2))
  VectorXd secondpart(grad.size()); //1/(L1+L2) grad(pi-acos(th))^2
  VectorXd gradL1L2m1(grad.size()); //grad( 1/(L1L2))
  VectorXd GRADc(grad.size()); //grad cos th in terms of v1.v2/v1v2
  VectorXd GRADnumerator(grad.size()); //grad v1.v2
  VectorXd GRADdenumerator(grad.size());//grad v1v2 , just the lengths
  
  
  
  //Initiate the gradient with zero's
  for(int i=0;i<grad.size();i++){
      grad(i)=0;
      firstpart(i)=0;
      secondpart(i)=0;
  }  

  int num=grad.size()/2;
  double numerator,denumerator;
  double c,s; //c=cos(theta)
 
 
 //add the first term of the gradient arccos(...)^2*grad(1/l12+l23)
  for(int i=0;i<springpairs.size();i++)
  {
    int springone=springpairs[i][0];
    int springtwo=springpairs[i][1];
    int coordNRone=springlist[springone].one;
    int coordNRtwo=springlist[springone].two;
    int coordNRthree=springlist[springtwo].two;
   
   double x1=XY(coordNRone);
   double y1=XY(coordNRone+num);
   double x21=XY(coordNRtwo)+springlist[springone].wlr;
   double y21=XY(coordNRtwo+num)+springlist[springone].wud;

   double x23=XY(coordNRtwo);
   double y23=XY(coordNRtwo+num);

   
   double x3=XY(coordNRthree)+springlist[springtwo].wlr;
   double y3=XY(coordNRthree+num)+springlist[springtwo].wud;
   
for(int j=0;j<gradL1L2m1.size();j++){
       gradL1L2m1(j)=0;
   }
   
   Vector2d v1,v2;
   v1<<(x1-x21),(y1-y21); 
   v2<<(x3-x23),(y3-y23);
   
   //When shearing the box we are going to change this to customized functions:
   numerator=v1.dot(v2);
   denumerator=sqrt(v1.dot(v1))*sqrt(v2.dot(v2));
  
   c=numerator/denumerator;
   if(c>1) c=1;
   if(c<-1) c=-1;
      
   double d12=distance1(x1,y1,x21,y21);
   double d23=distance1(x3,y3,x23,y23);
   
   gradL1L2m1(coordNRone)= (x1-x21)/d12;
   gradL1L2m1(coordNRone+num)=(y1-y21)/d12;
   gradL1L2m1(coordNRtwo)=(x21-x1)/d12+(x23-x3)/d23;
   gradL1L2m1(coordNRtwo+num)=(y21-y1)/d12+(y23-y3)/d23;
   gradL1L2m1(coordNRthree)=(x3-x23)/d23;
   gradL1L2m1(coordNRthree+num)=(y3-y23)/d23;
   
   gradL1L2m1=gradL1L2m1*(-1/(d12+d23)*(d12+d23));
   
   
    firstpart=firstpart+pow((pi-acos(c)),2)*gradL1L2m1;
    
  }
  
  //now the second part = 1/(l1+l2) * grad (pi-theta)^2;
  
  for(int i=0;i<springpairs.size();i++){
   
   
   for(int j=0;j<GRADnumerator.size();j++){ //Set the new gradients to zero
       GRADnumerator(j)=0;
       GRADdenumerator(j)=0;
       GRADc(j)=0;
    }
      
  int springone=springpairs[i][0];
  int springtwo=springpairs[i][1];
            
  int coordNRone=springlist[springone].one;
  int coordNRtwo=springlist[springone].two;
  int coordNRthree=springlist[springtwo].two;
  
  double x1=XY(coordNRone);
  double y1=XY(coordNRone+num);
  
  double x21=XY(coordNRtwo)+springlist[springone].wlr;
  double y21=XY(coordNRtwo+num)+springlist[springone].wud;
  
  double x23=XY(coordNRtwo);
  double y23=XY(coordNRtwo+num);
  
  double x3=XY(coordNRthree)+springlist[springtwo].wlr;
  double y3=XY(coordNRthree+num)+springlist[springtwo].wud;

  //cout<<x1<<"  "<<y1<<"  "<<x21<<"  "<<y21<<"  "<<x23<<"  "<<y23<<"  "<<x3<<"  "<<y3<<endl;
  
  Vector2d v1,v2; 
  v1<<(x1-x21),(y1-y21);
  v2<<(x3-x23),(y3-y23);
  
   double d12=distance1(x1,y1,x21,y21);
   double d23=distance1(x3,y3,x23,y23); 
  
   //numerator=v1.dot(v2);
   numerator=(x1-x21)*(x3-x23)+(y1-y21)*(y3-y23);
   denumerator=d12*d23;
   double c=numerator/denumerator;
   if(c>1) c=1;
   if(c<-1) c=-1;
   //This is the sin(theta);
   s=sqrt(1-c*c);
   if(s<0.0001) s=0.0001;
   s /= s;

    
    
    GRADnumerator(coordNRone)=GRADnumerator(coordNRone)-x23+x3;
    GRADnumerator(coordNRone+num)=GRADnumerator(coordNRone+num)-y23+y3;
    
    GRADnumerator(coordNRtwo)=GRADnumerator(coordNRtwo)-x3+x21+x23-x1;
    GRADnumerator(coordNRtwo+num)=GRADnumerator(coordNRtwo+num)-y3+y21+y23-y1;
    
    GRADnumerator(coordNRthree)=GRADnumerator(coordNRthree)-x21+x1;
    GRADnumerator(coordNRthree+num)=GRADnumerator(coordNRthree+num)-y21+y1;

    
    GRADdenumerator(coordNRone)=GRADdenumerator(coordNRone)+(d23/d12)*(x1-x21);
    GRADdenumerator(coordNRone+num)=GRADdenumerator(coordNRone+num)+(d23/d12)*(y1-y21);
    
    GRADdenumerator(coordNRtwo)=GRADdenumerator(coordNRtwo)+(d23/d12)*(x21-x1);
    GRADdenumerator(coordNRtwo+num)=GRADdenumerator(coordNRtwo+num)+(d23/d12)*(y21-y1);
    
    GRADdenumerator(coordNRtwo)=GRADdenumerator(coordNRtwo)+(d12/d23)*(x23-x3);
    GRADdenumerator(coordNRtwo+num)=GRADdenumerator(coordNRtwo+num)+(d12/d23)*(y23-y3);
    
    GRADdenumerator(coordNRthree)=GRADdenumerator(coordNRthree)+(d12/d23)*(x3-x23);
    GRADdenumerator(coordNRthree+num)=GRADdenumerator(coordNRthree+num)+(d12/d23)*(y3-y23);

    GRADc=(denumerator*GRADnumerator-numerator*GRADdenumerator)/(pow(denumerator,2));
    //cout<<GRADc.dot(GRADc)<<endl;

    //double dacos=-1-c*c/2-3*c*c*c*c/8;
    //if(GRADc.dot(GRADc)<.1) GRADc=1/s*GRADc;
    secondpart=secondpart+(1.0/(d12+d23))*2*(pi-acos(c))*s*GRADc;
  
  }

  grad=kappa*(firstpart+secondpart);
  return grad; 
}


double dEda(const VectorXd &XY,const VectorXd &s0,const vector<spring> &springlist,
    const vector<vector<int>> &springpairs,double kappa)
{  
    double out;
    out=s0.dot((Gradient(springlist,XY)+gradEbend(springpairs,springlist,XY,kappa)));
    return out;  
}

double quad(double x)
{
 double out=x*x-2*x-1;   
 return out;
}    
    

void doBracketfind(double &a1,double &a2,
                   const VectorXd &XY,
                   const VectorXd &s0, 
                   const vector<spring> &springlist,
                   const vector<vector<int>> &springpairs, 
                   double kappa)
//This function finds the inteval on which a mathematical function passes through zero.
//that is [x1,x2] where f(x1)*f(x2)<0.0;
{
 int maxit=50;   
 double f1,f2,FACTOR;
 f1=dEda(XY+a1*s0,s0,springlist,springpairs,kappa);
 f2=dEda(XY+a2*s0,s0,springlist,springpairs,kappa);

 FACTOR=1.6;
  
 if(a1==a2){ //We need two different points
    cout<<"Bad initial range in zbrac"<<endl;
  }
 
  for(int j=0;j<maxit;j++){ //Make a bracket.
    if(f1*f2<0.0) break;
    
    if(abs(f1)<abs(f2)){
        a1=a1+FACTOR*(a1-a2);
        f1=dEda(XY+a1*s0,s0,springlist,springpairs,kappa);
    }
    else{
        a2=a2+FACTOR*(a2-a1);
        f2=dEda(XY+a2*s0,s0,springlist,springpairs,kappa);
    }
 //cout<<f1<<"  "<<f2<<endl;
  }  
}
    
    
    
    
void doBisection(double a1,double a2,double &root,
    const VectorXd &XY,
    const VectorXd &s0, 
    const vector<spring> &springlist,
    const vector<vector<int>> &springpairs, 
    double kappa)
{
 double f1,f2,fc,c;
 int q=0;
 do{
 c=0.5*(a1+a2);
 f1=dEda(XY+a1*s0,s0,springlist,springpairs,kappa);
 f2=dEda(XY+a2*s0,s0,springlist,springpairs,kappa);
 fc=dEda(XY+c*s0,s0,springlist,springpairs,kappa);
 
 if(f1*fc>0.0) a1=c;
 else a2=c;
 q++;
 // cout<<a1<<"  "<<a2<<"  "<<fc<<endl;   
}while(abs(fc)>.000001 && q<50);
 root=c;
}

// void doBisection2(double x1,double x2,double &root,
//     const VectorXd &XY,
//     const VectorXd &s0, 
//     const vector<spring> &springlist,
//     const vector<vector<int>> &springpairs, 
//     double kappa)
// {
//     double xacc=.00000001;
//     double dx,f,fmid,xmid,rtb;
//     
//     f=dEda(XY+x1*s0,s0,springlist,springpairs,kappa);
//     fmid=dEda(XY+x2*s0,s0,springlist,springpairs,kappa);
//     
//     rtb=f<0.0 ? (dx=x2-x1,x1) : (dx=x1,x2,x2);
//     for(int j=0;j<50;j++){
//         fmid=quad(xmid=rtb+(dx*=0.5));
//         if (fmid<=0.0) rtb=xmid;
//         if (abs(dx)<xacc||fmid==0) root=rtb;
//         
//     } 
// }






void doFalsePosition(double &a1,double &a2,double &root,
                    const VectorXd &XY,
                    const VectorXd &s0, 
                    const vector<spring> &springlist,
                    const vector<vector<int>> &springpairs, 
                    double kappa)
{
 double fl,fh,xl,xh,swap,dx,del,f;   
 double xacc=.00001;
 
 int Maxit=50;
 fl=dEda(XY+a1*s0,s0,springlist,springpairs,kappa);
 fh=dEda(XY+a2*s0,s0,springlist,springpairs,kappa);

 
 if(fl<0.0){  //xl =xlow and xh=xhigh --> f(xl)<f(xh);
     xl=a1; 
     xh=a2;
 }
 else{
     xl=a2;
     xh=a1;
     swap=fl;
     fl=fh;
     fh=swap;
}
dx=xh-xl;
for(int i=0;i<Maxit;i++){
    root=xl+dx*fl/(fl-fh); //This is a secant step
    f=dEda(XY+root*s0,s0,springlist,springpairs,kappa);
    if(f<0.0){
     del=xl-root;
     xl=root;
     fl=f;
    }
    else{
        del=xh-root;
        xh=root;
        fh=f;
    }
    dx=xh-xl;
    if(abs(del)<xacc || f==0.0) break;
}
}

void doSecant(double &root,
              const VectorXd &XY,
              const VectorXd &s0, 
              const vector<spring> &springlist,
              const vector<vector<int>> &springpairs, 
              double kappa)
{
 double an2=0.0;
 double an1=0.0001;
 double an;
 double tol=0.0000001;
 int q=0; 
 double dEda2,dEda1;
 
dEda2=dEda(XY+an2*s0,s0,springlist,springpairs,kappa);

 do{ 
    dEda1=dEda(XY+an1*s0,s0,springlist,springpairs,kappa);
    an=an1-dEda1*(an1-an2)/(dEda1-dEda2);
    an2=an1;
    an1=an;
    dEda2=dEda1;   
    q++;
 }while(q<50 && abs(an2-an1)>tol);   
 root=an;   
}


void doBrent(double x1, double x2,double tol,double &root)
{
int iter;
int ITMAX=100;
double EPS=3e-8;
double a=x1;
double b=x2;
double c=x2;
double d,e,min1,min2;
double fa=quad(a);
double fb=quad(b);
double fc,p,q,r,s,tol1,xm;

fc=fb;

for(iter=1;iter<ITMAX;iter++) {
    if( (fb>0.0 && fc >0.0) ||(fb<0.0 && fc <0.0)){
        c=a;
        fc=fa;
        d=b-a;
        e=d;
    }
    if(abs(fc)<abs(fb)) {
        a=b;
        b=c;
        c=a;
        fa=fb;
        fb=fc;
        fc=fa;
    }
    tol1=2.0*EPS*abs(b)+0.5*tol;
    xm=0.5*(c-b);
    if(abs(xm)<=tol1 ||fb == 0.0) {
        root=b;
        break;
    }
    if(abs(e)>=tol1 && abs(fa)>abs(fb))  {
        s=fb/fa;
        if(a==c){
            p=2.0*xm*s;
            q=1.0-s;
        } else {
            q=fa/fc;
            r=fb/fc;
            p=s* (2.0*xm*q*(q-r)-(b-a)*(r-1.0));
            q=(q-1.0)*(r-1.0)*(s-1.0);
        }
        if(p>0.0) q=-q;
        p=abs(p);
        min1=3.0*xm*q-abs(tol1*q);
        min2=abs(e*q);
        if(2.0*p<(min1<min2 ? min1:min2)){
            e=d;
            d=p/q;
        } else{
            d=xm;
            e=d;
        }
    } else{
        d=xm;
        e=d;
    }
    a=b;
    fa=fb;
    if(abs(d)>tol1){
        b+=d;
    } else{
        b+=SIGN(tol1,xm);
        fb=quad(b);
        
    }
//root=0.0;   
}
    
    
    
    
    
    
    
    
    
    
}


