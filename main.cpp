//Modified from "eventloop.cpp" lecture code.
//enable up down left right arrows and q to exit,and space key.

#include <iostream>
#include <list>
#include <cstdlib>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <cmath>

/*Header files for X functions*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
using namespace std;

/*Defines for window geometry and keyboard events*/

const int Border =5;
const int BufferSize=10;
const int L=60;//length of plates

const int FPS=30;//frame per second

bool GameIsPlaying=false;//bool use to resume the game.
bool IsLandingOnPad=false;//bool use to detect if the ship landing on pad.
bool IsSpeadTooMuch=false;//bool use to detect if the ship is too fast.
/*information to draw on the window*/
bool isGoingTooFast=false;
struct XInfo{
    Display* display;
    int screen;
    Window window;
    GC gc[3];
    
    Pixmap pixmap;
    int width;
    int height;
};

//a less than function used to sort vector.
bool lesst(const XPoint& a, const XPoint& b){
    return a.x<=b.x;
}

//random number generator
int my_rand(int a, int b)
{
    return a + (b - a) * (double)rand() / RAND_MAX;
    
}


//a class of function.
class line_function {
    XPoint point1;
    XPoint point2;
public:
    bool intersect(line_function line2){
        double a1=(double)((point1.y)-(point2.y))/(double)(point1.x-point2.x);
        double b1=point1.y-a1*point1.x;
        
        double a2=(double)(line2.point1.y-line2.point2.y)/(double)(line2.point1.x-line2.point2.x);
        double b2=line2.point1.y-a2*line2.point1.x;
        
        double x=(b1-b2)/(a2-a1);
        double y=a1*x+b1;
//        cout<<"range is  "<<line2.point1.x<<" , "<<line2.point2.x<<endl;
        cout<<"the solution for x from"<<point1.x<<" to "<<point2.x<<"x is "<<x<<endl;
        cout<<"and a2-a1 equals to"<<a2-a1<<" "<<endl;
        cout<<"y is"<<y<<endl;

        for(int i=0;i<=line2.point2.x-line2.point1.x;i++){
            if ((abs((line2.point1.x+i)-x)<=1)&&((abs(line2.point1.y-y))<=1)){
                if(((line2.point1.x+i)<=point2.x)&&((line2.point1.x+i)>=point1.x)){
                    return true;
                }
            }
        }
         if((a1==0)&&(abs(line2.point1.y-point1.y)<=4)&&((line2.point1.x)>=point1.x)&&((line2.point2.x)<=point2.x)){
             IsLandingOnPad=true;
            return true;
         }
//        if((a1==0)&&(line2.point1.y>point1.y)&&((line2.point1.x)<=point2.x)&&((line2.point2.x)>=point1.x)){
//            IsLandingOnPad=false;
//            return true;
//        }
        if (line2.point1.y>=600){
            return true;
        }
        return false;
    }

    //Constructor
    line_function(XPoint a,XPoint b):point1(a),point2(b){};
};





/*
 * Helper method to put out a message on error exits.
 */
void error( string str ) {
    cerr << str << endl;
    exit(0);
}
/*
 * An abstract class representing displayable things.
 */
class Displayable {
public:
    virtual void paint(XInfo& xinfo) = 0;
};

/*
 * A text displayable
 */
class Text : public Displayable {
public:
    bool hide;
    virtual void paint(XInfo& xinfo) {
        if (hide==false){
            XDrawImageString( xinfo.display, xinfo.pixmap, xinfo.gc[1],
                             this->x, this->y, this->s.c_str(), this->s.length());
                             }
    }
    
    // constructor
    Text(int x, int y, string s,bool h):x(x), y(y), s(s),hide(h)  {}
private:
    XPoint p; // a 2D point (see also http://tronche.com/gui/x/xlib/graphics/drawing/)
    int x;
    int y;
    string s; // string to show
};

//plate
class Plate {
    XPoint left_end;
    XPoint right_end;
public:
    Plate(int left,int height){
        left_end.x=left;
        left_end.y=height;
        right_end.x=left+L;
        right_end.y=height;
    }
    void drawplate(XInfo &xinfo){
        XDrawRectangle(xinfo.display,xinfo.pixmap, xinfo.gc[1], left_end.x, left_end.y-5, 60 ,5);
    }
    XPoint Get_left_end(){
        return left_end;
    }
    XPoint Get_right_end(){
        return right_end;
    }
};




