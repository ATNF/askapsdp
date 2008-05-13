/// @file
///
/// Provides generic methods for pattern matching 
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <patternmatching/GrothTriangles.h>
#include <iostream>
#include <math.h>
#include <map>
#include <vector>
#include <string>

namespace askap
{
  namespace analysis
  {

    namespace matching
    {

      Point& Point::operator=(const Point& p)
      {
	if(this==&p) return *this;
	this->itsX = p.itsX;
	this->itsY = p.itsY;
	this->itsFlux = p.itsFlux;
	this->itsID = p.itsID;
	return *this;
      }

      //**************************************************************//
      //**************************************************************//

      Triangle::Triangle()
      {
	std::vector<Point> pts(3); this->itsPts=pts;
      }

      Triangle::Triangle(Point a, Point b, Point c)
      {
	std::vector<Point> pts(3); this->itsPts=pts;
	this->define(a,b,c);
      }

      Triangle::Triangle(double x1, double y1, double x2, double y2, double x3, double y3)
      {
	std::vector<Point> pts(3); this->itsPts=pts;
  	Point pt1(x1,y1);
	Point pt2(x2,y2);
	Point pt3(x3,y3);
	this->define(pt1,pt2,pt3);
      }

      //**************************************************************//

      Triangle& Triangle::operator= (const Triangle& t)
      {
	if(this==&t) return *this;
	this->itsLogPerimeter = t.itsLogPerimeter;
	this->itIsClockwise = t.itIsClockwise;
	this->itsRatio = t.itsRatio;
	this->itsRatioTolerance = t.itsRatioTolerance;
	this->itsAngle = t.itsAngle;
	this->itsAngleTolerance = t.itsAngleTolerance;
	this->itsPts = t.itsPts;
	return *this;
      }

      //**************************************************************//

      void Triangle::define(Point a, Point b, Point c)
      {
	std::vector<Point> ptslist(3);
  	ptslist[0]=a;
	ptslist[1]=b;
	ptslist[2]=c;
	std::vector<Side> sides(3);
	sides[0].define(a,b);
	sides[1].define(b,c);
	sides[2].define(c,a);
	std::vector<short> vote(3);
	for(int i=0;i<3;i++){
	  if( (min_element(sides.begin(), sides.end()) - sides.begin()) == i) vote[i]=1;
	  else if( (max_element(sides.begin(), sides.end()) - sides.begin()) == i) vote[i]=3;
	  else vote[i] = 2;
	}
	short matrix[3][3];
	for(int i=0;i<3;i++) matrix[i][i]=0;
	matrix[1][0] = matrix[0][1] = vote[0];
	matrix[2][1] = matrix[1][2] = vote[1];
	matrix[0][2] = matrix[2][0] = vote[2];

	for(int i=0;i<3;i++){
	  int sum=0;
	  for(int j=0;j<3;j++) sum += matrix[i][j];
	  switch(sum){
	  case 4: this->itsPts[0] = ptslist[i]; break;
	  case 3: this->itsPts[1] = ptslist[i]; break;
	  case 5: this->itsPts[2] = ptslist[i]; break;
	  }
	}

	std::sort(sides.begin(), sides.end());
	
	// the sides are now ordered, so that the first is the shortest

	// use terminology from Groth 1986, where r2=shortest side, r3=longest side
	double r2= sides.begin()->length(), r3= sides.rbegin()->length();
	double dx2=sides.begin()->run(),    dx3=sides.rbegin()->run();
	double dy2=sides.begin()->rise(),   dy3=sides.rbegin()->rise();

	this->itsRatio = r3/r2;

	this->itsAngle = (dx3*dx2 + dy3*dy2) / (r3*r2);

	double sum=0.;
	for(int i=0;i<3;i++) sum += sides[i].length();
	this->itsLogPerimeter = log10(sum);

	double tantheta = (dy2*dx3 - dy3*dx2) / (dx2*dx3 + dy2*dy3);
	this->itIsClockwise = (tantheta > 0.);

	defineTolerances();

      }

      //**************************************************************//

