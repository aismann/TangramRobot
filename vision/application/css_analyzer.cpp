// Copyright 2009 Erik Weitnauer
/// Visulalization of the CSS corner detection algorithm.

#include <ICLQuick/Common.h>
#include <ICLCC/Color.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLCore/Line.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/CornerDetectorCSS.h>
#include <fstream>
#include <iostream>

GUI gui("vsplit[@handle=B]");

Mutex mutex;
Color refColor = Color(255,255,255);
CornerDetectorCSS css;

std::string create_camcfg(const std::string&, const std::string &hint){
  return str("camcfg(")+hint+")[@maxsize=5x2]";
}

class Mouse : public MouseHandler{
  public:
  virtual void process(const MouseEvent &event){
    if(event.isPressEvent()){
      if(event.getColor().size() == 3) {
        Mutex::Locker l(mutex);
        for(int i=0;i<3;++i) refColor[i] = event.getColor()[i];
        std::cout << "new Ref-Color:"  << refColor.transp() << std::endl;
      }
    }
  }
} mouse;

void init(){
	GUI controls("vbox");
  controls << ( GUI("hbox") 
      			<< create_camcfg(FROM_PROGARG("-input"))
				    <<  "combo(color image,binary image)[@out=vis]"
				    << "button(print positions to file)[@handle=print-to-file-button]"
          );
  controls << "fslider(0,1,0.03)[@out=t@label=threshold]";
  controls << "fslider(1,20,3)[@out=sigma@label=gaussian sigma]";
  controls << "fslider(0,10,1.5)[@out=rc_coeff@handle=rc_coeff_handle@label=round corner coefficient]";
  controls << "fslider(0,1000,100)[@out=k_cutoff@label=curvature cutoff]";
  controls << "fslider(0,180,162)[@out=max_angle@label=maximum corner angle]";
  controls << "fslider(0,180,10)[@out=straight_line_thresh@label=straight line threshold]";
	
	gui << ( GUI("vsplit")
					 << ( GUI("hbox")
		      			<< "draw[@handle=img_in@minsize=16x12]"
					 			<< "draw[@handle=img1@minsize=16x12]"
					 			<< "draw[@handle=img2@minsize=16x12]"
					 		)
					 << "draw[@handle=img3@minsize=16x12]"
				 )
			<< controls;

  gui.show();
  gui.getValue<FSliderHandle>("rc_coeff_handle").setValue(1.5);
  (*gui.getValue<DrawHandle>("img_in"))->install(&mouse);
}

template<class T>
    void thresh(const Img<T> &input, Img8u &result, float t,const Color &ref){
  Mutex::Locker l(mutex);
  result.setChannels(1);
  result.setSize(input.getSize());
  const Channel<T> cs[3] = {input[0], input[1], input[2]};
  Channel8u dst = result[0];
  t *= 3*255*255;
  for(int i=0;i<cs[0].getDim();++i){
    int d = 0;
    for(int c=0;c<3;++c){
      d += pow( double(cs[c][i] - ref[c]) ,2.0); 	
    } 
    dst[i] = 255 * (d < t);
  }
    } 

template <class T>
inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss.precision(3);
  ss << t;
  return ss.str();
}

const vector< Point32f > &getThinnedBoundary(const vector< Point > &b) {
  static vector< Point32f > thinned;
  thinned.clear();
  if (b.size() < 2) return thinned;
  Point pre = b[b.size()-2];
  Point cur = b[b.size()-1];
  Point post; 
  for (unsigned i=0; i < b.size(); i++) {
    // search for the first point not in the 8 neighbourhood of current point
    if ((abs(b[i].x - cur.x) > 1) || (abs(b[i].y - cur.y) > 1)) {
      thinned.push_back((Point32f)cur);
      pre = cur;
      cur = post;
      post = b[i];
    } else post = b[i];
  }
  return thinned;
}

void setCSSParameters() {
	css.setAngleThreshold(gui.getValue<float>("max_angle"));
  css.setRCCoeff(gui.getValue<float>("rc_coeff"));
  css.setSigma(gui.getValue<float>("sigma"));
  css.setCurvatureCutoff(gui.getValue<float>("k_cutoff"));
  css.setStraightLineThreshold(gui.getValue<float>("straight_line_thresh"));
  css.setDebugMode(true);
}

void drawInput(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->color(255,0,0,255); w->fill(255,0,0,255);
  w->linestrip(css_inf.boundary,true);
  for (unsigned int i=0; i<css_inf.corners.size(); i++) {
    w->color(255,50,50,255); w->fill(255,50,50,255);
    w->ellipse(css_inf.corners[i].x-2, css_inf.corners[i].y-2,4,4);
  }
}