//Spaceship
class Spaceship : public Displayable {
private:
    int width;
    int height;
    int x;
    int y;
public:
    virtual void paint(XInfo &xinfo) {
         string CN="CN";
        string Help="Dude!You are going too fast!";
        string Scream="NOOOOOOO!!!";
            XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, width, height);
        
        XDrawImageString( xinfo.display, xinfo.pixmap, xinfo.gc[1],x+4,y+14, CN.c_str(),CN.length());
        if (isGoingTooFast&&y<320){
            XDrawImageString( xinfo.display, xinfo.pixmap, xinfo.gc[1],x-10,y-20, Help.c_str(),Help.length());
        }
        else if(isGoingTooFast&&y>=320){
            XDrawImageString( xinfo.display, xinfo.pixmap, xinfo.gc[1],x-10,y-20, Scream.c_str(),Scream.length());
        }
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, x+10, y-10);
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+10, y-10, x+20, y);
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x-3, y+height, x, y+height-10);
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x-3, y+height, x, y+height);
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+width, y+height-10, x+width+3, y+height);
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+width, y+height, x+width+3, y+height);
    }

    void move(XInfo &xinfo, int xDiff, int yDiff) {
       //if((x<=800&&x>=0)&&(y<=600&&y>=0)){
        //movement+boundary check.
        if ((x<=800&&x>=0)){
        x += xDiff;
        }
        if ((y>=0&&y<=600)){
            y=y+yDiff+2;
        }
        if (x>800){
            x-=(2+(x-800));
        }
        if (x<0){
            x+=(2+(0-x));
        }
        if (y<0){
            y+=(2+(0-y));
        }
        if (y>600){
            y-=(2+(y-600));
        }
    }

    int getX() {
        return x;
    }
    
    int getY() {
        return y;
    }
    int getWidth(){
        return width;
    }
    int getHeight(){
        return height;
    }
    
    Spaceship(int x, int y): x(x), y(y) {
        width = 20;
        height = 40;
    }
};

//speed meter
class SpeedMeter:public Displayable{
    int x;
    int y;
    int height;
    int width;
public:
    int currentSpeed;
    int maxSpeed;
    SpeedMeter(int x,int y,int width,int height,int initialspeed):x(x),y(y),width(width),height(height),currentSpeed(initialspeed){}
    
    virtual void paint(XInfo &xinfo){
        XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, width, height);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, width, (height/10)*currentSpeed);
    }
};
class HorizontalSpeedMeter:public Displayable{
    int x;
    int y;
    int height;
    int width;
public:
    int currentSpeed;
    int maxSpeed;
    HorizontalSpeedMeter(int x,int y,int width,int height,int initialspeed):x(x),y(y),width(width),height(height),currentSpeed(initialspeed){}
    
    virtual void paint(XInfo &xinfo){
        XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, width, height);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, (width/10)*currentSpeed, height);
    }
};

class LeftHorizontalSpeedMeter:public Displayable{
    int x;
    int y;
    int height;
    int width;
public:
    int currentSpeed;
    int maxSpeed;
    LeftHorizontalSpeedMeter(int x,int y,int width,int height,int initialspeed):x(x),y(y),width(width),height(height),currentSpeed(initialspeed){}
    
    virtual void paint(XInfo &xinfo){
        XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, width, height);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x+width-((double)width/10)*currentSpeed, y, ((double)width/10)*currentSpeed, height);
    }
};
class TopHorizontalSpeedMeter:public Displayable{
    int x;
    int y;
    int height;
    int width;
public:
    int currentSpeed;
    int maxSpeed;
    TopHorizontalSpeedMeter(int x,int y,int width,int height,int initialspeed):x(x),y(y),width(width),height(height),currentSpeed(initialspeed){}
    
    virtual void paint(XInfo &xinfo){
        XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, width, height);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y+height-((double)height/10)*currentSpeed, width, ((double)height/10)*currentSpeed);
    }
};

//Class pattern
class Pattern:public Displayable {
    
    vector< XPoint > points;
    vector<Plate>plates;
    vector<line_function>lines;
public:
    