      void Triangle::defineTolerances(double epsilon)
      {

	Side side1_2(itsPts[0].x()-itsPts[1].x(),itsPts[0].y()-itsPts[1].y());
	Side side1_3(itsPts[0].x()-itsPts[2].x(),itsPts[0].y()-itsPts[2].y());
	double r2=side1_2.length(),r3=side1_3.length();

	double sinthetaSqd = 1. - this->itsAngle*this->itsAngle;

	double factor = 1./(r3*r3) - this->itsAngle/(r3*r2) + 1./(r2*r2);

	this->itsRatioTolerance = 2. * this->itsRatio * this->itsRatio * epsilon * epsilon * factor;

	this->itsAngleTolerance = 2. * sinthetaSqd * epsilon * epsilon * factor +
	  3. * this->itsAngle * this->itsAngle * pow(epsilon,4) * factor * factor;
	

      }

      //**************************************************************//

      bool Triangle::isMatch(Triangle &comp, double epsilon)
      {
	defineTolerances(epsilon);
	
	double ratioSep = this->itsRatio - comp.ratio();
	ratioSep *= ratioSep;
	double ratioTol = this->itsRatioTolerance + comp.ratioTol();
	double angleSep = this->itsAngle - comp.angle();
	angleSep *= angleSep;
	double angleTol = this->itsAngleTolerance + comp.angleTol();

	return ((ratioSep < ratioTol) && (angleSep < angleTol));
      }

      //**************************************************************//
      //**************************************************************//

      std::vector<Triangle> getTriList(std::vector<Point> pixlist)
      {
	std::vector<Triangle> triList;
	int npix = pixlist.size();
	std::cout << "Pixel list of size "<<npix<<":\n";
 
	for(int i=0;i<npix-2;i++){
	  for(int j=i+1;j<npix-1;j++){
	    for(int k=j+1;k<npix;k++){

	      Triangle tri(pixlist[i],pixlist[j],pixlist[k]);
	      if(tri.ratio()<10.) triList.push_back(tri);

	    }
	  }
	}

	std::cout << "This makes a list of " << triList.size() << " triangles\n";

	return triList;
      }

      //**************************************************************//

      std::vector<std::pair<Triangle,Triangle> > matchLists(std::vector<Triangle> list1, std::vector<Triangle> list2, double epsilon)
      {

	int size1=list1.size(),size2=list2.size();

	// sort in order of increasing ratio
	std::stable_sort(list1.begin(), list1.end());
	std::stable_sort(list2.begin(), list2.end());
  
	// find maximum ratio tolerances for each list
	double maxTol1=list1[0].ratioTol(),maxTol2=list2[0].ratioTol();
	for(int i=1;i<size1;i++){
	  list1[i].defineTolerances(epsilon);
	  if(list1[i].ratioTol()>maxTol1) maxTol1=list1[i].ratioTol();
	}
	for(int i=1;i<size2;i++){
	  list2[i].defineTolerances(epsilon);
	  if(list2[i].ratioTol()>maxTol2) maxTol2=list2[i].ratioTol();
	}

	std::vector<bool> matches(size1*size2,false);

	int nmatch=0;
	// loop over the lists, finding matches
	for(int i=0; i<size1; i++){
	  double maxRatioB = list1[i].ratio() + sqrt(maxTol1+maxTol2);
	  double minRatioB = list1[i].ratio() - sqrt(maxTol1+maxTol2);

	  for(int j=0; j<size2 && list2[j].ratio()<maxRatioB; j++){

	    if(list2[j].ratio() > minRatioB)
	      matches[i+j*size1] = list1[i].isMatch(list2[j],epsilon);
	
	    if(matches[i+j*size1]) nmatch++;
	  }

	}
  
	std::cout << "Number of matching triangles = " << nmatch << "\n";;
	std::vector<std::pair<Triangle,Triangle> > matchList;

	for(int i=0;i<size1;i++){
	  for(int j=0;j<size2;j++){
	    if(matches[i+j*size1]) {
	      std::pair<Triangle,Triangle> match(list1[i],list2[j]);
	      matchList.push_back(match);
	    }
	  }
	}

	return matchList;

      }

      //**************************************************************//

