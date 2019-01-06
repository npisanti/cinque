#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

#ifndef __ARM_ARCH
	ofSetupOpenGL( 240*5, 1000, OF_WINDOW );			// <-------- setup the GL context
#else
	ofSetupOpenGL( 200, 200, OF_WINDOW );
#endif
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