    void init(int n, int max_x, int max_y,int num_plate) {//add one variable # of plate,and length of each plate.
        
        vector<int> p_of_plates; //a vector specify every plate's position.
        for (int i=0;i<num_plate;i++){
            int a;
            if (i+1<num_plate){
                a=my_rand((max_x/num_plate)*i,(max_x/num_plate)*(i+1));
            }
            else a=my_rand((max_x/num_plate)*i,max_x);
            if ((a-L)<0){
            }
            else a=a-L;
            p_of_plates.push_back(a);
            //cout<<i<<"is"<<a<<endl;
        }
        
        
        
        //now I am trying to construct a vector of plates.
        for (int i=0;i<num_plate;i++){
            int k=my_rand(2*(max_y/3),max_y);
            Plate tmp(p_of_plates[i],k);
            plates.push_back(tmp);
        }
        
        
        XPoint zero;  //the left bottom corner.
        zero.x=0;
        zero.y=max_y;
        XPoint max;  //the right bottom corner.
        max.x=max_x;
        max.y=max_y;
        points.push_back(zero);
        for (int j=0;j<num_plate;j++){
            points.push_back(plates[j].Get_left_end());
            points.push_back(plates[j].Get_right_end());
            for (int i=0; i < n; i++) {
                XPoint p;
                if (j+1>=num_plate){
                    p.x = my_rand(plates[j].Get_right_end().x, max_x);
                    p.y = my_rand(2*max_y/3, max_y);
                }
                else if(j==0){
                    p.x = my_rand(0,plates[j].Get_left_end().x);
                    p.y = my_rand(2*max_y/3, max_y);
                }
                else{
                    p.x = my_rand(plates[j].Get_right_end().x, plates[j+1].Get_left_end().x);
                    p.y = my_rand(2*max_y/3, max_y);
                    cout << p.x << "," << p.y << endl;
                }
                points.push_back(p);
                
            }
            sort(points.begin(),points.end(),lesst);
        }
        points.push_back(max);
        
        //initiate the line function as well.
        for(int i=0;i+1<points.size();i++){
            //cout<<" the "<<i<<" point is "<<points[i].x<<endl;
            line_function line_one(points[i],points[i+1]);
            lines.push_back(line_one);
        }
        for (int i=0;i<points.size();i++){
            cout<<" The patter point" <<i<< "has x"<<points[i].x<<" and y "<<points[i].y<<endl;
        }
    }
    
    virtual void paint(XInfo &xinfo) {
        // note the trick to pass a stl vector as an C array
        XDrawLines(xinfo.display,xinfo.pixmap,xinfo.gc[1],
                   &points[0], points.size(),  // vector of points
                   CoordModeOrigin ); // use absolute coordinates
        for(int i=0;i<plates.size();i++){
            plates[i].drawplate(xinfo);
         }
    }
    //experimental detection.
  
    bool detect(Spaceship spaceship,Pattern pattern){
        XPoint x1;
        XPoint x2;
        x1.x=spaceship.getX();
        x2.x=spaceship.getX()+spaceship.getWidth();
        x1.y=spaceship.getY()+spaceship.getHeight();
        x2.y=spaceship.getY()+spaceship.getHeight();
        line_function bottom(x1,x2);
        cout<<"spaceship at ("<<x1.x<<" , "<<x1.y<<endl;
        for(int i=0;i<pattern.lines.size();i++){
            if(pattern.lines[i].intersect(bottom)){
                return true;
            }
        }
        return false;
    }
};
/*
 * Helper method to return timestamp
 */
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

//a list of displayable.
list<Displayable *> listofd;
Spaceship apolo(10,10);
Text Starter(300,300,"Press space to start",false);
Text Restarter(300,300,"You lost! Press space to play again!",true);
Text Winner(300,300,"Congrats!You won! Press space to play again",true);
Text EngineStats(663,40,"Engine status",false);
Pattern pattern;
SpeedMeter vertical(700,100,10,40,0);
HorizontalSpeedMeter Horizontal(720,90,40,10,0);
LeftHorizontalSpeedMeter leftHorizontal(650,90,40,10,0);
TopHorizontalSpeedMeter TopMeter(700,50,10,40,0);
//SpeedMeter horizontal(690,90,-8,-40,0);

/*move the ship
 */
void handleDoubleBuffer(XInfo &xinfo, int xDiff, int yDiff) {
    apolo.move(xinfo, xDiff, yDiff);
}

//repaint function
void repaint( XInfo &xinfo) {
	list<Displayable *>::const_iterator begin = listofd.begin();
	list<Displayable *>::const_iterator end = listofd.end();
    
	// draw into the buffer
	// note that a window and a pixmap are “drawables”
	XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0],
                   0, 0, xinfo.width, xinfo.height);
	while( begin != end ) {
		Displayable *d = *begin;
		d->paint(xinfo); // the displayables know about the pixmap
		begin++;
	}
	// copy buffer to window
	XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0],
              0, 0, xinfo.width, xinfo.height,  // region of pixmap to copy
              0, 0); // position to put top left corner of pixmap in window
    
	XFlush( xinfo.display );
}