      void trimTriList(std::vector<std::pair<Triangle,Triangle> > &trilist)
      {
	double mean=0.,rms=0.,mag;
	unsigned int nIter=0,size;
	unsigned int nSame=0,nOpp=0;
	const unsigned int maxIter = 5;
	do{
	  size = trilist.size();
	  for(unsigned int i=0;i<size; i++){
	    mag = trilist[i].first.perimeter() - trilist[i].second.perimeter();
	    mean += mag;
	  }
	  mean /= double(size);
	  for(unsigned int i=0;i<size; i++){
	    mag = trilist[i].first.perimeter() - trilist[i].second.perimeter();
	    rms += (mag-mean)*(mag-mean);
	  }
	  rms  = sqrt(rms / double(size-1));

	  for(unsigned int i=0;i<size; i++){
	    if(trilist[i].first.isClockwise()==trilist[i].second.isClockwise()) nSame++;
	    else nOpp++;
	  }
	  double trueOnFalse = abs(nSame-nOpp) / double(nSame+nOpp-abs(nSame-nOpp));
	  double scale;
	  if(trueOnFalse < 1.) scale = 1.;
	  else if(trueOnFalse > 10.) scale = 3.;
	  else scale = 2.;

	  int ctr=0;
	  std::vector<std::pair<Triangle,Triangle> > newlist;
	  for(unsigned int i=0;i<size; i++){
	    mag = trilist[i].first.perimeter() - trilist[i].second.perimeter();
	    std::cout << mag << " " <<(mag-mean)/rms << "\n";
	    if(fabs((mag-mean)/rms) > scale){
	    }
	    else{
	      newlist.push_back(trilist[i]);
	      ctr++;
	    }
	  }

	  trilist = newlist;

	  nIter++;

	} while(nIter<maxIter && (trilist.size() < size) && trilist.size()>0);

	unsigned int ctr=0;
	for(unsigned int i=0;i<trilist.size(); i++){
	  if(trilist[i].first.isClockwise()==trilist[i].second.isClockwise()) nSame++;
	  else nOpp++;
	}
	while(ctr<trilist.size()){
	  if((nSame>nOpp)&&
	     (trilist[ctr].first.isClockwise()!=trilist[ctr].second.isClockwise()))
	    trilist.erase(trilist.begin()+ctr);
	  else if((nOpp>nSame) && 
		  (trilist[ctr].first.isClockwise()==trilist[ctr].second.isClockwise()))
	    trilist.erase(trilist.begin()+ctr);
	  else ctr++;
	}

      }

      //**************************************************************//

      std::vector<std::pair<Point,Point> > vote(std::vector<std::pair<Triangle,Triangle> > trilist)
      {
	std::multimap<int,std::pair<Point,Point> > voteList;
	std::multimap<int,std::pair<Point,Point> >::iterator vote;
	std::multimap<int,std::pair<Point,Point> >::reverse_iterator rvote;

	for(unsigned int i=0;i<trilist.size();i++){

	  std::vector<Point> ptlist1 = trilist[i].first.getPtList();
	  std::vector<Point> ptlist2 = trilist[i].second.getPtList();

	  for(int p=0;p<3;p++){ // for each of the three points:

	    int thisVote=1;
	    bool finished=false;
	    if(voteList.size()>0){
	      vote = voteList.begin();
	      while(!finished && vote!=voteList.end()){
		if((vote->second.first.ID() != ptlist1[p].ID()) ||
		   (vote->second.second.ID() != ptlist2[p].ID())  ){
		  vote++;
		}
		else{
		  // if the IDs match
		  thisVote = vote->first + 1;
		  voteList.erase(vote);
		  finished = true;
		}
	      }
	    }
	    std::pair<Point,Point> thisPt(ptlist1[p],ptlist2[p]);
	    voteList.insert(std::pair<int,std::pair<Point,Point> >(thisVote,thisPt));
	  }

	}

	std::cout << "\n";
	std::vector<std::pair<Point,Point> > outlist;
	for(vote=voteList.begin();vote!=voteList.end();vote++){
	  std::cout << vote->first << ": "
		    << "#"<<vote->second.first.ID() << " " << vote->second.first.x() << " " << vote->second.first.y() << "  "
		    << "#"<<vote->second.second.ID() << " " << vote->second.second.x() << " " << vote->second.second.y() << "\n";
	}

  
	bool stop = false;
	int prevVote=voteList.rbegin()->first;
	for(rvote=voteList.rbegin();rvote!=voteList.rend() && !stop;rvote++){
    
	  for(unsigned int i=0;i<outlist.size() && !stop;i++){
	    stop = (rvote->second.first.ID() == outlist[i].first.ID());
	  }
	  if(rvote!=voteList.rbegin()) stop = stop || (rvote->first < 0.5*prevVote);
	  if(!stop) outlist.push_back(rvote->second);
	  prevVote = rvote->first;
	}
      
	return outlist;
      }


    }
  }
}