void drawStep1(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->color(255,0,0);
  w->linestrip(css_inf.boundary,true);
	// draw smoothed boundary.
	float x,y,lx=-1,ly=-1;
	int n = css_inf.smoothed_boundary.size();
	for (int i=0; i<n; i++) {
		if (i<css_inf.offset || i>=n-css_inf.offset) continue;
		else w->color(0,0,0);
		x = css_inf.smoothed_boundary[i].x; y = css_inf.smoothed_boundary[i].y;
	  if (lx != -1) w->line(lx,ly,x,y);
	  lx = x; ly = y;
	}
	w->color(0,0,0);
	w->ellipse(css_inf.smoothed_boundary[css_inf.offset].x-1, css_inf.smoothed_boundary[css_inf.offset].y-1,2,2);
}

void drawStep3(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->rel(); // positions between [0,1]
	// draw kurvature
	int n = css_inf.kurvature.size();
	float lx=0, ly=0, x, y;
	float s=0.2; // scaling
	
//	for (y=0; y<1/s; y+= s*0.2) {
//		w->color(0, 0, 0);
//		w->line(0,1-y*s-0.005,0.05,1-y*s-0.005);
//		w->text(str(y),0.05,1-y*s-0.02,-1,-1,6);
//	}
	for (int i=0; i<n; i++) {
		if (i<css_inf.offset || i>=n-css_inf.offset) w->color(180,180,180);
		else w->color(0, 0, 0);
		x = float(i)/n;
		y = css_inf.kurvature[i]*s;
		w->line(lx,1-ly,x,1-y);
		lx = x; ly = y;
	}
	// draw extrema
	w->nofill();
	for (unsigned int i=0; i<css_inf.extrema.size(); i++) {
		if (i % 2 == 0) w->color(0,255,0); else w->color(0,0,255);
		w->ellipse(float(css_inf.extrema[i])/n-0.005,1-s*css_inf.kurvature[css_inf.extrema[i]]-0.005,0.01,0.01);
	}
	// draw extrema without minima and rond corners
	w->color(200,50,50);
	for (unsigned int i=0; i<css_inf.maxima_without_round_corners.size(); i++) {
		w->ellipse(float(css_inf.maxima_without_round_corners[i])/n-0.005,1-s*css_inf.kurvature[css_inf.maxima_without_round_corners[i]]-0.005,0.01,0.01);
	}
	
	int count = 0;
	// draw extrema without false corners
	for (unsigned int i=0; i<css_inf.maxima_without_false_corners.size(); i++) {
		float x = float(css_inf.maxima_without_false_corners[i])/n;
		float y = s*css_inf.kurvature[css_inf.maxima_without_false_corners[i]];
		if (css_inf.maxima_without_false_corners[i]>=css_inf.offset &&
		    css_inf.maxima_without_false_corners[i]<n-css_inf.offset) {
			w->color(0,0,0);
			w->text(to_string(count++),x,1-y,-1,-1,10);
		} else {
			w->color(180,180,180);
			w->text("x",x,1-y);
		}
	}
	w->abs();
}

void drawStep2(ICLDrawWidget *w, const CornerDetectorCSS::DebugInformation &css_inf) {
	w->color(0,0,0);
  w->linestrip(css_inf.corners,true);
  for (unsigned int i=0; i<css_inf.angles.size(); i++) {
  	w->text("("+to_string(i)+")"+to_string(css_inf.angles[i]), css_inf.corners[i].x, css_inf.corners[i].y,-1,-1,8);
  }
}

void printToFile(const CornerDetectorCSS::DebugInformation &css_inf) {
	string prefix = pa("-o");
	ofstream out;
	// write boundary
	int n = css_inf.boundary.size();
	out.open((prefix+"boundary.txt").c_str());
	for (int i=0; i<n; ++i) {
	  out << css_inf.boundary[i].x << " " << css_inf.boundary[i].y << endl;
	}
	out.close();
	// write smoothed boundary
	int offset = css_inf.offset;
	out.open((prefix+"smoothed_boundary.txt").c_str());
	for (int i=offset; i<n+offset; ++i) {
		out << css_inf.smoothed_boundary[i].x << " " << css_inf.smoothed_boundary[i].y << endl;
	}
	out.close();
	// write kurvature
	out.open((prefix+"kurvature.txt").c_str());
	for (int i=offset; i<n+offset; ++i) {
	  out << css_inf.kurvature[i] << endl;
	}
	out.close();
	// write corner candidates
	out.open((prefix+"corners.txt").c_str());
	out << "index type" << endl;
	{
		// type 0: extrema
		for (unsigned int i=0; i<css_inf.extrema.size(); ++i) {
			int index = css_inf.extrema[i]-offset;
			if (index >= 0 && index < n) out << index << " 0" << endl;
		}
		// type 1: maxima
		for (unsigned int i=0; i<css_inf.maxima.size(); ++i) {
			int index = css_inf.maxima[i]-offset;
			if (index >= 0 && index < n) out << index << " 1" << endl;
		}
		// type 2: no_round_corners
		for (unsigned int i=0; i<css_inf.maxima_without_round_corners.size(); ++i) {
			int index = css_inf.maxima_without_round_corners[i]-offset;
			if (index >= 0 && index < n) out << index << " 2" << endl;
		}
		// type 3: no_false_corners
		for (unsigned int i=0; i<css_inf.maxima_without_false_corners.size(); ++i) {
			int index = css_inf.maxima_without_false_corners[i]-offset;
			if (index >= 0 && index < n) out << index << " 3" << endl;
		}
	}
	out.close();
}

