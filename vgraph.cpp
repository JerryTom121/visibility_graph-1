#include "CImg.h"// Include CImg library header.
#include <iostream>
#include "line.h"
#include "point.h"
#include "skiplist.h"
#include <cmath>

using namespace cimg_library;
using namespace std;

//#undef min
//#undef max

const unsigned char WHITE[] = { 255, 255, 255 };
const unsigned char GREY[] = { 100, 100, 100 };
const unsigned char BLACK[] = { 0, 0, 0 };
const unsigned char RED[] = { 255, 0, 0 };
const unsigned char GREEN[] = { 0, 255, 0 };
const unsigned char BLUE[] = { 0, 0, 255};
const int screen_size = 500;

//-------------------------------------------------------------------------------
//  Prototypes
//-------------------------------------------------------------------------------
double vectorsAngle( int x, int y, int basex, int basey);
double distance( Point * a, Point * b );

//-------------------------------------------------------------------------------
//  Main procedure
//-------------------------------------------------------------------------------
int main()
{
	cout << endl << endl << "Visibility Graph by Dave Coleman -------------------- " << endl << endl;

	// Variables ----------------------------------------------------------------

	// Graphics:
	CImg<unsigned char> img(screen_size,screen_size,1,3,20);
	//img.fill(20);

	// Line segments:
	int seg = 7;
	Line segs[] =
		{  
			//			Line(280,300,330,120), // first
			Line(450,150,280,330), // second
			Line(400,150,401,190), // third, later		   
			Line(400,400,450,200), // far right
			Line(350,350,350,450),
			Line(10,200,100,215),
			Line(50,50,50,100),
			Line(200,450,300,450)
		};

	for(int outer = 0; outer < seg; ++outer)
	{
		
		// Move center to new point
		//		center = new Point( screen_size/2 , screen_size/2);
		//center_line = new Line( center->x, center->y, center->x+1, center->y );	
	
		// Datastructures:
		skiplist <Point*> angleList;		
		skiplist <Line*> edgeList;	

		double dist; // distance from scan line point to current point
		Line * l;
		Point * p;
		
		// Algorithm -----------------------------------------------------------------
	
		// Draw segments and insert POINTS into skiplist ordered by ANGLE -------------
		for(int i = 0; i < seg; ++i)
		{
			l = &segs[i];
			// Add pointers to all points back to parent line
			l->a->parentLine = l;
			l->b->parentLine = l;

			img.draw_line(l->a->x, l->a->y, l->b->x, l->b->y, WHITE);                    
			img.draw_circle(l->a->x, l->a->y, 2, WHITE);
			img.draw_circle(l->b->x, l->b->y, 2, WHITE);		

			// Calculate the angle from center line:
			l->a->theta = vectorsAngle( l->a->x, l->a->y, center->x, center->y );
			l->b->theta = vectorsAngle( l->b->x, l->b->y, center->x, center->y );
						
			//cout << i << " - T1: " << theta1 << " - T2: " << theta2 << endl;
			//cout << i << ": " << l->a->x << ", " << l->a->y << ", " << l->b->x << ", " << l->b->y << endl;
		
			// Sort the verticies:
		
			angleList.add( l->a);
			angleList.add( l->b);
			//cout << endl;
		}

		// Test SkipList
		cout << "Angle List - points ordered CC from base line";
		angleList.printAll();

		// Draw sweeper:
		img.draw_circle(screen_size / 2, screen_size / 2, 4, RED);
		img.draw_line(screen_size / 2, screen_size / 2, 450, 250, RED);

	
		// Initialize Edge List Of Lines -----------------------------------------------------
		for(int i = 0; i < seg; ++i)
		{
			l = &segs[i]; // get next line to check
		
			//l->print();
		
			// Check each line and see if it crosses scan line
			// Assume scan line is horizontal and to the right
			if( (l->a->y > center->y && l->b->y <= center->y) ||
				(l->a->y <= center->y && l->b->y > center->y) )
			{
				/*
				  m = (l->b->y - l->a->y)/(l->b->x - l->a->x);
				  x_intersect = abs( ( y - l->a->y + m * l->a->x ) / m );
				  dist = abs(x - x_intersect); // because we know the line is horizontal
				  //cout << "X: " << x << " Xi: " << x_intersect << " m: " << m << endl;
				  cout << "Dist: " << dist << endl << endl;

				  // Store distance of line from center for later removal reference
				  l->dist = dist;
				*/
			
				// It does intersect
				edgeList.add( l );

				// Mark as opened, somewhere on line
				l->visited = true;
			
				// Visualize:
				img.draw_line(l->a->x, l->a->y, l->b->x, l->b->y, GREEN);
			}
		}

		cout << "Edge List:";
		edgeList.printAll();

		CImgDisplay disp(img, "Visibility Graph");      // Display the modified image on the screen

		//return 0;
	
		// Sweep
		cout << endl << endl << endl << "BEGIN SWEEP ----------------------------------------------- " << endl << endl;
		sleep(1);
		//for(int i = 0; i < 6; ++i)
		for(int i = 0; i < 2*seg; ++i)
		{
			// take the first vertex in angular order
			p = angleList.pop();
			cout << "Sweep at "; p->print();

			// Update the center_line to the sweep location and update m,b 
			center_line->b = p;

			center_line->updateCalcs();
			// Update center point to contain theta between baseline and
			// current point, so that our line function can cache
			center->theta = p->theta;

		
			// decide what to do with it
			l = (Line*)p->parentLine; // cast it
			cout << "\t"; l->print();

			if( l->visited  ) // remove it from edgeList
			{
				cout << "remove" << endl;

				if( ! l->visitedStartPoint )
				{
					l->visited = false; // allow this line to be visisted again for its start point
				}
			
				// check if its first in the edge list. if it is, its VISIBLE
				if( edgeList.isRoot( l->id ) )
				{
					cout << "Drawing Line" << endl;				
					img.draw_line( center->x, center->y, p->x, p->y, BLUE );
				}

				// remove
				cout << "Value: " << l->value() << " " << l->id << endl;
			
				edgeList.remove( l->value(), l->id );

				img.draw_line(l->a->x, l->a->y, l->b->x, l->b->y, WHITE);
			}
			else // add it to edge list
			{
				cout << "add" << endl;			
				l->visited = true; // mark it as having been visited somewhere
				l->visitedStartPoint = true; // mark it as having found the first vertex
			
				// insert
				dist = distance( p, center );
				cout << "New distance = " << dist << endl;

				// Store distance of line from center 
				l->dist = dist;
			
				edgeList.add( l );

				// check if its first in the edge list. if it is, its VISIBLE
				if( edgeList.isRoot( l->id ) )
				{
					cout << "Drawing Line" << endl;
					img.draw_line( center->x, center->y, p->x, p->y, BLUE );
				}
			
				img.draw_line(l->a->x, l->a->y, l->b->x, l->b->y, GREEN);
			}

			img.draw_circle(p->x, p->y, 7, GREY);		
			disp.display(img);

			//debug
			cout << "Edge List:";
			edgeList.printAll();
			cout << endl << endl;
		
		
			//usleep(250*1000);
			//sleep(1);
		}
	}

	// Show window until user input:
	//disp.display(img);

	while (!disp.is_closed()) {
		if (disp.is_keyESC() )
			break;
		disp.wait();
	}
	
	return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------
//  Calculate Angle Btw 2 Vectors
//-------------------------------------------------------------------------------
double vectorsAngle( int x, int y, int basex, int basey)
{
	// Convert input point x & y to be vectors relative to base point
	int x2 = x - basex;
	int y2 = y - basey;

	// Hard code scan line to point right:
	int x1 = 1;
	int y1 = 0;

	//cout << "x1: " << x1 << " - y1: " << y1 << endl;
	//cout << "x2: " << x2 << " - y2: " << y2 << endl;

	double stuff = (  (x1*x2)+(y1*y2)   ) / (  sqrt(x1*x1+y1*y1)  *  sqrt(x2*x2+y2*y2) );
	//cout << "Stuff: " << stuff << endl;

	// Calculate angle:	
	double result = acos( stuff );
	//cout << "Result: " << result << endl;

	// Now add PI if below middle line:
	if( y > basey )
		result = 2*M_PI - result;
	
	//	cout << "Result: " << result*180/M_PI << " degrees" << endl;

	return result;
}
//-------------------------------------------------------------------------------
//  Distance Btw 2 Points
//-------------------------------------------------------------------------------
double distance( Point * a, Point * b )
{
	return sqrt( pow(b->x - a->x, 2.0) + pow(b->y - a->y,2.0) );
}
