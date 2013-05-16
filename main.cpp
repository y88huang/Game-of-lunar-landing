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

/*Header files for X functions*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
using namespace std;

/*Defines for window geometry and keyboard events*/

const int Border =5;
const int BufferSize=10;

/*information to draw on the window*/
struct XInfo{
    Display* display;
    Window window;
    GC gc;
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


//try a class for plate.
class Plate {
    XPoint left_end;
    XPoint right_end;
    int L; //length of the plate.
public:
    Plate(int left,int height){
        left_end.x=left;
        left_end.y=height;
        right_end.x=left+L;
        right_end.y=height;
        L=60;
    }
    XPoint Get_left_end(){
        return left_end;
    }
    XPoint Get_right_end(){
        return right_end;
    }
    int Get_L(){
        return L;
    }
};


//Class pattern
class Pattern {
    
    vector< XPoint > points;
    
public:
    
    void init(int n, int max_x, int max_y,int num_plate,int L) {//add one variable # of plate,and length of each plate.
        vector<int> p_of_plates; //a vector specify every plate's position.
        for (int i=0;i<num_plate;i++){
            int a=(max_x/num_plate)*(i+1);
            if ((a-L)<0){
            }
            else a=a-L;
            p_of_plates.push_back(a);
            cout<<i<<"is"<<a<<endl;
        }
        //now I am trying to construct a vector of plates.
        vector<Plate>plates;
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
            p.x = my_rand(plates[j].Get_right_end().x, plates[j+1].Get_left_end().x);
            p.y = my_rand(2*max_y/3, max_y);
            cout << p.x << "," << p.y << endl;
            points.push_back(p);
        }
        sort(points.begin(),points.end(),lesst);
        }
        points.push_back(max);
    }

void paint(Display* display, Window window, GC gc) {
        // note the trick to pass a stl vector as an C array
        XDrawLines(display, window, gc,
                   &points[0], points.size(),  // vector of points
                   CoordModeOrigin ); // use absolute coordinates
        
    }
};

//a pattern
Pattern pattern;

//His repaint funtion.
void repaint(Display* display, Window window, GC gc) {
    XClearWindow( display, window );
    pattern.paint(display, window, gc);
    XFlush(display);
}





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
    virtual void paint(XInfo& xinfo) {
        XDrawImageString( xinfo.display, xinfo.window, xinfo.gc,
                         this->x, this->y, this->s.c_str(), this->s.length() );
    }
    
    // constructor
    Text(int x, int y, string s):x(x), y(y), s(s)  {}
    
private:
    XPoint p; // a 2D point (see also http://tronche.com/gui/x/xlib/graphics/drawing/)
    int x;
    int y;
    string s; // string to show
};

void eventloop(XInfo& xinfo){
    XEvent event;
    KeySym key;
    char text[BufferSize];
    //infinite loop, enter q to exit.
    while (true) {
        XNextEvent(xinfo.display, &event);
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
                    repaint(xinfo.display,xinfo.window,xinfo.gc);
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
                    XCloseDisplay(xinfo.display);
                    return;
                }
                //UP key
                if ( key == XK_Up  ) {
                    cout << "[Up]" << endl;
                }
                //Down Key
                if ( key == XK_Down  ) {
                    cout << "[D]" << endl;
                }
                //Left key
                if ( key == XK_Left ) {
                    cout << "[L]" << endl;
                }
                //Right key
                if ( key == XK_Right  ) {
                    cout << "[D]" << endl;
                }
                //Space
                if ( i == 1 && text[0] == 32 ) {
                    cout << "[ ]" << endl;
                }
                break;
                    }
    }
}


void initX(int argc, char* argv[], XInfo& xinfo) {
    XSizeHints hints;
    unsigned long background, foreground;
    int screen;
    
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
    screen = DefaultScreen( xinfo.display );
    foreground = WhitePixel( xinfo.display, screen );
    background = BlackPixel( xinfo.display, screen );
    
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
    
    /*
     * Get a graphics context and set the drawing colours.
     * Arguments to XCreateGC
     *           display - which uses this GC
     *           window - which uses this GC
     *           GCflags - flags which determine which parts of the GC are used
     *           GCdata - a struct that fills in GC data at initialization.
     */
    xinfo.gc = XCreateGC (xinfo.display, xinfo.window, 0, 0 );
    XSetBackground( xinfo.display, xinfo.gc, background );
    XSetForeground( xinfo.display, xinfo.gc, foreground );
    
    
    // Tell the window manager what input events you want.
    XSelectInput( xinfo.display, xinfo.window,
                 ButtonPressMask | KeyPressMask |
                 ExposureMask | ButtonMotionMask );
    pattern.init(4, (int)hints.width, (int)hints.height,4,60);
    
    /*
     * Put the window on the screen.
     */
    XMapRaised( xinfo.display, xinfo.window );
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
    eventloop(xinfo);
}