void myrun(){
	// draw handles
	static DrawHandle &h = gui.getValue<DrawHandle>("img_in");
  static DrawHandle &h1 = gui.getValue<DrawHandle>("img1");
  static DrawHandle &h2 = gui.getValue<DrawHandle>("img2");
  static DrawHandle &h3 = gui.getValue<DrawHandle>("img3");
  // grabber
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  grabber.setIgnoreDesiredParams(true);
  grabber.setDesiredSize(pa("-size"));
  grabber.setDesiredDepth(depth8u);
	// images: get camera input and apply threshold
  const Img8u &image = *grabber.grab()->asImg<icl8u>();
  static Img8u threshedImage;
  static Img8u bgImage1(image.getSize(), 1); bgImage1.clear(0,255);
  static Img8u bgImage2(image.getSize(), 1); bgImage2.clear(0,255);
  static Img8u bgImage3(image.getSize(), 1); bgImage3.clear(0,255);
  thresh(image,threshedImage,gui.getValue<float>("t"),refColor);
	
	// draw background images
  static std::string &vis = gui.getValue<std::string>("vis");
  h = (vis == "color image") ? &image : &threshedImage;
	h1 = &bgImage1; h2 = &bgImage2; h3 = &bgImage3;

	// lock the DrawWidgets before drawing
  ICLDrawWidget *w = *h;
  ICLDrawWidget *w1 = *h1, *w2 = *h2, *w3 = *h3;
  w->lock(); w->reset();
  w1->lock(); w1->reset(); w2->lock(); w2->reset();
  w3->lock(); w3->reset();

	// detect regions
  static RegionDetector d(100,200000,255,255);
  const std::vector<icl::Region> &rs = d.detect(&threshedImage);
  setCSSParameters();
  // iterate over all regions and draw information onto the DrawWidgets
  for(unsigned int i=0;i<rs.size();++i) {
  	vector<Point32f> boundary;
  	{
  		boundary = getThinnedBoundary(rs[i].getBoundary());
  	}
  	{
  		css.detectCorners(boundary);
  	}
    const CornerDetectorCSS::DebugInformation &css_inf = css.getDebugInformation();
//    cout << "contour points: " << boundary.size() << endl;
//		cout << "extrema: " << css_inf.extrema.size() << endl;
//		cout << "maxima: " << css_inf.maxima.size() << endl;
//		cout << "maxima_without_round_corners: " << css_inf.maxima_without_round_corners.size() << endl;
//		cout << "maxima_without_false_corners: " << css_inf.maxima_without_false_corners.size() << endl;
//		cout << "corners: " << css_inf.corners.size() << endl;
		drawInput(w, css_inf);
    drawStep1(w1, css_inf);
    drawStep2(w2, css_inf);
    drawStep3(w3, css_inf);
    if (gui.getValue<ButtonHandle>("print-to-file-button").wasTriggered()) printToFile(css_inf);

    Point32f cog = rs[i].getCOG();
  	w->color(255,0,0,255); w->fill(255,0,0,255);
	  w->ellipse(cog.x-1, cog.y-1,2,2);  
  }
  w->unlock(); w1->unlock(); w2->unlock(); w3->unlock();
	
	// update the draw widgets
  h.update(); h1.update(); h2.update(); h3.update();
  Thread::msleep(5);
}


int main(int n, char **ppc){
  GenericGrabber::resetBus();
  paex("-input","define input device id and parameters");
  paex("-size","defines the input image size");
  paex("-output_prefix","prefix for files written");
  
  painit(n,ppc,"-size|-s(Size=VGA) -input|-i(device-type=dc,device-params=0) -output_prefix|-o(string=shape_)");
  QApplication app(n,ppc);
  ExecThread x(myrun);

  init();

  x.run();
  return app.exec();
}