// EVENT LOOP!!!!!!!!!!!!
void eventloop(XInfo& xinfo){
    // add stuff to pain.
    listofd.push_front(&apolo);
    listofd.push_front(&Starter);
    listofd.push_front(&pattern);
    listofd.push_back(&Restarter);
    listofd.push_back(&Winner);
    listofd.push_back((&vertical));
    listofd.push_back(&Horizontal);
    listofd.push_back(&leftHorizontal);
    listofd.push_back(&TopMeter);
    listofd.push_back((&EngineStats));
    //listofd.push_front((&pattern));
    XEvent event;
    KeySym key;
    char text[BufferSize];
    unsigned long lastRepaint = 0;
    
    int xDiff=0;
    int yDiff=0;
    //infinite loop, enter q to exit.
    while (true) {
        if (XPending(xinfo.display)>0){
        XNextEvent(xinfo.display, &event);
            //These are engine status.
            if(yDiff>=0){
            vertical.currentSpeed=yDiff;
            }
            if(xDiff>=0){
                Horizontal.currentSpeed=xDiff;
            }
            bool d = pattern.detect(apolo,pattern);
            if (xDiff<=0){
                leftHorizontal.currentSpeed=abs(xDiff);
            }
            if (yDiff<=0){
                TopMeter.currentSpeed=abs(yDiff);
            }
            //Here to make the driver complain.
            if (GameIsPlaying){
                if(abs(xDiff)>=3||yDiff>4){
                    isGoingTooFast=true;
                }
                else {isGoingTooFast=false;}
            }
            //Here after the collision happen.
            if(d&&GameIsPlaying){
                GameIsPlaying=!GameIsPlaying;
                if(yDiff>5){
                    IsSpeadTooMuch=true;
                }
               xDiff=0;
               yDiff=0;
                cout<<"position  "<<apolo.getX()<<"  "<<apolo.getY()<<endl;
                if(IsLandingOnPad&&(!IsSpeadTooMuch)){
                    Winner.hide=false;
                }
                else {
                Restarter.hide=false;
                }
            }
            
        switch (event.type) {
                /*
                 * Exit when 'q' is typed.
                 * Arguments for XLookupString :
                 *                 event - the keyboard event
                 *                 text - buffer into which text will be written
                 *                 BufferSize - size of text buffer
                 *                 key - workstation independent key symbol
                 *                 0 - pointer to a composeStatus structure
                 */
            case Expose:
                if(event.xexpose.count ==0){
                    printf("Expose event recieved. Redrawing\n");
                    repaint(xinfo);
                }
                break;

            case KeyPress:
               int i =XLookupString((XKeyEvent*)&event, text, BufferSize, &key, 0 );
                cout << "KeySym " << key
                << " text='" << text << "'"
                << " at " << event.xkey.time
                << endl;
                //press q to exit
                if ( i == 1 && text[0] == 113 ) {
                    cout << "Terminated normally." << endl;
                   // XCloseDisplay(xinfo.display);
                    return;
                }
                //UP key
                if ( key == XK_Up  ) {
                    cout << "[Up]" << endl;
                    if(GameIsPlaying&&yDiff>=-2){
                    yDiff-=1;
                    }
                    }
                //Down Key
                if ( key == XK_Down  ) {
                    cout << "[D]" << endl;
                    if (GameIsPlaying&&(yDiff<=10)){
                    yDiff=yDiff+1;
                    }
                }
                //Left key
                if ( key == XK_Left ) {
                    cout << "[L]" << endl;
                    if (GameIsPlaying&&(xDiff>=-10)){
                    xDiff=xDiff-1;
                    }
                }
                //Right key
                if ( key == XK_Right  ) {
                    cout << "[D]" << endl;
                    if((xDiff<=10)&&GameIsPlaying){
                    xDiff+=1;
                    }
                }
                //Space
                if ( i == 1 && text[0] == 32 ) {
                    cout << "[ ]" << endl;
                    GameIsPlaying=!GameIsPlaying;
                    Starter.hide=true;
                    if(d){
                        apolo=Spaceship(10,10);
                        Restarter.hide=true;
                        if(IsLandingOnPad){
                           Winner.hide=true;
                        }
                        if(IsSpeadTooMuch){
                            IsSpeadTooMuch=false;
                        }
                        IsLandingOnPad=false;
                    }
                }
                break;
                    }
        }
       
        //cout<<"aaaa"<<endl;
        //Handle key press done, now do the drawing
        unsigned long end = now();
        if (end - lastRepaint > 1000000/FPS) {
            if (GameIsPlaying){
            handleDoubleBuffer(xinfo, xDiff, yDiff);
            }
            repaint(xinfo);
                lastRepaint = now();
        } else if (XPending(xinfo.display) == 0) {
            usleep(1000000/FPS - (end - lastRepaint));
        }
        
    }
}



