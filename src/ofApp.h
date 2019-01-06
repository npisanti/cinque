#pragma once

#include "ofMain.h"
#include "ofxSIMDFloats.h"
#include "ofxPDSP.h"
#include "ofxPDSPPatches.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        
        void lamp( int fade );
        

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        
        ofxPDSPEngine       engine;
        ofxPDSPOscOutput    oscOut;
                        
        pdsp::Sequence  sequence;
  
  
        atomic<int> scene;
      
        ofxPanel beatsec;  
        std::vector<ofx::patch::sequencer::PTracker> trackers;
        ofx::patch::synth::SinePercussion kick;


        ofArduino	arduino;
        bool		bSetupArduino;
        void setupArduino(const int & version);      
        
        
        // harp---------------
        ofx::patch::synth::KarplusStrong ks;
        ofx::patch::tuning::PitchTable ptable;
        ofx::patch::sequencer::MarkovChain mc0;
        ofx::patch::sequencer::MarkovChain mc1;
        ofxPanel harp;  
        


        // graindrone -------------------
        ofxPanel            dronegui;
        ofxPanel            samples;

        ofx::patch::synth::GrainDrone      drone;
        ofx::patch::util::SampleControl      sample0;
        ofx::patch::util::SampleControl      sample1;
        ofx::patch::util::SampleControl      sample2;
        ofx::patch::util::SampleControl      sample3;
        
        ofx::patch::modulator::ClockedLFO poslfo;
        ofx::patch::modulator::ClockedLFO tremolo;
        
        pdsp::Amp tremL;
        pdsp::Amp tremR;
        ofxPDSPValue            droneamp;
        
        void drone_off();
        void drone_devil();
        void drone_randbeer( int bars );
        void drone_beer( int bars );
        void drone_main( float trem );
        
        ofxPDSPValueMultiplier kicktoggle;
        
        int tvmax;
        std::vector<int> extracted;
        void kicktrack( bool active );
       
        float gphase;

};