void initX(int argc, char* argv[], XInfo& xinfo) {
    XSizeHints hints;
    unsigned long background, foreground;
    
    /*
     * Display opening uses the DISPLAY  environment variable.
     * It can go wrong if DISPLAY isn't set, or you don't have permission.
     */
    xinfo.display = XOpenDisplay( "" );
    if ( !xinfo.display ) {
        error( "Can't open display." );
    }
    
    /*
     * Find out some things about the display you're using.
     */
    xinfo.screen = DefaultScreen( xinfo.display );
    foreground = WhitePixel( xinfo.display, xinfo.screen );
    background = BlackPixel( xinfo.display, xinfo.screen );
    
    /*
     * Set up hints and properties for the window manager, and open a window.
     * Arguments to XCreateSimpleWindow :
     *                 display - the display on which the window is opened
     *                 parent - the parent of the window in the window tree
     *                 x,y - the position of the upper left corner
     *                 width, height - the size of the window
     *                 Border - the width of the window border
     *                 foreground - the colour of the window border
     *                 background - the colour of the window background.
     * Arguments to XSetStandardProperties :
     *                 display - the display on which the window exists
     *                 window - the window whose properties are set
     *                 Hello1 - the title of the window
     *                 Hello2 - the title of the icon
     *                 none - a pixmap for the icon
     *                 argv, argc - a comand to run in the window
     *                 hints - sizes to use for the window.
     */
    hints.x = 100;
    hints.y = 100;
    hints.width = 800;
    hints.height = 600;
    hints.flags = PPosition | PSize;
    xinfo.window = XCreateSimpleWindow( xinfo.display, DefaultRootWindow( xinfo.display ),
                                       hints.x, hints.y, hints.width, hints.height,
                                       Border, foreground, background );
    XSetStandardProperties( xinfo.display, xinfo.window, "Lunar Landing", "Lunar Landing", None,
                           argv, argc, &hints );
    //initiate all the terrain and pattern.
    pattern.init(3, (int)hints.width, (int)hints.height,4);
    /*
     * Get a graphics context and set the drawing colours.
     * Arguments to XCreateGC
     *           display - which uses this GC
     *           window - which uses this GC
     *           GCflags - flags which determine which parts of the GC are used
     *           GCdata - a struct that fills in GC data at initialization.
     */
    int i = 0;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], BlackPixel(xinfo.display, xinfo.screen));
    XSetBackground(xinfo.display, xinfo.gc[i], WhitePixel(xinfo.display, xinfo.screen));
    XSetFillStyle(xinfo.display, xinfo.gc[i], FillSolid);
    XSetLineAttributes(xinfo.display, xinfo.gc[i],
                       1, LineSolid, CapButt, JoinRound);
    
    // Reverse Video
    i = 1;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], WhitePixel(xinfo.display, xinfo.screen));
    XSetBackground(xinfo.display, xinfo.gc[i], BlackPixel(xinfo.display, xinfo.screen));
    XSetFillStyle(xinfo.display, xinfo.gc[i], FillSolid);
    XSetLineAttributes(xinfo.display, xinfo.gc[i],
                       1, LineSolid, CapButt, JoinRound);
    
    int depth = DefaultDepth(xinfo.display, DefaultScreen(xinfo.display));
    xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, hints.width, hints.height, depth);
    xinfo.width = hints.width;
    xinfo.height = hints.height;

    // Tell the window manager what input events you want.
    // XSelectInput( xinfo.display, xinfo.window, KeyPressMask);
    
    /*
     * Put the window on the screen.
     */
    XMapRaised( xinfo.display, xinfo.window );
    
    XFlush(xinfo.display);
    sleep(2);   // let server get set up before sending drawing commands
}

/*
 * Start executing here.
 *   First initialize window.
 *   Next loop responding to events.
 *   Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char* argv[] ) {
    XInfo xinfo;
    initX(argc, argv, xinfo);
    //mask all other input except key press
    XSelectInput(xinfo.display, xinfo.window, KeyPressMask);
    eventloop(xinfo);
    XCloseDisplay(xinfo.display);
}
